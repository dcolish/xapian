/* socketserver.cc: class for socket-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "config.h"
#include "socketserver.h"
#include "database.h"
#include "stats.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"
#include "om/omerror.h"
#include "omerr_string.h"
#include "termlist.h"
#include "document.h"
#include "omdebug.h"
#include "om/autoptr.h"
#include "../api/omdatabaseinternal.h"
#include "omdatabaseinterface.h"
#include <strstream.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>

/// The SocketServer constructor, taking two filedescriptors and a database.
SocketServer::SocketServer(OmDatabase db_,
			   int readfd_,
			   int writefd_,
			   int msecs_timeout_)
	: db(db_),
	  readfd(readfd_),
	  writefd((writefd_ == -1) ? readfd_ : writefd_),
	  msecs_timeout(msecs_timeout_),
	  buf(new OmSocketLineBuf(readfd, writefd)),
	  conversation_state(conv_ready),
	  gatherer(0),
	  have_global_stats(0)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw OmNetworkError(std::string("signal: ") + strerror(errno));
    }
    buf->readline(msecs_timeout);
    buf->writeline("HELLO " +
		  om_tostring(OM_SOCKET_PROTOCOL_VERSION) + " " +
		  om_tostring(db.get_doccount()) + " " +
		  om_tostring(db.get_avlength()));
}

SocketServer::SocketServer(OmDatabase db_,
			   AutoPtr<OmLineBuf> buf_,
			   int msecs_timeout_)
	: db(db_),
	  readfd(-1),
	  writefd(-1),
	  msecs_timeout(msecs_timeout_),
	  buf(buf_),
	  conversation_state(conv_ready),
	  gatherer(0),
	  have_global_stats(0)
{
    Assert(buf.get() != 0);
}

/// The SocketServer destructor
SocketServer::~SocketServer()
{
}

void
SocketServer::send_local_stats(Stats stats)
{
    std::string mystatstr = std::string("MYSTATS ") + stats_to_string(stats);

    buf->writeline(mystatstr);
    DebugMsg("SocketServer::send_local_stats(): wrote " << mystatstr);
}

Stats
SocketServer::get_global_stats()
{
    Assert(have_global_stats);

    return global_stats;
}

void
SocketServer::run()
{
    try {
	while (1) {
	    std::string message;

	    // Message 3 (see README_progprotocol.txt)
	    message = buf->readline(msecs_timeout);

	    if (message == "QUIT") {
		return;
	    } else if (startswith(message, "MOPTIONS ")) {
		run_match(message);
	    } else if (startswith(message, "GETTLIST ")) {
		run_gettermlist(message);
	    } else if (startswith(message, "GETDOC ")) {
		run_getdocument(message);
	    } else if (startswith(message, "M")) {
		// ignore min weight message left over from postlist
	    } else if (startswith(message, "S")) {
		// ignore skip_to message left over from postlist
	    } else {
		throw OmInvalidArgumentError(std::string("Unexpected message:") +
					     message);
	    }

	}
    } catch (OmNetworkError &e) {
	// _Don't_ send network errors over, since they're likely to have
	// been caused by an error talking to the other end.
	throw;
    } catch (OmError &e) {
	buf->writeline(std::string("ERROR ") + omerror_to_string(e));
	throw;
    } catch (...) {
	buf->writeline(std::string("ERROR UNKNOWN"));
	throw;
    }
}

static OmLineBuf *snooper_buf; // FIXME FIXME FIXME
static snooper_do_collapse;

void
match_snooper(const OmMSetItem &i)
{
    std::string key = "";
    if (snooper_do_collapse) {
	key = ";" + omkey_to_string(i.collapse_key);
    }
    snooper_buf->writeline(om_tostring(i.did) + " " + om_tostring(i.wt) + key);
}

void
SocketServer::run_match(const std::string &firstmessage)
{
    std::string message = firstmessage;
    // extract the match options
    if (!startswith(message, "MOPTIONS ")) {
	throw OmNetworkError(std::string("Expected MOPTIONS, got ") + message);
    }

    OmSettings moptions = string_to_moptions(message.substr(9));

    // extract the rset
    message = buf->readline(msecs_timeout);
    if (!startswith(message, "RSET ")) {
	DEBUGLINE(UNKNOWN, "Expected RSET, got " << message);
	throw OmNetworkError(std::string("Invalid message: ") + message);
    }
    OmRSet omrset = string_to_omrset(message.substr(5));

    // extract the query
    message = buf->readline(msecs_timeout);

    if (!startswith(message, "SETQUERY ")) {
	DEBUGLINE(UNKNOWN, "Expected SETQUERY, got " << message);
	throw OmNetworkError("Invalid message");
    }

    message = message.substr(9, message.npos);

    // Extract the query
    if (message[0] != '\"' || message[message.length()-1] != '\"') {
	throw OmNetworkError("Invalid query specification");
    } else {
	message = message.substr(1, message.length() - 2);
    }
    OmQuery::Internal query = query_from_string(message);

    gatherer = new NetworkStatsGatherer(this);
    MultiMatch match(db, &query, omrset, moptions,
		     AutoPtr<StatsGatherer>(gatherer));

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay for statistics");
    sleep(1);
#endif

    // Message 4
    send_local_stats(gatherer->get_local_stats());

    // Message 5, part 1
    message = buf->readline(msecs_timeout);

    if (message.substr(0, 9) != "GLOBSTATS") {
	throw OmNetworkError(std::string("Expected GLOBSTATS, got ") + message);
    }

    global_stats = string_to_stats(message.substr(10));
    have_global_stats = true;

    // Message 5, part 2
    message = buf->readline(msecs_timeout);

    if (message.substr(0, 7) != "GETMSET") {
	if (message.substr(0, 8) != "GETPLIST") {
	    throw OmNetworkError(std::string("Expected GETMSET or GETPLIST, got ") + message);
	}
	message = message.substr(9);
	om_doccount first;
	om_doccount maxitems;
	{
	    // extract first,maxitems
	    istrstream is(message.c_str());
	    is >> first >> maxitems;
	}
#if 1
	snooper_do_collapse = (moptions.get_int("match_collapse_key", -1) >= 0);
	PostList *pl;
	{
	    std::map<om_termname, OmMSet::TermFreqAndWeight> terminfo;
	    pl = match.get_postlist(first, maxitems, terminfo, 0);
	    buf->writeline(om_tostring(pl->get_termfreq()) + " " +
			   om_tostring(pl->recalc_maxweight()));
	    buf->writeline(ommset_termfreqwts_to_string(terminfo));
	    snooper_buf = buf.get();
	    OmMSet mset;
	    match.get_mset_2(pl, terminfo, first, maxitems, mset, 0,
			     match_snooper);
	}
	buf->writeline("OK");
	return;
#else
	PostList *pl;
	{
	    std::map<om_termname, OmMSet::TermFreqAndWeight> terminfo;
	    // not sure we really need these numbers...
	    pl = match.get_postlist(first, maxitems, terminfo, 0);
	    buf->writeline(om_tostring(pl->get_termfreq()) + " " +
			   om_tostring(pl->recalc_maxweight()));
	    buf->writeline(ommset_termfreqwts_to_string(terminfo));
	}
	om_docid did = 0;
	om_weight w_min = 0;
	om_keyno collapse_key;
	bool do_collapse = true;
	{
	    int val = moptions.get_int("match_collapse_key", -1);
	    if (val >= 0) {
		do_collapse = true;
		collapse_key = val;
	    }
	}
	while (1) {
	    om_docid new_did = 0;
	    while (buf->data_waiting()) {
		std::string m = buf->readline(0);
		switch (m.empty() ? 0 : m[0]) {
		    case 'M': {
			// min weight has dropped
			istrstream is(message.c_str() + 1);
			is >> w_min;
			DEBUGLINE(UNKNOWN, "w_min now " << w_min);
			break;
		    }
		    case 'S': {
			// skip to
			istrstream is(message.c_str() + 1);
			is >> new_did;
			DEBUGLINE(UNKNOWN, "skip_to now " << new_did);
			break;
		    }
		default:
		    Assert(false);
		}
	    }
	    PostList *p;
	    if (new_did > did + 1) {
		DEBUGLINE(UNKNOWN, "skip_to(" << new_did << ", " << w_min << ")");
		p = pl->skip_to(new_did, w_min);
	    } else {
		DEBUGLINE(UNKNOWN, "next(" << w_min << ")");
		p = pl->next(w_min);
	    }
	    if (p) {
		delete pl;
		pl = p;
	    }
	    if (pl->at_end()) break;
	    did = pl->get_docid();
	    om_weight w = pl->get_weight();
	    if (w >= w_min) {
		DEBUGLINE(UNKNOWN, "Returning did " << did << " wt " << w);
		std::string key = "";
		if (do_collapse) {
		    AutoPtr<LeafDocument> doc(OmDatabase::InternalInterface::get(match.db)->open_document(did));
		    key = ";" + omkey_to_string(doc->get_key(collapse_key));		    
		}
		buf->writeline(om_tostring(did) + " " + om_tostring(w) + key);
	    } else {
		DEBUGLINE(UNKNOWN, "Ignoring did " << did << " wt " << w << " (since wt < " << w_min << ")");
	    }
	}
	delete pl;
	buf->writeline("OK");
	return;
#endif
    }
    message = message.substr(8);

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay...");
    sleep(2);
#endif

    om_doccount first;
    om_doccount maxitems;
    {
	// extract first,maxitems
	istrstream is(message.c_str());
	is >> first >> maxitems;
    }

    OmMSet mset;

    DEBUGLINE(UNKNOWN, "About to get_mset(" << first
	      << ", " << maxitems << "...");

    match.get_mset(first, maxitems, mset, 0, 0);

    DEBUGLINE(UNKNOWN, "done get_mset...");

    buf->writeline(std::string("MSET ") + ommset_to_string(mset));

    DEBUGLINE(UNKNOWN, "sent mset...");

    buf->writeline("OK");

    //DEBUGLINE(UNKNOWN, "sent OK...");
}

void
SocketServer::run_gettermlist(const std::string &firstmessage)
{
    std::string message = firstmessage;
    // extract the match options
    if (!startswith(message, "GETTLIST ")) {
	throw OmNetworkError(std::string("Expected GETTLIST, got ") + message);
    }

    om_docid did = atoi(message.c_str() + 9);

    OmTermListIterator tl = db.termlist_begin(did);
    OmTermListIterator tlend = db.termlist_end(did);

    while (tl != tlend) {
	std::string item = "TLISTITEM ";
	item = item + encode_tname(*tl) + " ";
	item = item + om_tostring(tl.get_wdf()) + " ";
	item = item + om_tostring(tl.get_termfreq());
	buf->writeline(item);

	tl++;
    }

    buf->writeline("END");
}

void
SocketServer::run_getdocument(const std::string &firstmessage)
{
    std::string message = firstmessage;
    // extract the match options
    if (!startswith(message, "GETDOC ")) {
	throw OmNetworkError(std::string("Expected GETDOC, got ") + message);
    }

    om_docid did = atoi(message.c_str() + 7);

    AutoPtr<LeafDocument> doc(OmDatabase::InternalInterface::get(db)->open_document(did));

    buf->writeline(std::string("DOC ") + encode_tname(doc->get_data().value));

    std::map<om_keyno, OmKey> keys = doc->get_all_keys();

    std::map<om_keyno, OmKey>::const_iterator i = keys.begin();
    while (i != keys.end()) {
	std::string item = std::string("KEY ") +
		om_tostring(i->first) + " " +
		omkey_to_string(i->second);
	buf->writeline(item);
	++i;
    }

    buf->writeline("END");
}

void
SocketServer::read_global_stats()
{
    Assert(conversation_state == conv_getglobal);

    global_stats = string_to_stats(buf->readline(msecs_timeout));

    conversation_state = conv_sendresult;

    have_global_stats = true;
}
