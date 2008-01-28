/** @file replicatetcpserver.cc
 * @brief TCP/IP replication server class.
 */
/* Copyright (C) 2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "replicatetcpserver.h"

#include <xapian/error.h>
#include <xapian/replication.h>

#include "omtime.h"

using namespace std;

ReplicateTcpServer::ReplicateTcpServer(const string & host, int port,
				       const string & path_)
    : TcpServer(host, port, false, false), path(path_)
{
}

ReplicateTcpServer::~ReplicateTcpServer() {
}

void
ReplicateTcpServer::handle_one_connection(int socket)
{
    RemoteConnection client(socket, -1, "");
    try {
	string start_revision;
	// Read start_revision from the client.
	if (client.get_message(start_revision, OmTime()) != 'R') {
	    throw Xapian::NetworkError("Bad replication client message");
	}
	Xapian::DatabaseMaster master(path);
	master.write_changesets_to_fd(socket, start_revision);
    } catch (...) {
	// Ignore exceptions.
    }
}
