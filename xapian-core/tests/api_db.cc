/* api_db.cc: tests which need a backend
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 * Copyright 2006 Richard Boulton
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

#include <config.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

// We have to use the deprecated Quartz::open() method.
#define XAPIAN_DEPRECATED(D) D
#include <xapian.h>

#include "backendmanager.h"
#include "testsuite.h"
#include "testutils.h"
#include "unixcmds.h"
#include "utils.h"

#include "apitest.h"

#include "safefcntl.h"
#include "safeunistd.h"
#include "safeerrno.h"

#include <list>

using namespace std;

static Xapian::Query
query(const string &t)
{
    return Xapian::Query(Xapian::Stem("english")(t));
}

// #######################################################################
// # Tests start here

// tests Xapian::Database::get_termfreq() and Xapian::Database::term_exists()
static bool test_termstats()
{
    // open the database (in this case a simple text file
    // we prepared earlier)

    Xapian::Database db(get_database("apitest_simpledata"));

    TEST(!db.term_exists("corn"));
    TEST(db.term_exists("paragraph"));
    TEST_EQUAL(db.get_termfreq("banana"), 1);
    TEST_EQUAL(db.get_termfreq("paragraph"), 5);

    return true;
}

// check that stubdbs work
static bool test_stubdb1()
{
    ofstream out("stubdb1");
    TEST(out.is_open());
    // FIXME: not very reliable...
    out << "remote :" << BackendManager::get_xapian_progsrv_command()
	<< " .flint/db=apitest_simpledata\n";
    out.close();

    {
	Xapian::Database db = Xapian::Auto::open_stub("stubdb1");
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	enquire.get_mset(0, 10);
    }
    {
	Xapian::Database db("stubdb1");
	Xapian::Enquire enquire(db);
	enquire.set_query(Xapian::Query("word"));
	enquire.get_mset(0, 10);
    }

    unlink("stubdb1");

    return true;
}

#if 0 // the "force error" mechanism is no longer in place...
class MyErrorHandler : public Xapian::ErrorHandler {
    public:
	int count;

	bool handle_error(Xapian::Error & error) {
	    ++count;
	    tout << "Error handling caught: " << error.get_description()
		 << ", count is now " << count << "\n";
	    return true;
	}

	MyErrorHandler() : count (0) {}
};

// tests error handler in multimatch().
static bool test_multierrhandler1()
{
    MyErrorHandler myhandler;

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    Xapian::Database mydb3(get_database("apitest_simpledata2"));
    int errcount = 1;
    for (int testcount = 0; testcount < 14; testcount ++) {
	tout << "testcount=" << testcount << "\n";
	Xapian::Database mydb4(get_database("-e", "apitest_termorder"));
	Xapian::Database mydb5(get_network_database("apitest_termorder", 1));
	Xapian::Database mydb6(get_database("-e2", "apitest_termorder"));
	Xapian::Database mydb7(get_database("-e3", "apitest_simpledata"));

	Xapian::Database dbs;
	switch (testcount) {
	    case 0:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb4);
		break;
	    case 1:
		dbs.add_database(mydb4);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 2:
		dbs.add_database(mydb3);
		dbs.add_database(mydb4);
		dbs.add_database(mydb2);
		break;
	    case 3:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb5);
		sleep(1);
		break;
	    case 4:
		dbs.add_database(mydb5);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		sleep(1);
		break;
	    case 5:
		dbs.add_database(mydb3);
		dbs.add_database(mydb5);
		dbs.add_database(mydb2);
		sleep(1);
		break;
	    case 6:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb6);
		break;
	    case 7:
		dbs.add_database(mydb6);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 8:
		dbs.add_database(mydb3);
		dbs.add_database(mydb6);
		dbs.add_database(mydb2);
		break;
	    case 9:
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		dbs.add_database(mydb7);
		break;
	    case 10:
		dbs.add_database(mydb7);
		dbs.add_database(mydb2);
		dbs.add_database(mydb3);
		break;
	    case 11:
		dbs.add_database(mydb3);
		dbs.add_database(mydb7);
		dbs.add_database(mydb2);
		break;
	    case 12:
		dbs.add_database(mydb2);
		dbs.add_database(mydb6);
		dbs.add_database(mydb7);
		break;
	    case 13:
		dbs.add_database(mydb2);
		dbs.add_database(mydb7);
		dbs.add_database(mydb6);
		break;
	}
	tout << "db=" << dbs << "\n";
	Xapian::Enquire enquire(dbs, &myhandler);

	// make a query
	Xapian::Query myquery = query(Xapian::Query::OP_OR, "inmemory", "word");
	enquire.set_weighting_scheme(Xapian::BoolWeight());
	enquire.set_query(myquery);

	tout << "query=" << myquery << "\n";
	// retrieve the top ten results
	Xapian::MSet mymset = enquire.get_mset(0, 10);

	switch (testcount) {
	    case 0: case 3: case 6: case 9:
		mset_expect_order(mymset, 2, 4, 10);
		break;
	    case 1: case 4: case 7: case 10:
		mset_expect_order(mymset, 3, 5, 11);
		break;
	    case 2: case 5: case 8: case 11:
		mset_expect_order(mymset, 1, 6, 12);
		break;
	    case 12:
	    case 13:
		mset_expect_order(mymset, 4, 10);
		errcount += 1;
		break;
	}
	TEST_EQUAL(myhandler.count, errcount);
	errcount += 1;
    }

    return true;
}
#endif

class myMatchDecider : public Xapian::MatchDecider {
    public:
	bool operator()(const Xapian::Document &doc) const {
	    // Note that this is not recommended usage of get_data()
	    return doc.get_data().find("This is") != string::npos;
	}
};

// tests the match decision functor
static bool test_matchfunctor1()
{
    // FIXME: check that the functor works both ways.
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    myMatchDecider myfunctor;

    Xapian::MSet mymset = enquire.get_mset(0, 100, 0, &myfunctor);

    Xapian::MSetIterator i = mymset.begin();
    TEST(i != mymset.end());
    TEST_EQUAL(mymset.size(), 3);
    for ( ; i != mymset.end(); ++i) {
	const Xapian::Document doc(i.get_document());
	TEST(myfunctor(doc));
    }

    return true;
}

// tests that mset iterators on msets compare correctly.
static bool test_msetiterator1()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 2);

    Xapian::MSetIterator j;
    j = mymset.begin();
    Xapian::MSetIterator k = mymset.end();
    Xapian::MSetIterator l(j);
    Xapian::MSetIterator m(k);
    Xapian::MSetIterator n = mymset.begin();
    Xapian::MSetIterator o = mymset.begin();
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    k = j;
    TEST_EQUAL(j, k);
    TEST_EQUAL(j, o);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_NOT_EQUAL(k, m);
    TEST_NOT_EQUAL(k, o);
    o++;
    TEST_EQUAL(k, o);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_EQUAL(k, m);
    TEST_EQUAL(n, l);

    n = m;
    TEST_NOT_EQUAL(n, l);
    TEST_EQUAL(n, m);
    TEST_NOT_EQUAL(n, mymset.begin());
    TEST_EQUAL(n, mymset.end());

    return true;
}

// tests that mset iterators on empty msets compare equal.
static bool test_msetiterator2()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));
    Xapian::MSet mymset = enquire.get_mset(0, 0);

    Xapian::MSetIterator j = mymset.begin();
    Xapian::MSetIterator k = mymset.end();
    Xapian::MSetIterator l(j);
    Xapian::MSetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

// tests that begin().get_document() works when first != 0
static bool test_msetiterator3()
{
    Xapian::Database mydb(get_database("apitest_simpledata"));
    Xapian::Enquire enquire(mydb);
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(2, 10);

    TEST(!mymset.empty());
    Xapian::Document doc(mymset.begin().get_document());
    TEST(!doc.get_data().empty());

    return true;
}

// tests that eset iterators on empty esets compare equal.
static bool test_esetiterator1()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(2, myrset);
    Xapian::ESetIterator j;
    j = myeset.begin();
    Xapian::ESetIterator k = myeset.end();
    Xapian::ESetIterator l(j);
    Xapian::ESetIterator m(k);
    Xapian::ESetIterator n = myeset.begin();

    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    k = j;
    TEST_EQUAL(j, k);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_NOT_EQUAL(k, m);
    k++;
    TEST_NOT_EQUAL(j, k);
    TEST_NOT_EQUAL(k, l);
    TEST_EQUAL(k, m);
    TEST_EQUAL(n, l);

    n = m;
    TEST_NOT_EQUAL(n, l);
    TEST_EQUAL(n, m);
    TEST_NOT_EQUAL(n, myeset.begin());
    TEST_EQUAL(n, myeset.end());

    return true;
}

// tests that eset iterators on empty esets compare equal.
static bool test_esetiterator2()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset = enquire.get_mset(0, 10);
    TEST(mymset.size() >= 2);

    Xapian::RSet myrset;
    Xapian::MSetIterator i = mymset.begin();
    myrset.add_document(*i);
    myrset.add_document(*(++i));

    Xapian::ESet myeset = enquire.get_eset(0, myrset);
    Xapian::ESetIterator j = myeset.begin();
    Xapian::ESetIterator k = myeset.end();
    Xapian::ESetIterator l(j);
    Xapian::ESetIterator m(k);
    TEST_EQUAL(j, k);
    TEST_EQUAL(l, m);
    TEST_EQUAL(k, m);
    TEST_EQUAL(j, l);
    TEST_EQUAL(j, j);
    TEST_EQUAL(k, k);

    return true;
}

// tests the collapse-on-key
static bool test_collapsekey1()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    Xapian::doccount mymsize1 = mymset1.size();

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 100);

	TEST_AND_EXPLAIN(mymsize1 > mymset.size(),
			 "Had no fewer items when performing collapse: don't know whether it worked.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately.
static bool test_collapsekey3()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 3);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 3);

	TEST_AND_EXPLAIN(mymset1.get_matches_lower_bound() > mymset.get_matches_lower_bound(),
			 "Lower bound was not lower when performing collapse: don't know whether it worked.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() > mymset.get_matches_upper_bound(),
			 "Upper bound was not lower when performing collapse: don't know whether it worked.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    // Test that, if no duplicates are found (eg, by collapsing on key 1000,
    // which has no entries), the upper bound stays the same, but the lower
    // bound drops.
    {
	Xapian::valueno value_no = 1000;
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 3);

	TEST_AND_EXPLAIN(mymset1.get_matches_lower_bound() > mymset.get_matches_lower_bound(),
			 "Lower bound was not lower when performing collapse: don't know whether it worked.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() == mymset.get_matches_upper_bound(),
			 "Upper bound was not equal when collapse turned on, but no duplicates found.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    return true;
}

// tests that collapse-on-key modifies the predicted bounds for the number of
// matches appropriately even when no results are requested.
static bool test_collapsekey4()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    Xapian::MSet mymset1 = enquire.get_mset(0, 0);

    for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	enquire.set_collapse_key(value_no);
	Xapian::MSet mymset = enquire.get_mset(0, 0);

	TEST_AND_EXPLAIN(mymset.get_matches_lower_bound() == 1,
			 "Lower bound was not 1 when performing collapse but not asking for any results.");
	TEST_AND_EXPLAIN(mymset1.get_matches_upper_bound() == mymset.get_matches_upper_bound(),
			 "Upper bound was changed when performing collapse but not asking for any results.");

	map<string, Xapian::docid> values;
	Xapian::MSetIterator i = mymset.begin();
	for ( ; i != mymset.end(); ++i) {
	    string value = i.get_document().get_value(value_no);
	    TEST(values[value] == 0 || value == "");
	    values[value] = *i;
	}
    }

    return true;
}

// test for keepalives
static bool test_keepalive1()
{
    Xapian::Database db(get_network_database("apitest_simpledata", 5000));

    /* Test that keep-alives work */
    for (int i = 0; i < 10; ++i) {
	sleep(2);
	db.keep_alive();
    }
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query("word"));
    enquire.get_mset(0, 10);

    /* Test that things break without keepalives */
    sleep(10);
    enquire.set_query(Xapian::Query("word"));
    TEST_EXCEPTION(Xapian::NetworkError,
		   enquire.get_mset(0, 10));

    return true;
}

