/* progclient.cc: implementation of NetClient which spawns a program.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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

#include <config.h>
#include "progclient.h"
#include <xapian/error.h>
#include "utils.h"
#include "netutils.h"
#include "omdebug.h"

#include <string>
#include <vector>

using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <cstdio>
#include <cerrno>

ProgClient::ProgClient(string progname, const string &args, int msecs_timeout_)
	: SocketClient(get_spawned_socket(progname, args),
		       msecs_timeout_,
		       get_progcontext(progname, args),
		       false /* closing socket our responsibility */)
{
    DEBUGCALL(DB, void, "ProgClient::ProgClient", progname << ", " << args <<
	      ", " << msecs_timeout_);
}

string
ProgClient::get_progcontext(string progname, const string &args)
{
    DEBUGCALL_STATIC(DB, string, "ProgClient::get_progcontext", progname <<
	   	    ", " << args);
    string result = "remote:prog(" + progname + " " + args;
    return result;
}

int
ProgClient::get_spawned_socket(string progname, const string &args)
{
    DEBUGCALL(DB, int, "ProgClient::get_spawned_socket", progname << ", " <<
	      args);
    /* socketpair() returns two sockets.  We keep sv[0] and give
     * sv[1] to the child process.
     */
    int sv[2];

    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) < 0) {
	throw Xapian::NetworkError(string("socketpair:") + strerror(errno), get_progcontext(progname, args));
    }

    pid = fork();

    if (pid < 0) {
	throw Xapian::NetworkError(string("fork:") + strerror(errno), get_progcontext(progname, args));
    }

    if (pid == 0) {
	/* child process:
	 *   set up file descriptors and exec program
	 */

	// replace stdin and stdout with the socket
	// FIXME: check return values.
	close(0);
	close(1);
	dup2(sv[1], 0);
	dup2(sv[1], 1);

	// close unnecessary file descriptors
	// FIXME: Probably a bit excessive...
	for (int i=3; i<256; ++i) {
	    close(i);
	}

	vector<string> argvec;
	split_words(args, argvec);

	// In theory, a potential memory leak here.
	// In practice, we either execvp() or exit().
	const char **new_argv = new const char *[argvec.size() + 2];

	new_argv[0] = progname.c_str();
	for (vector<string>::size_type i=0;
	     i<argvec.size();
	     ++i) {
	    new_argv[i+1] = argvec[i].c_str();
	}
	new_argv[argvec.size() + 1] = 0;
	execvp(progname.c_str(),
	       const_cast<char *const *>(new_argv));

	// if we get here, then execvp failed.
	/* throwing an exception is a bad idea, since we're
	 * not the original process. */
	_exit(-1);
    } else {
	// parent
	// close the child's end of the socket
	close(sv[1]);
	return sv[0];
    }
}

ProgClient::~ProgClient()
{
    // close the socket and reap the child.
    do_close();
    waitpid(pid, 0, 0);
}
