/* api_posdb.cc: tests which need a backend with positional information
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::vector;
using std::string;

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"

#include "apitest.h"

/// Simple test of NEAR
static bool test_near1()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    enquire.set_weighting_scheme(BoolWeight());

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2, 3);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(4);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(7);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(8);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    // test really large window size
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(999999999);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    return true;
}

/// Test NEAR over operators
static bool test_near2()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    enquire.set_weighting_scheme(BoolWeight());

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("and")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    q = OmQuery(OmQuery::OP_NEAR, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    return true;
}

/// Simple test of PHRASE
static bool test_phrase1()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    enquire.set_weighting_scheme(BoolWeight());

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("phrase")));
    subqs.push_back(OmQuery(stemmer.stem_word("near")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 1, 2);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(3);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(4);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(5);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(6);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(7);
    enquire.set_query(q);

    // retrieve the top twenty results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(8);
    enquire.set_query(q);

    // retrieve the top 20 results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    // test really large window size
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("leave")));
    subqs.push_back(OmQuery(stemmer.stem_word("fridge")));
    subqs.push_back(OmQuery(stemmer.stem_word("on")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(999999999);
    enquire.set_query(q);

    // retrieve the top 20 results
    mymset = enquire.get_mset(0, 20);
    mset_expect_order(mymset, 4);

    // regression test (was matching doc 15, should fail)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("first")));
    subqs.push_back(OmQuery(stemmer.stem_word("second")));
    subqs.push_back(OmQuery(stemmer.stem_word("third")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(9);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    // regression test (should match doc 15, make sure still does with fix)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("first")));
    subqs.push_back(OmQuery(stemmer.stem_word("second")));
    subqs.push_back(OmQuery(stemmer.stem_word("third")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(10);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 15);

    // regression test (phrase matching was getting order wrong when
    // build_and_tree reordered vector of PostLists)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("milk")));
    subqs.push_back(OmQuery(stemmer.stem_word("rare")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 16);

    // regression test (phrase matching was getting order wrong when
    // build_and_tree reordered vector of PostLists)
    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("rare")));
    subqs.push_back(OmQuery(stemmer.stem_word("milk")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 17);

    return true;
}

/// Test PHRASE over operators
static bool test_phrase2()
{
    OmDatabase mydb(get_database("apitest_phrase"));
    OmEnquire enquire(make_dbgrp(&mydb));
    OmStem stemmer("english");
    enquire.set_weighting_scheme(BoolWeight());

    // make a query
    vector<OmQuery> subqs;
    OmQuery q;
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("and")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    OmMSet mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    subqs.clear();
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset, 2);

    subqs.clear();
    subqs.push_back(OmQuery(stemmer.stem_word("operator")));
    subqs.push_back(OmQuery(OmQuery::OP_AND,
			    OmQuery(stemmer.stem_word("phrase")),
			    OmQuery(stemmer.stem_word("near"))));
    q = OmQuery(OmQuery::OP_PHRASE, subqs.begin(), subqs.end());
    q.set_window(2);
    enquire.set_query(q);

    // retrieve the top ten results
    mymset = enquire.get_mset(0, 10);
    mset_expect_order(mymset);

    return true;
}

/// Test getting position lists from databases
static bool test_poslist1()
{
    OmDatabase mydb(get_database("apitest_poslist"));

    OmStem stemmer("english");
    string term = stemmer.stem_word("sponge");
    
    OmPositionListIterator pli = mydb.positionlist_begin(2, term);

    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 1);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 2);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 3);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 5);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 8);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 13);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 21);
    pli++;
    TEST(pli != mydb.positionlist_end(2, term));
    TEST(*pli == 34);
    pli++;
    TEST(pli == mydb.positionlist_end(2, term));

    TEST_EXCEPTION(Xapian::DocNotFoundError, mydb.positionlist_begin(55, term));

    /* FIXME: what exception should be thrown here?  Quartz throws
     * Xapian::DocNotFoundError, and InMemory throws Xapian::RangeError.
     */
    TEST_EXCEPTION(Xapian::RuntimeError, mydb.positionlist_begin(2, "adskfjadsfa"));
    TEST_EXCEPTION(Xapian::DocNotFoundError, mydb.positionlist_begin(55, "adskfjadsfa"));

    return true;
}

static bool test_poslist2()
{
    OmWritableDatabase db = get_writable_database("");

    OmDocument doc;
    doc.add_term_nopos("nopos");
    om_docid did = db.add_document(doc);

    TEST_EXCEPTION(Xapian::RangeError,
	// Check what happens when term doesn't exist
	OmPositionListIterator i = db.positionlist_begin(did, "nosuchterm");
	// FIXME: quartz doesn't throw!
    );

    TEST_EXCEPTION(Xapian::DocNotFoundError,
        // Check what happens when the document doesn't even exist
        OmPositionListIterator i = db.positionlist_begin(123, "nosuchterm");
	// FIXME: quartz doesn't throw!
    );            
    
    {
	OmPositionListIterator i = db.positionlist_begin(did, "nopos");
	TEST_EQUAL(i, db.positionlist_end(did, "nopos"));
    }
    
    OmDocument doc2 = db.get_document(did);
   
    OmTermIterator term = doc2.termlist_begin();

    {
	OmPositionListIterator i = term.positionlist_begin(); 
	TEST_EQUAL(i, term.positionlist_end());
    }

    OmDocument doc3;
    doc3.add_posting("hadpos", 1);
    om_docid did2 = db.add_document(doc3);

    OmDocument doc4 = db.get_document(did2);
    doc4.remove_posting("hadpos", 1);
    db.replace_document(did2, doc4);
   
    {
	OmPositionListIterator i = db.positionlist_begin(did2, "hadpos");
	TEST_EQUAL(i, db.positionlist_end(did2, "hadpos"));
    }

    db.delete_document(did);
    TEST_EXCEPTION(Xapian::DocNotFoundError,
        // Check what happens when the document doesn't even exist
	// (but once did)
	OmPositionListIterator i = db.positionlist_begin(did, "nosuchterm");
	// FIXME: quartz doesn't throw!
    );

    return true;
}
// # End of test cases: now we list the tests to run.

/// The tests which need a backend which supports positional information
test_desc positionaldb_tests[] = {
    {"near1",		   test_near1},
    {"near2",		   test_near2},
    {"phrase1",		   test_phrase1},
    {"phrase2",		   test_phrase2},
    {0, 0}
};

/** The tests which need a backend which supports positional information
 *  and opening position lists from the database
 *  FIXME: implement for network?
 */
test_desc localpositionaldb_tests[] = {
    {"poslist1",	   test_poslist1},
// FIXME: fix quartz to pass this test
    {"poslist2",	   test_poslist2},
    {0, 0}
};