// test that iterating through all terms in a database works.
static bool test_allterms1()
{
    Xapian::Database db(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    Xapian::TermIterator ati2 = ati;

    ati++;
    TEST(ati != db.allterms_end());
    if (verbose) {
	tout << "*ati = `" << *ati << "'\n";
	tout << "*ati.length = `" << (*ati).length() << "'\n";
	tout << "*ati == \"one\" = " << (*ati == "one") << "\n";
	tout << "*ati[3] = " << ((*ati)[3]) << "\n";
	tout << "*ati = `" << *ati << "'\n";
    }
    TEST(*ati == "three");
    TEST(ati.get_termfreq() == 3);

#if 0
    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "one");
    TEST(ati2.get_termfreq() == 1);
#endif

    ++ati;
#if 0
    ++ati2;
#endif
    TEST(ati != db.allterms_end());
    TEST(*ati == "two");
    TEST(ati.get_termfreq() == 2);

#if 0
    TEST(ati2 != db.allterms_end());
    TEST(*ati2 == "three");
    TEST(ati2.get_termfreq() == 3);
#endif

    ati++;
    TEST(ati == db.allterms_end());

    return true;
}

// test that iterating through all terms in two databases works.
static bool test_allterms2()
{
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    db.add_database(get_database("apitest_allterms2"));
    Xapian::TermIterator ati = db.allterms_begin();

    TEST(ati != db.allterms_end());
    TEST(*ati == "five");
    TEST(ati.get_termfreq() == 2);
    ati++;

    TEST(ati != db.allterms_end());
    TEST(*ati == "four");
    TEST(ati.get_termfreq() == 1);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "one");
    TEST(ati.get_termfreq() == 1);

    ++ati;
    TEST(ati != db.allterms_end());
    TEST(*ati == "six");
    TEST(ati.get_termfreq() == 3);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "three");
    TEST(ati.get_termfreq() == 3);

    ati++;
    TEST(ati != db.allterms_end());
    TEST(*ati == "two");
    TEST(ati.get_termfreq() == 2);

    ati++;
    TEST(ati == db.allterms_end());

    return true;
}

