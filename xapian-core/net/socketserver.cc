/* socketserver.cc: class for socket-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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
#include "autoptr.h"
#include "../api/omdatabaseinternal.h"
#include <strstream.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#ifdef TIMING_PATCH
#include <sys/time.h>

#define uint64_t unsigned long long
#endif /* TIMING_PATCH */

/// An object used for "close down" exceptions
struct SocketServerFinished { };

/// The SocketServer constructor, taking two filedescriptors and a database.
SocketServer::SocketServer(OmDatabase db_, int readfd_, int writefd_,
			   int msecs_active_timeout_, int msecs_idle_timeout_
#ifdef TIMING_PATCH
			   , bool timing_
#endif /* TIMING_PATCH */
			   )
	: db(db_),
	  readfd(readfd_),
	  writefd((writefd_ == -1) ? readfd_ : writefd_),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
#ifdef TIMING_PATCH
	  timing(timing_),
#endif /* TIMING_PATCH */
	  buf(new OmSocketLineBuf(readfd, writefd, "socketserver(" + db.get_description() + ')')),
	  conversation_state(conv_ready),
	  gatherer(0),
	  have_global_stats(0)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw OmNetworkError("Couldn't install SIGPIPE handler", errno);
    }
    writeline("OM "STRINGIZE(OM_SOCKET_PROTOCOL_VERSION)" " +
	      om_tostring(db.get_doccount()) + ' ' +
	      om_tostring(db.get_avlength()));
}

SocketServer::SocketServer(OmDatabase db_, AutoPtr<OmLineBuf> buf_,
			   int msecs_active_timeout_, int msecs_idle_timeout_
#ifdef TIMING_PATCH
			   , bool timing_
			   
#endif /* TIMING_PATCH */
			   )
	: db(db_), readfd(-1), writefd(-1),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
#ifdef TIMING_PATCH
	  timing(timing_),
#endif /* TIMING_PATCH */
	  buf(buf_), conversation_state(conv_ready), gatherer(0),
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
    writeline("L" + stats_to_string(stats));
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
#ifdef TIMING_PATCH
    struct timeval stp, etp;
    uint64_t time = 0;
    uint64_t total = 0;
    uint64_t totalidle = 0;
    int returnval = 0;
#endif /* TIMING_PATCH */
    while (1) {
	try {
	    std::string message;

	    // Message 3 (see README_progprotocol.txt)
#ifdef TIMING_PATCH
	    returnval = gettimeofday(&stp, NULL);
#endif /* TIMING_PATCH */
	    message = readline(msecs_idle_timeout);
	    
#ifdef TIMING_PATCH
	    returnval = gettimeofday(&etp, NULL);
	    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
	       	- ((1000000 * stp.tv_sec) + stp.tv_usec);
	    totalidle += time;
#endif /* TIMING_PATCH */
	    switch (message.empty() ? '\0' : message[0]) {
#ifndef TIMING_PATCH
		case 'Q': run_match(message.substr(1)); break;
		case 'T': run_gettermlist(message.substr(1)); break;
		case 'D': run_getdocument(message.substr(1)); break;
		case 'K': run_keepalive(message.substr(1)); break;
#else /* TIMING_PATCH */
		case 'Q': {
		    returnval = gettimeofday(&stp, NULL);
		    run_match(message.substr(1));
		    returnval = gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
		       	- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Match time = " << time
			<< " usecs. (socketserver.cc)\n";
		    break;
		}
		case 'T': {
		    returnval = gettimeofday(&stp, NULL);
		    run_gettermlist(message.substr(1));
		    returnval = gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
		       	- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Get Term List time = " << time
		       	<< " usecs. (socketserver.cc)\n";
		    break;
		}
		case 'D': {
		    returnval = gettimeofday(&stp, NULL);
		    run_getdocument(message.substr(1));
		    gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
		       	- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Get Doc time = " << time
		       	<< " usecs. (socketserver.cc)\n";
		    break;
		}
		case 'K': {
		    returnval = gettimeofday(&stp, NULL);
		    run_keepalive(message.substr(1));
		    gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
		       	- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Keep-alive time = " << time
		       	<< " usecs. (socketserver.cc)\n";
		    break;
		}
#endif /* TIMING_PATCH */
		case 'm': // ignore min weight message left over from postlist
		    break;
		case 'S': // ignore skip_to message left over from postlist
		    break;
		default:
		    throw OmInvalidArgumentError(std::string("Unexpected message:") +
						       message);
	    }
	} catch (const SocketServerFinished &) {
	    // received close message, just return.
#ifdef TIMING_PATCH
	    if (timing) {
		cout << "Total working time = " << total
		    << " usecs. (socketserver.cc)\n";
		cout << "Total waiting time = " << totalidlei
		    << " usecs. (socketserver.cc)\n";
	    }
#endif
	    return;
	} catch (const OmNetworkError &e) {
	    // _Don't_ send network errors over, since they're likely
	    // to have been caused by an error talking to the other end.
	    // (This isn't necessarily true with cascaded remote
	    // databases, though...)
	    throw;
	} catch (const OmError &e) {
	    /* Pass the error across the link, and continue. */
	    writeline(std::string("E") + omerror_to_string(e));
	} catch (...) {
	    /* Do what we can reporting the error, and then propagate
	     * the exception.
	     */
	    writeline(std::string("EUNKNOWN"));
	    throw;
	}
    }
}

