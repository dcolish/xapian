/* socketclient.cc: implementation of NetClient using a socket
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
#include "socketclient.h"
#include "om/omerror.h"
#include "omerr_string.h"
#include "utils.h"
#include "netutils.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstdio>
#include <cerrno>
#include <strstream.h>

SocketClient::SocketClient(int socketfd_,
			   bool close_socket_)
	: socketfd(socketfd_),
	  close_socket(close_socket_),
	  buf(socketfd),
	  conv_state(state_getquery),
	  remote_stats_valid(false),
	  global_stats_valid(false)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw OmNetworkError(string("signal: ") + strerror(errno));
    }

    do_write("HELLO!\n");

    string received = do_read();

    DebugMsg("Read back " << received << endl);
    if (received.substr(0, 5) != "HELLO") {
	throw OmNetworkError("Unknown start of conversation");
    }

    handle_hello(received.substr(6));
}

void
SocketClient::handle_hello(const string &s)
{
    istrstream is(s.c_str());

    int version;
    is >> version >> doccount >> avlength;

    if (version != OM_SOCKET_PROTOCOL_VERSION) {
	throw OmNetworkError(string("Invalid protocol version: ") +
			     om_inttostring(version));
    }
}

NetClient::TermListItem
string_to_tlistitem(const string &s)
{
    istrstream is(s.c_str());
    NetClient::TermListItem item;

    string tencoded;

    is >> tencoded >> item.wdf >> item.termfreq;

    item.tname = decode_tname(tencoded);

    return item;
}

void
SocketClient::get_tlist(om_docid did,
			vector<NetClient::TermListItem> &items) {
    string message;

    buf.writeline(string("GETTLIST ") + om_inttostring(did));

    while (startswith(message = do_read(), "TLISTITEM ")) {
	items.push_back(string_to_tlistitem(message.substr(10)));
    }

    if (message != "END") {
	throw OmNetworkError(string("Expected END, got ") + message);
    }
}

void
SocketClient::get_doc(om_docid did,
		      string &doc,
		      map<om_keyno, OmKey> &keys)
{
    string message;
    buf.writeline(string("GETDOC ") + om_inttostring(did));

    message = do_read();
    if (!startswith(message, "DOC ")) {
	throw OmNetworkError(string("Expected DOC, got ") + message);
    }

    doc = decode_tname(message.substr(4));

    while (startswith(message = do_read(), "KEY ")) {
	istrstream is(message.substr(4).c_str());
	om_keyno keyno;
	string omkey;
	is >> keyno >> omkey;
	keys[keyno] = string_to_omkey(omkey);
    }

    if (message != "END") {
	throw OmNetworkError(string("Expected END, got ") + message);
    }
}

om_doccount
SocketClient::get_doccount()
{
    return doccount;
}

om_doclength
SocketClient::get_avlength()
{
    return avlength;
}

string
SocketClient::do_read()
{
    string retval = buf.readline();

    DebugMsg("do_read(): " << retval << endl);

    if (retval.substr(0, 5) == "ERROR") {
	string_to_omerror(retval.substr(6), "REMOTE:");
    }

    return retval;
}

void
SocketClient::do_write(string data)
{
    DebugMsg("do_write(): " << data.substr(0, data.find_last_of('\n')) << endl);
    buf.writeline(data);
}

void
SocketClient::write_data(string msg)
{
    do_write(msg);
}

string
SocketClient::read_data()
{
    return do_read();
}

bool
SocketClient::data_is_available()
{
    return buf.data_waiting();
}

void
SocketClient::do_close()
{
    // musn't let any exception escape a destructor, or else we
    // abort immediately if from a destructor.
    try {
	buf.writeline("QUIT");
    } catch (...) {
    }
    close(socketfd);
}

SocketClient::~SocketClient()
{
    if (close_socket) {
	do_close();
    }
}

void
SocketClient::set_weighting(IRWeight::weight_type wt_type)
{
    Assert(conv_state == state_getquery);
    wt_string = om_inttostring(wt_type);
}

void
SocketClient::set_query(const OmQueryInternal *query_)
{
    Assert(conv_state == state_getquery);
    query_string = query_->serialise();
} 

void
SocketClient::set_options(const OmMatchOptions &moptions_)
{
    moptions = moptions_;
}

bool
SocketClient::finish_query()
{
    bool success = false;
    switch (conv_state) {
	case state_getquery:
	    // Message 3 (see README_progprotocol.txt)
	    {
		string message;

		message += "MOPTIONS " +
			moptions_to_string(moptions) + '\n';
		message += "RSET " +
			omrset_to_string(omrset) + '\n';
		message += "SETQUERY " +
		                 wt_string + " \"" +
		       	         query_string + "\"" + '\n';
		do_write(message);
	    }
	    conv_state = state_sentquery;
	    // fall through...
	case state_sentquery:

	    // Message 4
	    if (!buf.data_waiting()) {
		break;
	    }
	    
	    {
		string response = do_read();
		if (response.substr(0, 7) != "MYSTATS") {
		    throw OmNetworkError("Error getting statistics");
		}
		remote_stats = string_to_stats(response.substr(8, response.npos));
		remote_stats_valid = true;

		success = true;
	    }

	    conv_state = state_sendglobal;
	    // fall through...
	case state_sendglobal:
	case state_getmset:
	case state_getresult:
	    ;
    }
    return success;
}

void
SocketClient::wait_for_input()
{
    buf.wait_for_data(10000);
}

bool
SocketClient::get_remote_stats(Stats &out)
{
    Assert(remote_stats_valid && conv_state >= state_sentquery);
    if (!remote_stats_valid && conv_state <= state_sendglobal) {
	bool finished = finish_query();

	if (!finished) return false;
    }

    out = remote_stats;

    conv_state = state_sendglobal;
    return true;
}

void
SocketClient::do_simple_transaction(string msg)
{
    do_write(msg + '\n');
    string response = do_read();

    if (response != "OK") {
	throw OmNetworkError(string("Invalid response: (") +
			     msg + ") -> (" + response + ")");
    }
}

string
SocketClient::do_transaction_with_result(string msg)
{
    do_write(msg + '\n');
    string response = do_read();

    if (response == "ERROR") {
	throw OmNetworkError(string("Error response: (") +
			     msg + ") -> (" + response + ")");
    }
    return response;
}

OmMSetItem
string_to_msetitem(string s)
{
    istrstream is(s.c_str());
    om_weight wt;
    om_docid did;
    string keyval;

    string header;

    is >> header >> wt >> did >> keyval;

    Assert (header == "MSETITEM:");

    return OmMSetItem(wt, did, string_to_omkey(keyval));
}


void
SocketClient::send_global_stats(const Stats &stats)
{
    Assert(conv_state >= state_sendglobal);
    if (conv_state == state_sendglobal) {
	global_stats = stats;
	global_stats_valid = true;
	conv_state = state_getmset;
    };
}

bool
SocketClient::get_mset(om_doccount first,
		       om_doccount maxitems,
		       vector<OmMSetItem> &mset,
		       om_doccount *mbound,
		       om_weight *greatest_wt)
{
    Assert(global_stats_valid);
    Assert(conv_state >= state_getmset);
    switch (conv_state) {
	case state_getquery:
	case state_sentquery:
	case state_sendglobal:
	    throw OmInvalidArgumentError("get_mset called before global stats given");
	    break;
	case state_getmset:

	    // Message 5 (see README_progprotocol.txt)
	    {
		string message = "GLOBSTATS " +
			stats_to_string(global_stats) + '\n';
		message += "GETMSET " +
			    om_inttostring(first) + " " +
			    om_inttostring(maxitems);
		do_write(message);
	    }
	    conv_state = state_getresult;

	    // fall through...
	case state_getresult:

	    if (!buf.data_waiting()) {
		return false;
	    }
	
	    // Message 6
	    {
		string response = do_read();
		if (response.substr(0, 9) != "MSETITEMS") {
		    throw OmNetworkError(string("Expected MSETITEMS, got ") + response);
		}
		response = response.substr(10);

		int numitems;
		{
		    istrstream is(response.c_str());

		    is >> numitems >> remote_maxweight >> *greatest_wt;
		}

		for (int i=0; i<numitems; ++i) {
		    string msetline = do_read();
		    //DebugMsg("MSet string: " << msetline);
		    OmMSetItem mitem = string_to_msetitem(msetline);
		    //DebugMsg("MSet item: " << mitem.wt << " " << mitem.did << endl);
		    mset.push_back(mitem);
		}
		response = do_read();
		if (response != "OK") {
		    throw OmNetworkError("Error at end of mset");
		}
	    }
    } // switch (conv_state)

    // reset the state
    conv_state = state_getquery;
    wt_string = "";
    query_string = "";
    remote_stats = Stats();
    remote_stats_valid = false;
    global_stats = Stats();
    global_stats_valid = false;
    moptions = OmMatchOptions();
    omrset = OmRSet();

    return true;
}

om_weight
SocketClient::get_max_weight()
{
    return remote_maxweight;
}

void
SocketClient::set_rset(const OmRSet &omrset_)
{
    omrset = omrset_;
}
