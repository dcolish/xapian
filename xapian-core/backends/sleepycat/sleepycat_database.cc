/* sleepy_database.cc: interface to sleepycat database routines */


#include "omassert.h"
#include "sleepy_database.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

// Sleepycat database stuff
#include <db_cxx.h>

#define FILENAME_POSTLIST "postlist.db"
#define FILENAME_TERMLIST "termlist.db"
#define FILENAME_TERMTOID "termid.db"
#define FILENAME_IDTOTERM "termname.db"

/////////////////////////////
// Internal database state //
/////////////////////////////

class SleepyDatabaseInternals {
    private:
	DbEnv dbenv;
	bool opened;
    public:
	Db *postlist_db;
	Db *termlist_db;
	Db *termid_db;
	Db *termname_db;

	SleepyDatabaseInternals();
	~SleepyDatabaseInternals();
	void open(const string &, bool);
	void close();
};

inline
SleepyDatabaseInternals::SleepyDatabaseInternals() {
    postlist_db = NULL;
    termlist_db = NULL;
    termid_db = NULL;
    termname_db = NULL;
    opened = false;
}

inline
SleepyDatabaseInternals::~SleepyDatabaseInternals(){
    close();
}

inline void
SleepyDatabaseInternals::open(const string &pathname, bool readonly)
{
    // Set up environment
    u_int32_t flags = DB_INIT_CDB;
    int mode = 0;

    if(readonly) {
	flags = DB_RDONLY;
    } else {
	flags = DB_CREATE;
	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    }
    dbenv.appinit(pathname.c_str(), NULL, flags);
    opened = true;

    Db::open(FILENAME_POSTLIST, DB_BTREE, flags, mode,
	     &dbenv, NULL, &postlist_db);

    Db::open(FILENAME_TERMLIST, DB_BTREE, flags, mode,
	     &dbenv, NULL, &termlist_db);

    Db::open(FILENAME_TERMTOID, DB_HASH, flags, mode,
	     &dbenv, NULL, &termlist_db);

    Db::open(FILENAME_IDTOTERM, DB_RECNO, flags, mode,
	     &dbenv, NULL, &termlist_db);
}

inline void
SleepyDatabaseInternals::close()
{
    if(postlist_db) postlist_db->close(0);
    postlist_db = NULL;
    if(termlist_db) termlist_db->close(0);
    termlist_db = NULL;
    if(termid_db) termid_db->close(0);
    termid_db = NULL;
    if(termname_db) termname_db->close(0);
    termname_db = NULL;

    if(opened) dbenv.appexit();
}

///////////////
// Postlists //
///////////////

SleepyPostList::SleepyPostList(docid *data_new, doccount termfreq_new) {
    pos = 0;
    data = data_new;
    termfreq = termfreq_new;
}


SleepyPostList::~SleepyPostList() {
    free(data);
}

weight SleepyPostList::get_weight() const {
    return 1;
}

///////////////
// Termlists //
///////////////

SleepyTermList::SleepyTermList(termid *data_new, termcount terms_new) {
    pos = 0;
    data = data_new;
    terms = terms_new;
}

SleepyTermList::~SleepyTermList() {
    free(data);
}

///////////////////////////
// Actual database class //
///////////////////////////

SleepyDatabase::SleepyDatabase() {
    internals = new SleepyDatabaseInternals();
}

SleepyDatabase::~SleepyDatabase() {
    delete internals;
}

void SleepyDatabase::open(const string &pathname, bool readonly) {
    // Open databases
    // FIXME - catch exceptions
    internals->open(pathname, readonly);
}

void SleepyDatabase::close() {
    // Close databases
    // FIXME - catch exceptions
    internals->close();
}

PostList *
SleepyDatabase::open_post_list(termid id) {
    Dbt key(&id, sizeof(id));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("PostlistDb error:" + string(e.what()));
    }

    return new SleepyPostList((docid *)data.get_data(),
			      data.get_size() / sizeof(docid));
}

TermList *
SleepyDatabase::open_term_list(docid id) {
    Dbt key(&id, sizeof(id));
    Dbt data;

    // FIXME - should use DB_DBT_USERMEM and DB_DBT_PARTIAL eventually
    data.set_flags(DB_DBT_MALLOC);

    // Get, no transactions, no flags
    try {
	int found = internals->postlist_db->get(NULL, &key, &data, 0);
	if(found == DB_NOTFOUND) throw RangeError("Termid not found");

	// Any other errors should cause an exception.
	Assert(found == 0);
    }
    catch (DbException e) {
	throw OmError("TermlistDb error:" + string(e.what()));
    }

    return new SleepyTermList((termid *)data.get_data(),
			      data.get_size() / sizeof(termid));
}

termid
SleepyDatabase::term_name_to_id(const termname &) {
    return 1;
}

termname
SleepyDatabase::term_id_to_name(termid) {
    return "a";
}