// test that skip_to sets at_end (regression test)
static bool test_allterms3()
{
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();

    ati.skip_to(string("zzzzzz"));
    TEST(ati == db.allterms_end());

    return true;
}

// test that next ignores extra entries due to long posting lists being
// chunked (regression test for quartz)
static bool test_allterms4()
{
    // apitest_allterms4 contains 682 documents each containing just the word
    // "foo".  682 was the magic number which started to cause Quartz problems.
    Xapian::Database db = get_database("apitest_allterms4");

    Xapian::TermIterator i = db.allterms_begin();
    TEST(i != db.allterms_end());
    TEST(*i == "foo");
    TEST(i.get_termfreq() == 682);
    ++i;
    TEST(i == db.allterms_end());

    return true;
}

// test that skip_to with an exact match sets the current term (regression test
// for quartz)
static bool test_allterms5()
{
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    Xapian::TermIterator ati = db.allterms_begin();
    ati.skip_to("three");
    TEST(ati != db.allterms_end());
    TEST_EQUAL(*ati, "three");

    return true;
}

// test allterms iterators with prefixes
static bool test_allterms6()
{
    Xapian::Database db;
    db.add_database(get_database("apitest_allterms"));
    db.add_database(get_database("apitest_allterms2"));

    Xapian::TermIterator ati = db.allterms_begin("three");
    TEST(ati != db.allterms_end("three"));
    TEST_EQUAL(*ati, "three");
    ati.skip_to("three");
    TEST(ati != db.allterms_end("three"));
    TEST_EQUAL(*ati, "three");
    ati++;
    TEST(ati == db.allterms_end("three"));

    ati = db.allterms_begin("thre");
    TEST(ati != db.allterms_end("thre"));
    TEST_EQUAL(*ati, "three");
    ati.skip_to("three");
    TEST(ati != db.allterms_end("thre"));
    TEST_EQUAL(*ati, "three");
    ati++;
    TEST(ati == db.allterms_end("thre"));

    ati = db.allterms_begin("f");
    TEST(ati != db.allterms_end("f"));
    TEST_EQUAL(*ati, "five");
    TEST(ati != db.allterms_end("f"));
    ati.skip_to("three");
    TEST(ati == db.allterms_end("f"));

    ati = db.allterms_begin("f");
    TEST(ati != db.allterms_end("f"));
    TEST_EQUAL(*ati, "five");
    ati++;
    TEST(ati != db.allterms_end("f"));
    TEST_EQUAL(*ati, "four");
    ati++;
    TEST(ati == db.allterms_end("f"));

    return true;
}

// test that searching for a term with a special characters in it works
static bool test_specialterms1()
{
    Xapian::Enquire enquire(get_database("apitest_space"));
    Xapian::MSet mymset;
    Xapian::doccount count;
    Xapian::MSetIterator m;
    Xapian::Stem stemmer("english");

    enquire.set_query(stemmer("new\nline"));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    for (Xapian::valueno value_no = 0; value_no < 7; ++value_no) {
	string value = mymset.begin().get_document().get_value(value_no);
	TEST_NOT_EQUAL(value, "");
	if (value_no == 0) {
	    TEST(value.size() > 263);
	    TEST_EQUAL(static_cast<unsigned char>(value[262]), 255);
	    for (int k = 0; k < 256; k++) {
		TEST_EQUAL(static_cast<unsigned char>(value[k+7]), k);
	    }
	}
    }

    enquire.set_query(stemmer(string("big\0zero", 8)));
    mymset = enquire.get_mset(0, 10);
    TEST_MSET_SIZE(mymset, 1);
    count = 0;
    for (m = mymset.begin(); m != mymset.end(); ++m) ++count;
    TEST_EQUAL(count, 1);

    return true;
}

