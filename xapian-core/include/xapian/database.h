/** \file database.h
 * \brief API for working with Xapian databases
 */
/* ----START-LICENCE----
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

#ifndef XAPIAN_INCLUDED_DATABASE_H
#define XAPIAN_INCLUDED_DATABASE_H

#include <string>
#include <vector>

#include <xapian/base.h>
#include <xapian/types.h>

namespace Xapian {
    
class Document;
class PositionListIterator;
class PostListIterator;
class TermIterator;
class WritableDatabase;

/** This class is used to access a database, or a set of databases.
 *
 *  This class is used in conjunction with an Enquire object.
 *
 *  @exception InvalidArgumentError will be thrown if an invalid
 *  argument is supplied, for example, an unknown database type.
 *
 *  @exception OpeningError may be thrown if the database cannot
 *  be opened (for example, a required file cannot be found).
 */
class Database {
    public:
	/** Add an existing database (or group of databases) to those
	 *  accessed by this object.
	 *
	 *  @param database the database(s) to add.
	 */
	void add_database(const Database & database);
    
    public:
	class Internal;
	/// @internal Reference counted internals.
	std::vector<Xapian::Internal::RefCntPtr<Internal> > internal;

    public:
	/** Create a Database with no databases in.
	 */
	Database();

	/** @internal Create a Database from its internals.
	 */
	Database(Internal *internal);

	/** Destroy this handle on the database.
	 *
	 *  If there are no copies of this object remaining, the database
	 *  will be closed.
	 */
	virtual ~Database();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is cheap.
	 */
	Database(const Database &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is cheap.
	 */
	virtual void operator=(const Database &other);

	/** Re-open the database.
	 *  This re-opens the database(s) to the latest available version(s).
	 *  It can be used either to make sure the latest results are
	 *  returned, or to recover from a Xapian::DatabaseModifiedError.
	 */
	void reopen();

	/** Introspection method.
	 *
	 *  @return A string describing this object.
	 */
	virtual std::string get_description() const;

	/** An iterator pointing to the start of the postlist
	 *  for a given term.
	 */
	PostListIterator postlist_begin(const std::string &tname) const;

	/** Corresponding end iterator to postlist_begin()
	 */
	PostListIterator postlist_end(const std::string &tname) const;

	/** An iterator pointing to the start of the termlist
	 *  for a given document.
	 */
	TermIterator termlist_begin(om_docid did) const;
	
	/** Corresponding end iterator to termlist_begin()
	 */
	TermIterator termlist_end(om_docid did) const;

	/** An iterator pointing to the start of the position list
	 *  for a given term in a given document.
	 */
	PositionListIterator positionlist_begin(om_docid did, const std::string &tname) const;

	/** Corresponding end iterator to positionlist_begin()
	 */
	PositionListIterator positionlist_end(om_docid did, const std::string &tname) const;

	/** An iterator which runs across all terms in the database.
	 */
	TermIterator allterms_begin() const;

	/** Corresponding end iterator to allterms_begin()
	 */
	TermIterator allterms_end() const;

	/// Get the number of documents in the database.
	om_doccount get_doccount() const;
	
	/// Get the average length of the documents in the database.
	om_doclength get_avlength() const;

	/// Get the number of documents in the database indexed by a given term.
	om_doccount get_termfreq(const std::string & tname) const;

	/** Check if a given term exists in the database.
	 *
	 *  Return true if and only if the term exists in the database.
	 *  This is the same as (get_termfreq(tname) != 0), but will often be
	 *  more efficient.
	 */
	bool term_exists(const std::string & tname) const;

	/** Return the total number of occurrences of the given term.
	 *
	 *  This is the sum of the number of ocurrences of the term in each
	 *  document: ie, the sum of the within document frequencies of the
	 *  term.
	 *
	 *  @param tname  The term whose collection frequency is being
	 *  requested.
	 */
	om_termcount get_collection_freq(const std::string & tname) const;

	/** Get the length of a document.
	 */
	om_doclength get_doclength(om_docid did) const;

	/** Send a "keep-alive" to remote databases to stop them timing
	 *  out.
	 */
	void keep_alive();

	/** Get a document from the database, given its document id.
	 *
	 *  This method returns a Xapian::Document object which provides the
	 *  information about a document.
	 *
	 *  @param did   The document id for which to retrieve the data.
	 *
	 *  @return      A Xapian::Document object containing the document data
	 *
	 *  @exception Xapian::DocNotFoundError      The document specified
	 *		could not be found in the database.
	 */
	Xapian::Document get_document(om_docid did) const;
};

/** This class provides read/write access to a database.
 */
class WritableDatabase : public Database {
    public:
	/** Destroy this handle on the database.
	 *
	 *  If there are no copies of this object remaining, the database
	 *  will be closed, and if there are any sessions or transactions
	 *  in progress these will be ended.
	 */
	virtual ~WritableDatabase();

