/* database.cc: Database factories and base class
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
#include "database.h"
#include "utils.h" // for om_tostring
#include <xapian/error.h>

#include <fstream>
#include <string>

#include<iostream>
using namespace std;

// Include headers for all the enabled database backends
#ifdef MUS_BUILD_BACKEND_MUSCAT36
#include "muscat36/da_database.h"
#include "muscat36/db_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_INMEMORY
#include "inmemory/inmemory_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_QUARTZ
#include "quartz/quartz_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_REMOTE
// These headers are all in common
#include "net_database.h"
#include "progclient.h"
#include "tcpclient.h"
#endif

namespace Xapian {

#ifdef MUS_BUILD_BACKEND_QUARTZ
Database
Quartz::open(const string &dir) {
    DEBUGAPICALL_STATIC(Database, "Quartz::open", dir);
    return Database(new QuartzDatabase(dir));
}

WritableDatabase
Quartz::open(const string &dir, int action, int block_size) {
    DEBUGAPICALL_STATIC(WritableDatabase, "Quartz::open", dir << ", " <<
			action << ", " << block_size);
    return WritableDatabase(new QuartzWritableDatabase(dir, action,
						       block_size));
}
#endif

#ifdef MUS_BUILD_BACKEND_INMEMORY
// Note: a read-only inmemory database will always be empty, and so there's
// not much use in allowing one to be created.
WritableDatabase
InMemory::open() {
    DEBUGAPICALL_STATIC(Database, "InMemory::open", "");
    return WritableDatabase(new InMemoryDatabase());
}
#endif

#ifdef MUS_BUILD_BACKEND_MUSCAT36
Database
Muscat36::open_da(const string &R, const string &T, bool heavy_duty) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_da", R << ", " << T << ", " <<
			heavy_duty);
    return Database(new DADatabase(R, T, "", heavy_duty));
}

Database
Muscat36::open_da(const string &R, const string &T, const string &keys,
		  bool heavy_duty) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_da", R << ", " << T << ", " <<
			keys << ", " << heavy_duty);
    return Database(new DADatabase(R, T, keys, heavy_duty));
}

Database
Muscat36::open_db(const string &DB, size_t cache_size) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_db", DB << ", " <<
			cache_size);
    return Database(new DBDatabase(DB, "", cache_size));
}

Database
Muscat36::open_db(const string &DB, const string &keys, size_t cache_size) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_db", DB << ", " << keys <<
			", " << cache_size);
    return Database(new DBDatabase(DB, keys, cache_size));
}
#endif

#ifdef MUS_BUILD_BACKEND_REMOTE
Database
Remote::open(const string &program, const string &args, unsigned int timeout)
{
    DEBUGAPICALL_STATIC(Database, "Remote::open", args << ", " << timeout);
    Xapian::Internal::RefCntPtr<NetClient> link(new ProgClient(program, args, timeout));
    return Database(new NetworkDatabase(link));
}

Database
Remote::open(const string &host, unsigned int port,
	unsigned int timeout, unsigned int connect_timeout)
{
    DEBUGAPICALL_STATIC(Database, "Remote::open", host << ", " << port <<
			", " << timeout << ", " << connect_timeout);
    if (connect_timeout == 0) connect_timeout = timeout;
    Xapian::Internal::RefCntPtr<NetClient> link(new TcpClient(host, port, timeout, connect_timeout));
    return Database(new NetworkDatabase(link));
}
#endif

Database
Auto::open_stub(const string &file)
{
    DEBUGAPICALL_STATIC(Database, "Auto::open_stub", file);
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    ifstream stub(file.c_str());
    Database db;
    string line;
    int line_no = 1;
    bool ok = false;
    while (getline(stub, line)) {
	string::size_type space = line.find(' ');
	if (space != string::npos) {
	    string type = line.substr(0, space);
	    line.erase(0, space + 1);
	    if (type == "auto") {
		db.add_database(Auto::open(line));
		ok = true;
#ifdef MUS_BUILD_BACKEND_QUARTZ
	    } else if (type == "quartz") {
		db.add_database(Quartz::open(line));
		ok = true;
#endif
#ifdef MUS_BUILD_BACKEND_REMOTE
	    } else if (type == "remote") {
		string::size_type colon = line.find(':');
		if (colon == 0) {
		    // prog
		    // FIXME: timeouts
		    // FIXME: Is prog actually useful beyond testing?
		    // Is it a security risk?
		    space = line.find(' ');
		    string args;
		    if (space != string::npos) {
			args = line.substr(space + 1);
			line = line.substr(1, space - 1);
		    } else {
			line.erase(0, 1);
		    }
		    db.add_database(Remote::open(line, args));
		    ok = true;
		} else if (colon != string::npos) {
		    // tcp
		    // FIXME: timeouts
		    unsigned int port = atoi(line.c_str() + colon);
		    line.erase(colon);
		    db.add_database(Remote::open(line, port));
		    ok = true;
		}
#endif
	    }
#ifdef MUS_BUILD_BACKEND_MUSCAT36
	    // FIXME: da and db too, but I'm too slack to do those right now!
#endif
	}
	if (!ok) break;
	++line_no;
    }
    if (!ok) {
	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange it to be read as a stub database.  The line number is
	// enough information to identify the problem line.
	throw OpeningError("Bad line " + om_tostring(line_no) + " in stub database file `" + file + "'");
    }
    return db;
}

Database
Auto::open(const string &path)
{
    DEBUGAPICALL_STATIC(Database, "Auto::open", path);
    // Check for path actually being a file - if so, assume it to be
    // a stub database.
    if (file_exists(path)) {
	return Auto::open_stub(path);
    }

#ifdef MUS_BUILD_BACKEND_QUARTZ
    if (file_exists(path + "/record_DB")) {
	return Quartz::open(path);
    }
#endif
#ifdef MUS_BUILD_BACKEND_MUSCAT36
    if (file_exists(path + "/R") && file_exists(path + "/T")) {
	// can't easily tell flimsy from heavyduty so assume hd
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	return Muscat36::open_da(path + "/R", path + "/T", keyfile, true);
    }
    if (file_exists(path + "/DB")) {
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	return Muscat36::open_db(path + "/DB", keyfile);
    }
    if (file_exists(path + "/DB.da")) {
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	return Muscat36::open_db(path + "/DB.da", keyfile);
    }
#endif

    throw FeatureUnavailableError("Couldn't detect type of database");
}

WritableDatabase
Auto::open(const std::string &path, int action)
{
    DEBUGAPICALL_STATIC(WritableDatabase, "Auto::open", path << ", " << action);
#ifdef MUS_BUILD_BACKEND_QUARTZ
    // Only quartz currently supports disk-based writable databases - if other
    // writable backends are added then this code needs to look at action and
    // perhaps autodetect.
    return Quartz::open(path, action);
#else
    throw FeatureUnavailableError("No disk-based writable backend is enabled");
#endif
}

///////////////////////////////////////////////////////////////////////////////

Database::Internal::Internal()
	: session_in_progress(false),
	  transaction_in_progress(false)
{
}

Database::Internal::~Internal()
{
    // Can't end_session() here because derived class destructors have already
    // run, and the derived classes therefore don't exist.  Thus, the
    // derived classes have responsibility for ending outstanding sessions
    // (by calling internal_end_session()):  let's check they did their job.
    Assert(!session_in_progress);
}

void
Database::Internal::Internal::keep_alive() const
{
    /* For the normal case of local databases, nothing needs to be done.
     */
}