// test that terms with a special characters in appear correctly when iterating allterms
static bool test_specialterms2()
{
    Xapian::Database db(get_database("apitest_space"));

    // Check the terms are all as expected (after stemming) and that allterms copes with
    // iterating over them.
    Xapian::TermIterator t;
    t = db.allterms_begin();
    TEST_EQUAL(*t, "back\\slash"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, string("big\0zero", 8)); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "new\nlin"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "one\x01on"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "space man"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "tab\tbi"); ++t; TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, "tu\x02tu"); ++t; TEST_EQUAL(t, db.allterms_end());

    // Now check that skip_to exactly a term containing a zero byte works.
    // This is a regression test for flint and quartz - an Assert() used to fire in debug builds
    // (the Assert was wrong - the actual code handled this OK).
    t = db.allterms_begin();
    t.skip_to(string("big\0zero", 8));
    TEST_NOT_EQUAL(t, db.allterms_end());
    TEST_EQUAL(*t, string("big\0zero", 8));

    return true;
}

// test that rsets behave correctly with multiDBs
static bool test_rsetmultidb2()
{
    Xapian::Database mydb1(get_database("apitest_rset", "apitest_simpledata2"));
    Xapian::Database mydb2(get_database("apitest_rset"));
    mydb2.add_database(get_database("apitest_simpledata2"));

    Xapian::Enquire enquire1(mydb1);
    Xapian::Enquire enquire2(mydb2);

    Xapian::Query myquery = query("is");

    enquire1.set_query(myquery);
    enquire2.set_query(myquery);

    Xapian::RSet myrset1;
    Xapian::RSet myrset2;
    myrset1.add_document(4);
    myrset2.add_document(2);

    Xapian::MSet mymset1a = enquire1.get_mset(0, 10);
    Xapian::MSet mymset1b = enquire1.get_mset(0, 10, &myrset1);
    Xapian::MSet mymset2a = enquire2.get_mset(0, 10);
    Xapian::MSet mymset2b = enquire2.get_mset(0, 10, &myrset2);

    mset_expect_order(mymset1a, 4, 3);
    mset_expect_order(mymset1b, 4, 3);
    mset_expect_order(mymset2a, 2, 5);
    mset_expect_order(mymset2b, 2, 5);

    mset_range_is_same_weights(mymset1a, 0, mymset2a, 0, 2);
    mset_range_is_same_weights(mymset1b, 0, mymset2b, 0, 2);
    TEST_NOT_EQUAL(mymset1a, mymset1b);
    TEST_NOT_EQUAL(mymset2a, mymset2b);

    return true;
}

// tests an expand across multiple databases
static bool test_multiexpand1()
{
    Xapian::Database mydb1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Enquire enquire1(mydb1);

    Xapian::Database mydb2(get_database("apitest_simpledata"));
    mydb2.add_database(get_database("apitest_simpledata2"));
    Xapian::Enquire enquire2(mydb2);

    // make simple equivalent rsets, with a document from each database in each.
    Xapian::RSet rset1;
    Xapian::RSet rset2;
    rset1.add_document(1);
    rset1.add_document(7);
    rset2.add_document(1);
    rset2.add_document(2);

    // retrieve the top ten results from each method of accessing
    // multiple text files

    // This is the single database one.
    Xapian::ESet eset1 = enquire1.get_eset(1000, rset1);

    // This is the multi database with approximation
    Xapian::ESet eset2 = enquire2.get_eset(1000, rset2);

    // This is the multi database without approximation
    Xapian::ESet eset3 = enquire2.get_eset(1000, rset2, Xapian::Enquire::USE_EXACT_TERMFREQ);

    TEST_EQUAL(eset1.size(), eset2.size());
    TEST_EQUAL(eset1.size(), eset3.size());

    Xapian::ESetIterator i = eset1.begin();
    Xapian::ESetIterator j = eset2.begin();
    Xapian::ESetIterator k = eset3.begin();
    bool all_iwts_equal_jwts = true;
    while (i != eset1.end() && j != eset2.end() && k != eset3.end()) {
	if (i.get_weight() != j.get_weight()) all_iwts_equal_jwts = false;
	TEST_EQUAL(i.get_weight(), k.get_weight());
	TEST_EQUAL(*i, *k);
	++i;
	++j;
	++k;
    }
    TEST(i == eset1.end());
    TEST(j == eset2.end());
    TEST(k == eset3.end());
    TEST(!all_iwts_equal_jwts);
    return true;
}

// tests that opening a non-existent postlist return an empty list
static bool test_postlist1()
{
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST_EQUAL(db.postlist_begin("rosebud"), db.postlist_end("rosebud"));

    string s = "let_us_see_if_we_can_break_it_with_a_really_really_long_term.";
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    s += s;
    TEST_EQUAL(db.postlist_begin(s), db.postlist_end(s));

    // a regression test (no, really)
    TEST_NOT_EQUAL(db.postlist_begin("a"), db.postlist_end("a"));

    return true;
}

// tests that a Xapian::PostingIterator works as an STL iterator
static bool test_postlist2()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator p;
    p = db.postlist_begin("this");
    Xapian::PostingIterator pend = db.postlist_end("this");

    // test operator= creates a copy which compares equal
    Xapian::PostingIterator p_copy = p;
    TEST_EQUAL(p, p_copy);

    // test copy constructor creates a copy which compares equal
    Xapian::PostingIterator p_clone(p);
    TEST_EQUAL(p, p_clone);

    vector<Xapian::docid> v(p, pend);

    p = db.postlist_begin("this");
    pend = db.postlist_end("this");
    vector<Xapian::docid>::const_iterator i;
    for (i = v.begin(); i != v.end(); i++) {
	TEST_NOT_EQUAL(p, pend);
	TEST_EQUAL(*i, *p);
	p++;
    }
    TEST_EQUAL(p, pend);
    return true;
}