	/** Create an empty WritableDatabase.
	 */
	WritableDatabase();

	/** @internal Create an WritableDatabase given its internals.
	 */
	WritableDatabase(Database::Internal *internal);

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is cheap.
	 */
	WritableDatabase(const WritableDatabase &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is cheap.
	 *
	 *  Note that only an WritableDatabase may be assigned to an
	 *  WritableDatabase: an attempt to assign a Database will throw
	 *  an exception (FIXME: is this actually possible to do?)
	 */
	void operator=(const WritableDatabase &other);

	/** Flush to disk any modifications made to the database.
	 *
	 *  For efficiency reasons, when performing multiple updates to a
	 *  database it is best (indeed, almost essential) to make as many
	 *  modifications as memory will permit in a single pass through
	 *  the database.  To ensure this, Xapian performs modifications in
	 *  "sessions".  Sessions are begun and ended implicitly, when the
	 *  database is first modified, or closed.
	 *
	 *  Flush may be called at any time during a modification session to
	 *  ensure that the modifications which have been made are written to
	 *  disk: if the flush succeeds, all the preceding modifications will
	 *  have been written to disk.
	 *
	 *  If any of the modifications fail, an exception will be thrown and
	 *  the database will be left in a state in which each separate
	 *  addition, replacement or deletion operation has either been fully
	 *  performed or not performed at all: it is then up to the
	 *  application to work out which operations need to be repeated.
	 *
	 *  If called within a transaction, this will flush database
	 *  modifications made before the transaction was begun, but will
	 *  not flush modifications made since begin_transaction() was
	 *  called.
	 *
	 *  Beware of calling flush too frequently: this will have a severe
	 *  performance cost.
	 *
	 *  Note that flush need not be called explicitly: it will be called
	 *  automatically when the database is closed, or when a sufficient
	 *  number of modifications have been made.
	 *
	 *  @exception Xapian::DatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception Xapian::DatabaseLockError will be thrown if a lock
	 *	       couldn't be acquired on the database.
	 */
	void flush();

	/** Begin a transaction.
	 *
	 *  For the purposes of Xapian, a transaction is a group of
	 *  modifications to the database which are grouped together such
	 *  that either all or none of them will succeed.  Even in the case
	 *  of a power failure, this characteristic should be preserved (as
	 *  long as the filesystem isn't corrupted, etc).
	 *
	 *  Transactions are only available with certain access methods,
	 *  and as you might expect will generally have a fairly high
	 *  performance cost.
	 *
	 *  A transaction may only be begun within a session, see
	 *  begin_session().
	 * 
	 *  @exception Xapian::UnimplementedError will be thrown if transactions
	 *             are not available for this database type.
	 *
	 *  @exception Xapian::InvalidOperationError will be thrown if this is
	 *             called at an invalid time, such as when a transaction
	 *             is already in progress.
	 */
	void begin_transaction();

	/** End the transaction currently in progress, committing the
	 *  modifications made to the database.
	 *
	 *  If this completes successfully, all the database modifications
	 *  made during the transaction will have been committed to the
	 *  database.
	 *
	 *  If an error occurs, an exception will be thrown, and none of
	 *  the modifications made to the database during the transaction
	 *  will have been applied to the database.
	 *  
	 *  Whatever occurs, after this method the transaction will no
	 *  longer be in progress.
	 *
	 *  @exception Xapian::DatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception Xapian::InvalidOperationError will be thrown if a
	 *	       transaction is not currently in progress.
	 *
	 *  @exception Xapian::UnimplementedError will be thrown if transactions
	 *             are not available for this database type.
	 */
	void commit_transaction();

	/** End the transaction currently in progress, cancelling the
	 *  potential modifications made to the database.
	 *
	 *  If an error occurs in this method, an exception will be thrown,
	 *  but the transaction will be cancelled anyway.
	 *
	 *  @exception Xapian::DatabaseError will be thrown if a problem occurs
	 *             while modifying the database.
	 *  
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception Xapian::InvalidOperationError will be thrown if a
	 *	       transaction is not currently in progress.
	 *
	 *  @exception Xapian::UnimplementedError will be thrown if transactions
	 *             are not available for this database type.
	 */
	void cancel_transaction();

	/** Add a new document to the database.
	 *
	 *  This method adds the specified document to the database,
	 *  returning a newly allocated document ID.
	 *
	 *  Note that this does not mean the document will immediately
	 *  appear in the database; see flush() for more details.
	 *
	 *  As with all database modification operations, the effect is
	 *  atomic: the document will either be fully added, or the document
	 *  fails to be added and an exception is thrown (possibly at a
	 *  later time when the session is ended or flushed).
	 *
	 *  If a session is not in progress when this method is called, a
	 *  session will be started.
	 *
	 *  @param document The new document to be added.
	 *
	 *  @return         The document ID of the newly added document.
	 *
	 *  @exception Xapian::DatabaseError will be thrown if a problem occurs
	 *             while writing to the database.
	 *
	 *  @exception Xapian::DatabaseCorruptError will be thrown if the
	 *             database is in a corrupt state.
	 *
	 *  @exception Xapian::DatabaseLockError will be thrown if a lock
	 *  couldn't be acquired or subsequently released on the database.
	 */
	om_docid add_document(const Xapian::Document & document);

	/** Delete a document in the database.
	 */
	// FIXME: document more.
	void delete_document(om_docid did);

	/** Replace a given document in the database.
	 */
	// FIXME: document more.
	void replace_document(om_docid did, const Xapian::Document & document);

	/** Introspection method.
	 *
	 *  @return A string describing this object.
	 */
	std::string get_description() const;
};

const int DB_CREATE_OR_OPEN = 1;
const int DB_CREATE = 2;
const int DB_CREATE_OR_OVERWRITE = 3;
const int DB_OPEN = 4;
// Can't see any sensible use for this one
// const int DB_OVERWRITE = XXX;

/* It's mostly harmless to provide prototypes for all the backends, even
 * if they may not all be built in - it means the failure will be at
 * link time rather than when a file is built.  The main benefit is
 * simplicity, but this also allows us to easily split the backends into
 * separate libraries and link the ones we actually use in an application.
 */

namespace Auto {
/** Open a database read-only, automatically determining the database
 *  backend to use.
 *
 * @param path directory that the database is stored in.
 */
Database open(const std::string &path);

/** Open a database for update, automatically determining the database
 *  backend to use.
 *
 * @param path directory that the database is stored in.
 * @param action one of:
 *  - Xapian::DB_CREATE_OR_OPEN open for read/write; create if no db exists
 *  - Xapian::DB_CREATE create new database; fail if db exists
 *  - Xapian::DB_CREATE_OR_OVERWRITE overwrite existing db; create if none exists
 *  - Xapian::DB_OPEN open for read/write; fail if no db exists
 */
WritableDatabase open(const std::string &path, int action);

/** Open a stub database.
 *
 * This opens a file which contains types and serialised parameters for one
 * or more databases.
 *
 * @param file the stub database file
 */
Database open_stub(const std::string &file);

}

namespace Quartz {
/** Open a Quartz database read-only.
 *
 * @param dir directory that the database is stored in.
 */
Database open(const std::string &dir);

/** Open a Quartz database for update.
 *
 * @param dir directory that the database is stored in.
 * @param action one of:
 *  - Xapian::DB_CREATE_OR_OPEN open for read/write; create if no db exists
 *  - Xapian::DB_CREATE create new database; fail if db exists
 *  - Xapian::DB_CREATE_OR_OVERWRITE overwrite existing db; create if none exists
 *  - Xapian::DB_OPEN open for read/write; fail if no db exists
 * @param block_size the size of the blocks to use in
 *                 the tables, in bytes.  Acceptable values are powers of
 *                 two in the range 2048 to 65536.  The default is 8192.
 *                 This setting is only used when creating databases.  If
 *                 the database already exists, it is completely ignored.
 */
WritableDatabase
open(const std::string &dir, int action, int block_size = 8192);

}

namespace InMemory {
/** Open an InMemory database for update.
 */
WritableDatabase open();
}

namespace Muscat36 {
/** Open a Muscat 3.6 DA database.
 *
 * This opens a DA database with no values file.
 *
 * @param R filename of the Record file
 * @param T filename of the Term file
 * @param heavy_duty is this database heavy-duty (3 byte lengths) or flimsy (2
 * byte lengths)
 */
Database open_da(const std::string &R, const std::string &T, bool heavy_duty = true);

/** Open a Muscat 3.6 DA database.
 *
 * This opens a DA database with a values file.
 *
 * @param R filename of the Record file
 * @param T filename of the Term file
 * @param values filename of the values file
 * @param heavy_duty is this database heavy-duty (3 byte lengths) or flimsy (2
 * byte lengths)
 */
Database open_da(const std::string &R, const std::string &T, const std::string &values, bool heavy_duty = true);

/** Open a Muscat 3.6 DB database.
 *
 * This opens a DB database with no values file.  The backend auto-detects
 * if the database is heavy-duty or flimsy.
 *
 * @param DB filename of the database btree file
 * @param cache_size how many blocks to cache
 */
Database open_db(const std::string &DB, size_t cache_size = 30);

/** Open a Muscat 3.6 DB database.
 *
 * This opens a DB database with a values file.  The backend auto-detects
 * if the database is heavy-duty or flimsy.
 *
 * @param DB filename of the database btree file
 * @param values filename of the values file
 * @param cache_size how many blocks to cache (default 30).
 */
Database open_db(const std::string &DB, const std::string &values = "", size_t cache_size = 30);
}

namespace Remote {
/** Open a remote database (using a program).
 *
 * This opens a remote database by running a program which it communicates
 * with on stdin/stdout.
 *
 * @param program the program to run
 * @param arguments the arguments to pass to the program
 * @param timeout how long to wait for a response (in milliseconds).
 *  If this timeout is reached for any operation, then a
 *  Xapian::NetworkTimeoutError exception will be thrown.  The default if not
 *  specified is 10000ms (i.e. 10 seconds).
 */
Database open(const std::string &program, const std::string &args,
	Xapian::timeout timeout = 10000);

/** Open a remote database (using a TCP connection).
 *
 * This opens a remote database by connecting to the specified TCP port on
 * the specified host.
 *
 * @param host the name of the host running a tcp server
 * @param port the port on which the tcp server is running
 * @param timeout how long to wait for a response (in milliseconds).
 *  If this timeout is reached for any operation, then a Xapian::NetworkTimeoutError
 *  exception will be thrown.  The default if not specified is 10000ms
 *  (10 seconds).
 * @param connect_timeout how long to wait when attempting to connect to
 *  the server.  If this timeout is reached when attempting to connect, then
 *  a Xapian::NetworkTimeoutError exception wil be thrown.  The default if not
 *  specified is to use the same value given for timeout.
 */
Database
open(const std::string &host, unsigned int port,
	Xapian::timeout timeout = 10000, Xapian::timeout connect_timeout = 0);
}

}

#endif /* XAPIAN_INCLUDED_DATABASE_H */