void
Database::Internal::ensure_session_in_progress()
{
    if (!session_in_progress) {
	do_begin_session();
	session_in_progress = true;
    }
}

void
Database::Internal::internal_end_session()
{
    if (!session_in_progress) return;

    if (transaction_in_progress) {
	try {
	    transaction_in_progress = false;
	    do_cancel_transaction();
	} catch (...) {
	    session_in_progress = false;
	    try {
		do_end_session();
	    } catch (...) {
		// Discard exception - we want to re-throw the first error
		// which occured
	    }
	    throw;
	}
    }

    session_in_progress = false;
    do_end_session();
}

void
Database::Internal::flush()
{
    if (session_in_progress) {
	do_flush();
    }
}

void
Database::Internal::begin_transaction()
{
    ensure_session_in_progress();
    if (transaction_in_progress)
	throw InvalidOperationError("Cannot begin transaction - transaction already in progress");
    do_begin_transaction();
    transaction_in_progress = true;
}

void
Database::Internal::commit_transaction()
{
    if (!transaction_in_progress)
	throw InvalidOperationError("Cannot commit transaction - no transaction currently in progress");
    transaction_in_progress = false;
    Assert(session_in_progress);
    do_commit_transaction();
}

void
Database::Internal::cancel_transaction()
{
    if (!transaction_in_progress)
	throw InvalidOperationError("Cannot cancel transaction - no transaction currently in progress");
    transaction_in_progress = false;
    Assert(session_in_progress);
    do_cancel_transaction();
}

Xapian::docid
Database::Internal::add_document(const Xapian::Document & document)
{
    ensure_session_in_progress();
    return do_add_document(document);
}

void
Database::Internal::delete_document(Xapian::docid did)
{
    ensure_session_in_progress();
    do_delete_document(did);
}

void
Database::Internal::replace_document(Xapian::docid did, const Xapian::Document & document)
{
    ensure_session_in_progress();
    do_replace_document(did, document);
}

}
