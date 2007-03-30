/** @file  remoteconnection.h
 *  @brief RemoteConnection class used by the remote backend.
 */
/* Copyright (C) 2006 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_REMOTECONNECTION_H
#define XAPIAN_INCLUDED_REMOTECONNECTION_H

#include <string>

#include "remoteprotocol.h"

class OmTime;

/** A RemoteConnection object provides a bidirectional connection to another
 *  RemoteConnection object on a remote machine.
 *
 *  The connection is implemented using a pair of file descriptors.  Messages
 *  with a single byte type code and arbitrary data as the contents can be
 *  sent and received.
 */
class RemoteConnection {
    /// Don't allow assignment.
    void operator=(const RemoteConnection &);

    /// Don't allow copying.
    RemoteConnection(const RemoteConnection &);

    /// The file descriptor used for reading.
    int fdin;

    /// The file descriptor used for writing.
    int fdout;

    /// The context to report with errors
    std::string context;

    /// Buffer to hold unprocessed input.
    std::string buffer;

    /** Read until there are at least min_len bytes in buffer.
     *
     *  If for some reason this isn't possible, throws NetworkError.
     *
     *  @param min_len	Minimum number of bytes required in buffer.
     *  @param end_time	If this time is reached, then a timeout
     *			exception will be thrown.  If end_time == OmTime(),
     *			then keep trying indefinitely.
     */
    void read_at_least(size_t min_len, const OmTime & end_time);

#ifdef __WIN32__
    // On Windows we use overlapped IO.  We share an overlapped structure
    // for both reading and writing, as we know that we always wait for
    // one to finish before starting another (ie, we don't *really* use
    // overlapped IO - no IO is overlapped - its used only to manage timeouts)
    WSAOVERLAPPED overlapped;
#endif
  public:
    /// Constructor.
    RemoteConnection(int fdin_, int fdout_, const std::string & context_);

    /** See if there is data available to read.
     *
     *  @return		true if there is data waiting to be read.
     */
    bool ready_to_read() const;

    /** Read one message from fdin.
     *
     *  @param[out] result	Message data.
     *  @param end_time		If this time is reached, then a timeout
     *				exception will be thrown.
     *
     *  @return			Message type code.
     */
    char get_message(std::string &result, const OmTime & end_time);

    /// Send a message.
    void send_message(char type, const std::string & s, const OmTime & end_time);

    /// Shutdown the connection.
    void do_close();
};

#endif // XAPIAN_INCLUDED_REMOTECONNECTION_H