static Xapian::PostingIterator
test_postlist3_helper()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    return db.postlist_begin("this");
}

// tests that a Xapian::PostingIterator still works when the DB is deleted
static bool test_postlist3()
{
    Xapian::PostingIterator u = test_postlist3_helper();
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator p = db.postlist_begin("this");
    Xapian::PostingIterator pend = db.postlist_end("this");

    while (p != pend) {
	TEST_EQUAL(*p, *u);
	p++;
	u++;
    }
    return true;
}

// tests skip_to
static bool test_postlist4()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator i = db.postlist_begin("this");
    i.skip_to(1);
    i.skip_to(999999999);
    TEST(i == db.postlist_end("this"));
    return true;
}

// tests long postlists
static bool test_postlist5()
{
    Xapian::Database db(get_database("apitest_manydocs"));
    // Allow for databases which don't support length
    if (db.get_avlength() != 1)
	TEST_EQUAL_DOUBLE(db.get_avlength(), 4);
    Xapian::PostingIterator i = db.postlist_begin("this");
    unsigned int j = 1;
    while (i != db.postlist_end("this")) {
	TEST_EQUAL(*i, j);
	i++;
	j++;
    }
    TEST_EQUAL(j, 513);
    return true;
}

// tests document length in postlists
static bool test_postlist6()
{
    Xapian::Database db(get_database("apitest_simpledata"));
    Xapian::PostingIterator i = db.postlist_begin("this");
    TEST(i != db.postlist_end("this"));
    while (i != db.postlist_end("this")) {
	TEST_EQUAL(i.get_doclength(), db.get_doclength(*i));
	i++;
    }
    return true;
}

// tests collection frequency
static bool test_collfreq1()
{
    Xapian::Database db(get_database("apitest_simpledata"));

    TEST_EQUAL(db.get_collection_freq("this"), 11);
    TEST_EQUAL(db.get_collection_freq("first"), 1);
    TEST_EQUAL(db.get_collection_freq("last"), 0);
    TEST_EQUAL(db.get_collection_freq("word"), 9);

    Xapian::Database db1(get_database("apitest_simpledata", "apitest_simpledata2"));
    Xapian::Database db2(get_database("apitest_simpledata"));
    db2.add_database(get_database("apitest_simpledata2"));

    TEST_EQUAL(db1.get_collection_freq("this"), 15);
    TEST_EQUAL(db1.get_collection_freq("first"), 1);
    TEST_EQUAL(db1.get_collection_freq("last"), 0);
    TEST_EQUAL(db1.get_collection_freq("word"), 11);
    TEST_EQUAL(db2.get_collection_freq("this"), 15);
    TEST_EQUAL(db2.get_collection_freq("first"), 1);
    TEST_EQUAL(db2.get_collection_freq("last"), 0);
    TEST_EQUAL(db2.get_collection_freq("word"), 11);

    return true;
}

// Regression test for split msets being incorrect when sorting
static bool test_sortvalue1()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_query(Xapian::Query("this"));

    for (int pass = 1; pass <= 2; ++pass) {
	for (Xapian::valueno value_no = 1; value_no < 7; ++value_no) {
	    tout << "Sorting on value " << value_no << endl;
	    enquire.set_sort_by_value(value_no);
	    Xapian::MSet allbset = enquire.get_mset(0, 100);
	    Xapian::MSet partbset1 = enquire.get_mset(0, 3);
	    Xapian::MSet partbset2 = enquire.get_mset(3, 97);
	    TEST_EQUAL(allbset.size(), partbset1.size() + partbset2.size());

	    bool ok = true;
	    int n = 0;
	    Xapian::MSetIterator i, j;
	    j = allbset.begin();
	    for (i = partbset1.begin(); i != partbset1.end(); ++i) {
		tout << "Entry " << n << ": " << *i << " | " << *j << endl;
		TEST(j != allbset.end());
		if (*i != *j) ok = false;
		++j;
		++n;
	    }
	    tout << "===\n";
	    for (i = partbset2.begin(); i != partbset2.end(); ++i) {
		tout << "Entry " << n << ": " << *i << " | " << *j << endl;
		TEST(j != allbset.end());
		if (*i != *j) ok = false;
		++j;
		++n;
	    }
	    TEST(j == allbset.end());
	    if (!ok)
		FAIL_TEST("Split msets aren't consistent with unsplit");
	}
	enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    }

    return true;
}

// consistency check match - vary mset size and check results agree
static bool test_consistency1()
{
    Xapian::Database db(get_database("etext"));
    Xapian::Enquire enquire(db);
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, Xapian::Query("the"), Xapian::Query("sky")));
    Xapian::doccount lots = 214;
    Xapian::MSet bigmset = enquire.get_mset(0, lots);
    TEST_EQUAL(bigmset.size(), lots);
    try {
	for (Xapian::doccount start = 0; start < lots; ++start) {
	    for (Xapian::doccount size = 0; size < lots - start; ++size) {
		Xapian::MSet mset = enquire.get_mset(start, size);
		if (mset.size()) {
		    TEST_EQUAL(start + mset.size(),
			       min(start + size, bigmset.size()));
		} else if (size) {
//		tout << start << mset.size() << bigmset.size() << endl;
		    TEST(start >= bigmset.size());
		}
		for (Xapian::doccount i = 0; i < mset.size(); ++i) {
		    TEST_EQUAL(*mset[i], *bigmset[start + i]);
		    TEST_EQUAL_DOUBLE(mset[i].get_weight(),
				      bigmset[start + i].get_weight());
		}
	    }
	}
    }
    catch (const Xapian::NetworkTimeoutError &) {
	// consistency1 is a long test - may timeout with the remote backend...
	SKIP_TEST("Test taking too long");
    }
    return true;
}