void
SocketServer::run_match(const std::string &firstmessage)
{
    std::string message = firstmessage;
    
    gatherer = new NetworkStatsGatherer(this);
    
    OmQuery::Internal query = query_from_string(message);

    // extract the match options
    message = readline(msecs_active_timeout);
    OmSettings moptions = string_to_moptions(message);

    // extract the rset
    message = readline(msecs_active_timeout);
    OmRSet omrset = string_to_omrset(message);

    MultiMatch match(db, &query, omrset, moptions, 0,
		     AutoPtr<StatsGatherer>(gatherer));

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay for statistics");
    sleep(1);
#endif

    // Message 4
    send_local_stats(gatherer->get_local_stats());

    // Message 5, part 1
    message = readline(msecs_active_timeout);

    if (message.empty() || message[0] != 'G') {
	throw OmNetworkError(std::string("Expected 'G', got ") + message);
    }

    global_stats = string_to_stats(message.substr(1));
    have_global_stats = true;

    // Message 5, part 2
    message = readline(msecs_active_timeout);

    if (message.empty() || message[0] != 'M') {
	throw OmNetworkError(std::string("Expected 'M', got ") + message);
    }

    message = message.substr(1);

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

    match.get_mset(first, maxitems, mset, 0);

    DEBUGLINE(UNKNOWN, "done get_mset...");

    writeline("O" + ommset_to_string(mset));

    DEBUGLINE(UNKNOWN, "sent mset...");
}

std::string
SocketServer::readline(int msecs_timeout)
{
    std::string result = buf->readline(OmTime::now() + OmTime(msecs_timeout));
    // intercept 'X' messages.
    if (!result.empty() && result[0] == 'X') {
	throw SocketServerFinished();
    }
    return result;
}

void
SocketServer::writeline(const std::string &message,
			int milliseconds_timeout)
{
    if (milliseconds_timeout == 0) {
	// default to our normal timeout
	milliseconds_timeout = msecs_active_timeout;
    }
    buf->writeline(message, OmTime::now() + OmTime(milliseconds_timeout));
}

void
SocketServer::run_gettermlist(const std::string &firstmessage)
{
    std::string message = firstmessage;

    om_docid did = atoi(message.c_str());

    OmTermIterator tl = db.termlist_begin(did);
    OmTermIterator tlend = db.termlist_end(did);

    while (tl != tlend) {
	std::string item = om_tostring(tl.get_wdf());
        item += ' ';
        item += om_tostring(tl.get_termfreq());
        item += ' ';
	item += encode_tname(*tl);
	writeline(item);
	tl++;
    }

    writeline("Z");
}

void
SocketServer::run_getdocument(const std::string &firstmessage)
{
    std::string message = firstmessage;

    om_docid did = atoi(message.c_str());

    AutoPtr<Document> doc(db.internal->open_document(did));

    writeline("O" + encode_tname(doc->get_data().value));

    std::map<om_keyno, OmKey> keys = doc->get_all_keys();

    std::map<om_keyno, OmKey>::const_iterator i = keys.begin();
    while (i != keys.end()) {
	std::string item = om_tostring(i->first);
	item += ' ';
	item += omkey_to_string(i->second);
	writeline(item);
	++i;
    }

    writeline("Z");
}

void
SocketServer::run_keepalive(const std::string &message)
{
    /* Transmit to any of our own remote databases */
    db.keep_alive();
    writeline("OK");
}
