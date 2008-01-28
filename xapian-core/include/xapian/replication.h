/** \file replication.h
 * \brief Replication support for Xapian databases.
 */
/* Copyright 2008 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_REPLICATION_H
#define XAPIAN_INCLUDED_REPLICATION_H

#include <xapian/database.h>
#include <string>

namespace Xapian {

/// Access to a master database for replication.
class DatabaseMaster {
    /// The path to the master database.
    std::string path;

  public:
    /** Create a new DatabaseMaster for the database at the specified path.
     *
     *  The database isn't actually opened until a set of changesets is
     *  requested.
     */
    DatabaseMaster(const std::string & path_) : path(path_) {}

    /** Write a set of changeset to a file descriptor to upgrade a database
     *  from a specified revision to the latest available revision.
     */
    void write_changesets_to_fd(int fd,
				const std::string & start_revision) const;
};

/// Access to a database replica, for applying replication to it.
class DatabaseReplica {

    /// The path to the replica.
    std::string path;

    WritableDatabase db;
  public:
    /** Open a new DatabaseReplica for the database at the specified path.
     *
     *  The path should either point to a database previously created by a
     *  DatabaseReplica, or to a path which doesn't yet exist.
     *
     *  The path should always be in a directory which exists.
     *
     *  If the specified path does not contain a database, a database will be
     *  created when an appropriate changeset is supplied to the replica.
     *
     *  @param path The path to make the replica at.
     */
    DatabaseReplica(const std::string & path_);

    /// Get a string describing the current revision of the replica.
    std::string get_revision_info() const;

    /** Read and apply the next changeset.
     *
     *  If no changesets are found on the file descriptor, returns false
     *  immediately.
     *
     *  If any changesets are found on the file descriptor, exactly one of them
     *  is applied.
     *
     *  A common way to use this method is to call it repeatedly until it
     *  returns false, with an appropriate gap between each call.
     *
     *  @param fd The file descriptor to read the changeset from.
     *  @return true if there are more changesets to apply on the file
     *  descriptor, false otherwise.
     */

    bool apply_next_changeset_from_fd(int fd);
};

}

#endif /* XAPIAN_INCLUDED_REPLICATION_H */
