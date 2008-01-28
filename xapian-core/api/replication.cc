/** @file replication.cc
 * @brief Replication support for Xapian databases.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
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

#include <config.h>
#include "xapian/replication.h"

#include "xapian/dbfactory.h"
#include "xapian/error.h"

#include "database.h"
#include "omdebug.h"
#include "utils.h"

#include <fstream>
#include <string>

using namespace std;
using namespace Xapian;

void
DatabaseMaster::write_changesets_to_fd(int fd,
				       const string & start_revision) const
{
    DEBUGAPICALL(void, "DatabaseMaster::write_changesets_to_fd",
		 fd << ", " << start_revision);
    // FIXME - implement
    (void) fd;
    (void) start_revision;
}

DatabaseReplica::DatabaseReplica(const string & path_)
	: path(path_), db()
{
    DEBUGAPICALL(void, "DatabaseReplica::DatabaseReplica", path_);
    if (dir_exists(path)) {
	throw InvalidOperationError("Replica path should not be a directory");
    }
#ifndef XAPIAN_HAS_FLINT_BACKEND
    throw FeatureUnavailableError("Flint backend is not enabled, and needed for database replication");
#endif
    if (!file_exists(path)) {
	// The database doesn't already exist - make a stub database, and point
	// it to a separate flint database.
	string real_path(path);
	real_path += "_0";
	ofstream stub(path.c_str());
	stub << "flint " << real_path;
    } else {
	// The database already exists as a stub database - open it.  We can't
	// just use the standard opening routines, because we want to open it
	// for writing.
	ifstream stub(path.c_str());
	string line;
	while (getline(stub, line)) {
	    string::size_type space = line.find(' ');
	    if (space != string::npos) {
		string type = line.substr(0, space);
		line.erase(0, space + 1);
		if (type == "flint") {
		    db.add_database(Flint::open(line, Xapian::DB_OPEN));
		} else {
		    throw FeatureUnavailableError("Database replication only works with flint databases.");
		}
	    }
	}
	if (db.internal.size() != 1) {
	    throw Xapian::InvalidOperationError("DatabaseReplica needs to be pointed at exactly one subdatabase");
	}
    }
}

string
DatabaseReplica::get_revision_info() const
{
    DEBUGAPICALL(string, "DatabaseReplica::get_revision_info", "");
    if (db.internal.size() != 1) {
	throw Xapian::InvalidOperationError("DatabaseReplica needs to be pointed at exactly one subdatabase");
    }
    RETURN((db.internal[0])->get_revision_info());
}

bool 
DatabaseReplica::apply_next_changeset_from_fd(int fd)
{
    DEBUGAPICALL(bool, "DatabaseReplica::apply_next_changeset_from_fd", fd);
    // FIXME - implement
    (void) fd;
    RETURN(false);
}