// tests that specifying a nonexistent input file throws an exception.
static bool test_quartzdatabaseopeningerror1()
{
    mkdir(".quartz", 0755);

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Quartz::open(".quartz/nosuchdirectory"));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Quartz::open(".quartz/nosuchdirectory", Xapian::DB_OPEN));

    mkdir(".quartz/emptydirectory", 0700);
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Quartz::open(".quartz/emptydirectory"));

    touch(".quartz/somefile");
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Quartz::open(".quartz/somefile"));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Quartz::open(".quartz/somefile", Xapian::DB_OPEN));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Quartz::open(".quartz/somefile", Xapian::DB_CREATE));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Quartz::open(".quartz/somefile", Xapian::DB_CREATE_OR_OPEN));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
	Xapian::Quartz::open(".quartz/somefile", Xapian::DB_CREATE_OR_OVERWRITE));

    return true;
}

/// Test opening of a quartz database
static bool test_quartzdatabaseopen1()
{
    const char * dbdir = ".quartz/test_quartzdatabaseopen1";
    mkdir(".quartz", 0755);

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Quartz::open(dbdir, Xapian::DB_OPEN));
	Xapian::Quartz::open(dbdir);
    }

    {
	rm_rf(dbdir);
	mkdir(dbdir, 0700);
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Quartz::open(dbdir, Xapian::DB_OPEN));
	Xapian::Quartz::open(dbdir);
    }

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE_OR_OPEN);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE));
	Xapian::Quartz::open(dbdir);
    }

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE_OR_OPEN));
	Xapian::Quartz::open(dbdir);
    }

    {
	TEST_EXCEPTION(Xapian::DatabaseCreateError,
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE));
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	Xapian::Quartz::open(dbdir);
    }

    {
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_CREATE_OR_OPEN);
	Xapian::Quartz::open(dbdir);
    }

    {
	Xapian::WritableDatabase wdb =
	    Xapian::Quartz::open(dbdir, Xapian::DB_OPEN);
	Xapian::Quartz::open(dbdir);
    }

    return true;
}

// tests that specifying a nonexistent input file throws an exception.
static bool test_flintdatabaseopeningerror1()
{
    mkdir(".flint", 0755);

    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/nosuchdirectory"));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/nosuchdirectory", Xapian::DB_OPEN));

    mkdir(".flint/emptydirectory", 0700);
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/emptydirectory"));

    touch(".flint/somefile");
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/somefile"));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/somefile", Xapian::DB_OPEN));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/somefile", Xapian::DB_CREATE));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/somefile", Xapian::DB_CREATE_OR_OPEN));
    TEST_EXCEPTION(Xapian::DatabaseOpeningError,
		   Xapian::Flint::open(".flint/somefile", Xapian::DB_CREATE_OR_OVERWRITE));

    return true;
}

// Helper function to make a flint version file with the given version number.
static void write_version_file(std::string filename, unsigned long version)
{
#define MAGIC_STRING "IAmFlint"
#define CONST_STRLEN(S) (sizeof(S"")-1)
#define MAGIC_LEN CONST_STRLEN(MAGIC_STRING)
#define VERSIONFILE_SIZE (MAGIC_LEN + 4)

    char buf[VERSIONFILE_SIZE] = MAGIC_STRING;

    unsigned char *v = reinterpret_cast<unsigned char *>(buf) + MAGIC_LEN;
    v[0] = static_cast<unsigned char>(version & 0xff);
    v[1] = static_cast<unsigned char>((version >> 8) & 0xff);
    v[2] = static_cast<unsigned char>((version >> 16) & 0xff);
    v[3] = static_cast<unsigned char>((version >> 24) & 0xff);

    int fd = ::open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);

    if (fd < 0) {
	string msg("Failed to create fake flint version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    try {
	size_t n = VERSIONFILE_SIZE;
	const char * p = buf;

	while (n) {
	    int c = write(fd, p, n);
	    if (c < 0) {
		if (errno == EINTR) continue;
		throw Xapian::DatabaseError("Error writing to fake flint version file", errno);
	    }
	    p += c;
	    n -= c;
	}
    } catch (...) {
	(void)close(fd);
	throw;
    }

    if (close(fd) != 0) {
	string msg("Failed to create fake flint version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
}

// tests that appropriate error is thrown for database format change
static bool test_flintdatabaseformaterror1()
{
    mkdir(".flint", 0755);
    std::string dbdir = ".flint/formatdb";
    std::string flintfilename = dbdir + "/iamflint";

    rm_rf(dbdir);
    // Create a database
    {
	Xapian::Flint::open(dbdir, Xapian::DB_CREATE);
    }

    // Fiddle with the version file, so that xapian thinks it's an old format
    // database.
    write_version_file(flintfilename, 200611200);

    // We should get a version error when we try and open the database for
    // reading now.
    TEST_EXCEPTION(Xapian::DatabaseVersionError,
		   Xapian::Flint::open(dbdir));
    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    return true;
}

// regression test for not releasing lock on error.
static bool test_flintdatabaseformaterror2()
{
    mkdir(".flint", 0755);
    std::string dbdir = ".flint/formatdb";
    std::string flintfilename = dbdir + "/iamflint";

    rm_rf(dbdir);
    // Create a database
    {
	Xapian::Flint::open(dbdir, Xapian::DB_CREATE);
    }

    // Fiddle with the version file, so that xapian thinks it's an old format
    // database.
    write_version_file(flintfilename, 200611200);

    TEST_EXCEPTION(Xapian::DatabaseVersionError,
		   Xapian::WritableDatabase(dbdir, Xapian::DB_CREATE_OR_OPEN));

    // This used to throw a DatabaseLockError: "Unable to acquire database
    // write lock on .flint/formatdb: already locked"
    Xapian::WritableDatabase(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);

    return true;
}


/// Test opening of a flint database
static bool test_flintdatabaseopen1()
{
    const string dbdir = ".flint/test_flintdatabaseopen1";
    mkdir(".flint", 0755);

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb =
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Flint::open(dbdir, Xapian::DB_OPEN));
	Xapian::Flint::open(dbdir);
    }

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb =
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OPEN);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE));
	Xapian::Flint::open(dbdir);
    }

    {
	rm_rf(dbdir);
	Xapian::WritableDatabase wdb =
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	TEST_EXCEPTION(Xapian::DatabaseLockError,
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OPEN));
	Xapian::Flint::open(dbdir);
    }

    {
	TEST_EXCEPTION(Xapian::DatabaseCreateError,
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE));
	Xapian::WritableDatabase wdb =
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	Xapian::Flint::open(dbdir);
    }

    {
	Xapian::WritableDatabase wdb =
	    Xapian::Flint::open(dbdir, Xapian::DB_CREATE_OR_OPEN);
	Xapian::Flint::open(dbdir);
    }

    {
	Xapian::WritableDatabase wdb =
	    Xapian::Flint::open(dbdir, Xapian::DB_OPEN);
	Xapian::Flint::open(dbdir);
    }

    return true;
}

// feature test for Enquire:
// set_sort_by_value
// set_sort_by_value_then_relevance
// set_sort_by_relevance_then_value
static bool test_sortrel1()
{
    Xapian::Enquire enquire(get_database("apitest_sortrel"));
    enquire.set_sort_by_value(1);
    enquire.set_query(Xapian::Query("woman"));

    const Xapian::docid order1[] = { 1,2,3,4,5,6,7,8,9 };
    const Xapian::docid order2[] = { 2,1,3,6,5,4,7,9,8 };
    const Xapian::docid order3[] = { 3,2,1,6,5,4,9,8,7 };
    const Xapian::docid order4[] = { 7,8,9,4,5,6,1,2,3 };
    const Xapian::docid order5[] = { 9,8,7,6,5,4,3,2,1 };
    const Xapian::docid order6[] = { 7,9,8,6,5,4,2,1,3 };
    const Xapian::docid order7[] = { 7,9,8,6,5,4,2,1,3 };
    const Xapian::docid order8[] = { 7,6,2,9,5,1,8,4,3 };
    const Xapian::docid order9[] = { 2,6,7,1,5,9,3,4,8 };

    Xapian::MSet mset;
    size_t i;

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order1) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order1) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order1[i]);
    }

    enquire.set_sort_by_value_then_relevance(1);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order2) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order2) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order2[i]);
    }

    enquire.set_sort_by_value(1);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order1) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order1) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order1[i]);
    }

    enquire.set_sort_by_value_then_relevance(1);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order2) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order2) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order2[i]);
    }

    enquire.set_sort_by_value(1);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);

    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order3) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order3) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order3[i]);
    }

    enquire.set_sort_by_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order4) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order4) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order4[i]);
    }

    enquire.set_sort_by_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order5) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order5) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order5[i]);
    }

    enquire.set_sort_by_value_then_relevance(1, false);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order6) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order6) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order6[i]);
    }

    enquire.set_sort_by_value_then_relevance(1, false);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order7) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order7) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order7[i]);
    }

    enquire.set_sort_by_relevance_then_value(1);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order8) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order8) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order8[i]);
    }

    enquire.set_sort_by_relevance_then_value(1);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order8) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order8) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order8[i]);
    }

    enquire.set_sort_by_relevance_then_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::ASCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order9) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order9) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order9[i]);
    }

    enquire.set_sort_by_relevance_then_value(1, false);
    enquire.set_docid_order(Xapian::Enquire::DESCENDING);
    mset = enquire.get_mset(0, 10);
    TEST_EQUAL(mset.size(), sizeof(order9) / sizeof(Xapian::docid));
    for (i = 0; i < sizeof(order9) / sizeof(Xapian::docid); ++i) {
	TEST_EQUAL(*mset[i], order9[i]);
    }

    return true;
}

// Test network stats and local stats give the same results.
static bool test_netstats1()
{
    BackendManager local_manager;
#if defined XAPIAN_HAS_FLINT_BACKEND
    local_manager.set_dbtype("flint");
#elif defined XAPIAN_HAS_QUARTZ_BACKEND
    local_manager.set_dbtype("quartz");
#else
    SKIP_TEST("No suitable local database backend enabled");
#endif
    local_manager.set_datadir(test_driver::get_srcdir() + "/testdata/");

    const char * words[] = { "paragraph", "word" };
    Xapian::Query query(Xapian::Query::OP_OR, words, words + 2);
    const size_t MSET_SIZE = 10;

    Xapian::RSet rset;
    rset.add_document(4);
    rset.add_document(9);

    Xapian::MSet mset_alllocal;
    {
	Xapian::Database db;
	db.add_database(local_manager.get_database("apitest_simpledata"));
	db.add_database(local_manager.get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	mset_alllocal = enq.get_mset(0, MSET_SIZE, &rset);
    }

    {
	Xapian::Database db;
	db.add_database(local_manager.get_database("apitest_simpledata"));
	db.add_database(get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, MSET_SIZE, &rset);
	TEST_EQUAL(mset, mset_alllocal);
    }

    {
	Xapian::Database db;
	db.add_database(get_database("apitest_simpledata"));
	db.add_database(local_manager.get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, MSET_SIZE, &rset);
	TEST_EQUAL(mset, mset_alllocal);
    }

    {
	Xapian::Database db;
	db.add_database(get_database("apitest_simpledata"));
	db.add_database(get_database("apitest_simpledata2"));

	Xapian::Enquire enq(db);
	enq.set_query(query);
	Xapian::MSet mset = enq.get_mset(0, MSET_SIZE, &rset);
	TEST_EQUAL(mset, mset_alllocal);
    }

    return true;
}

// Coordinate matching - scores 1 for each matching term
class MyWeight : public Xapian::Weight {
    public:
	MyWeight * clone() const {
	    return new MyWeight;
	}
	MyWeight() { }
	~MyWeight() { }
	std::string name() const { return "Coord"; }
	std::string serialise() const { return ""; }
	MyWeight * unserialise(const std::string & /*s*/) const {
	    return new MyWeight;
	}
	Xapian::weight get_sumpart(Xapian::termcount /*wdf*/, Xapian::doclength /*len*/) const { return 1; }
	Xapian::weight get_maxpart() const { return 1; }

	Xapian::weight get_sumextra(Xapian::doclength /*len*/) const { return 0; }
	Xapian::weight get_maxextra() const { return 0; }

	bool get_sumpart_needs_doclength() const { return false; }
};

// tests user weighting scheme
static bool test_userweight1()
{
    Xapian::Enquire enquire(get_database("apitest_simpledata"));
    enquire.set_weighting_scheme(MyWeight());
    const char * query[] = { "this", "line", "paragraph", "rubbish" };
    enquire.set_query(Xapian::Query(Xapian::Query::OP_OR, query,
				    query + sizeof(query) / sizeof(query[0])));
    Xapian::MSet mymset1 = enquire.get_mset(0, 100);
    // MyWeight scores 1 for each matching term, so the weight should equal
    // the number of matching terms.
    for (Xapian::MSetIterator i = mymset1.begin(); i != mymset1.end(); ++i) {
	Xapian::termcount matching_terms = 0;
	Xapian::TermIterator t = enquire.get_matching_terms_begin(i);
	while (t != enquire.get_matching_terms_end(i)) {
	    ++matching_terms;
	    ++t;
	}
	TEST_EQUAL(i.get_weight(), matching_terms);
    }

    return true;
}

// #######################################################################
// # End of test cases: now we list the tests to run.

/// The tests which require a database which supports values > 0 sensibly
test_desc multivalue_tests[] = {
    {"collapsekey1",	   test_collapsekey1},
    // There no longer is a collapsekey2 test!
    {"collapsekey3",	   test_collapsekey3},
    {"collapsekey4",	   test_collapsekey4},
    {0, 0}
};

/// The tests which need a backend which supports iterating over all terms
test_desc allterms_tests[] = {
    {"allterms1",	   test_allterms1},
    {"allterms2",	   test_allterms2},
    {"allterms3",	   test_allterms3},
    {"allterms4",	   test_allterms4},
    {"allterms5",	   test_allterms5},
    {"allterms6",	   test_allterms6},
    {"specialterms2",	   test_specialterms2},
    {0, 0}
};

/// The tests which need a backend which supports terms with newlines / zeros
test_desc specchar_tests[] = {
    {"specialterms1",	   test_specialterms1},
    {0, 0}
};

/// The tests which need a backend which supports document length information
test_desc doclendb_tests[] = {
// Mset comes out in wrong order - no document length?
    {"rsetmultidb2",       test_rsetmultidb2},
    {0, 0}
};

/// Tests which need getting collection frequencies to be supported.
test_desc collfreq_tests[] = {
    {"collfreq1",	   test_collfreq1},
    {0, 0}
};

test_desc localdb_tests[] = {
    {"matchfunctor1",	   test_matchfunctor1},
    {"msetiterator1",	   test_msetiterator1},
    {"msetiterator2",	   test_msetiterator2},
    {"msetiterator3",	   test_msetiterator3},
    {"esetiterator1",	   test_esetiterator1},
    {"esetiterator2",	   test_esetiterator2},
    {"multiexpand1",       test_multiexpand1},
    {"postlist1",	   test_postlist1},
    {"postlist2",	   test_postlist2},
    {"postlist3",	   test_postlist3},
    {"postlist4",	   test_postlist4},
    {"postlist5",	   test_postlist5},
    {"postlist6",	   test_postlist6},
    {"termstats",	   test_termstats},
    {"sortvalue1",	   test_sortvalue1},
    // consistency1 will run on the remote backend, but it's particularly slow
    // with that, and testing it there doesn't actually improve the test
    // coverage really.
    {"consistency1",	   test_consistency1},
    // Would work with remote if we registered the weighting scheme.
    // FIXME: do this so we also test that functionality...
    {"userweight1",	   test_userweight1},
    {0, 0}
};

test_desc remotedb_tests[] = {
// FIXME:    {"multierrhandler1",   test_multierrhandler1},
    {"msetiterator1",	   test_msetiterator1},
    {"msetiterator2",	   test_msetiterator2},
    {"msetiterator3",	   test_msetiterator3},
    {"esetiterator1",	   test_esetiterator1},
    {"esetiterator2",	   test_esetiterator2},
    {"multiexpand1",       test_multiexpand1},
    {"postlist1",	   test_postlist1},
    {"postlist2",	   test_postlist2},
    {"postlist3",	   test_postlist3},
    {"postlist4",	   test_postlist4},
    {"postlist5",	   test_postlist5},
    {"postlist6",	   test_postlist6},
    {"stubdb1",		   test_stubdb1},
    {"keepalive1",	   test_keepalive1},
    {"termstats",	   test_termstats},
    {"sortvalue1",	   test_sortvalue1},
    {"sortrel1",	   test_sortrel1},
    {"netstats1",	   test_netstats1},
    {0, 0}
};

test_desc flint_tests[] = {
    {"flintdatabaseopeningerror1",	test_flintdatabaseopeningerror1},
    {"flintdatabaseformaterror1",	test_flintdatabaseformaterror1},
    {"flintdatabaseformaterror2",	test_flintdatabaseformaterror2},
    {"flintdatabaseopen1",		test_flintdatabaseopen1},
    {"sortrel1",	   test_sortrel1},
    {0, 0}
};

test_desc quartz_tests[] = {
    {"quartzdatabaseopeningerror1",	test_quartzdatabaseopeningerror1},
    {"quartzdatabaseopen1",		test_quartzdatabaseopen1},
    {"sortrel1",	   test_sortrel1},
    {0, 0}
};
