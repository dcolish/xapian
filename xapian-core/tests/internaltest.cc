/* internaltest.cc: test of the Xapian internals
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
#include <iostream>
#include <string>

using namespace std;

#include "om/om.h"
#include "testsuite.h"
#include "omstringstream.h"

static bool test_except1()
{
    try {
	throw 1;
    } catch (int) {
    }
    return true;
}

class Test_Exception {
    public:
	int value;
	Test_Exception(int value_) : value(value_) {}
};

static bool test_exception1()
{
    try {
	try {
	    throw Test_Exception(1);
	} catch (...) {
	    try {
		throw Test_Exception(2);
	    } catch (...) {
	    }
	}
    } catch (Test_Exception & e) {
	TEST_EQUAL(e.value, 1);
    }
    return true;
}

// ###########################################
// # Tests of the reference counted pointers #
// ###########################################

class test_refcnt : public Xapian::Internal::RefCntBase {
    private:
	bool &deleted;
    public:
	test_refcnt(bool &deleted_) : deleted(deleted_) {
	    tout << "constructor\n";
	}
	Xapian::Internal::RefCntPtr<const test_refcnt> test() {
	    return Xapian::Internal::RefCntPtr<const test_refcnt>(this);
	}
	~test_refcnt() {
	    deleted = true;
	    tout << "destructor\n";
	}
};

static bool test_refcnt1()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    TEST_EQUAL(p->ref_count, 0);

    {
	Xapian::Internal::RefCntPtr<test_refcnt> rcp(p);

	TEST_EQUAL(rcp->ref_count, 1);
	
	{
	    Xapian::Internal::RefCntPtr<test_refcnt> rcp2;
	    rcp2 = rcp;
	    TEST_EQUAL(rcp->ref_count, 2);
	    // rcp2 goes out of scope here
	}
	
	TEST_AND_EXPLAIN(!deleted, "Object prematurely deleted!");
	TEST_EQUAL(rcp->ref_count, 1);
	// rcp goes out of scope here
    }
    
    TEST_AND_EXPLAIN(deleted, "Object not properly deleted");

    return true;
}

// This is a regression test - a RefCntPtr used to delete the object pointed
// to if you assignment it to itself and the reference count was 1.
static bool test_refcnt2()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    Xapian::Internal::RefCntPtr<test_refcnt> rcp(p);
    
    rcp = rcp;
    
    TEST_AND_EXPLAIN(!deleted, "Object deleted by self-assignment");

    return true;
}

// test string comparisions
static bool test_stringcomp1()
{
    bool success = true;

    string s1;
    string s2;

    s1 = "foo";
    s2 = "foo";

    if ((s1 != s2) || (s1 > s2)) {
	success = false;
	if (verbose) {
	    cout << "String comparisons BADLY wrong" << endl;
	}
    }

    s1 += '\0';

    if ((s1 == s2) || (s1 < s2)) {
	success = false;
	if (verbose) {
	    cout << "String comparisions don't cope with extra nulls" << endl;
	}
    }

    s2 += '\0';

    s1 += 'a';
    s2 += 'z';

    if ((s1.length() != 5) || (s2.length() != 5)) {
	success = false;
	if (verbose) {
	    cout << "Lengths with added nulls wrong" << endl;
	}
    }

    if ((s1 == s2) || !(s1 < s2)) {
	success = false;
	if (verbose) {
	    cout << "Characters after a null ignored in comparisons" << endl;
	}
    }

    return success;
}

static bool test_omstringstream1()
{
    om_ostringstream oss;
    oss << "foo" << 4 << "bar";
    TEST_EQUAL(oss.str(), "foo4bar");

    return true;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"except1",			test_except1},
    {"exception1",              test_exception1},
    {"refcnt1",			test_refcnt1},
    {"refcnt2",			test_refcnt2},
    {"stringcomp1",		test_stringcomp1},
    {"omstringstream1",		test_omstringstream1},
    {0, 0}
};

int main(int argc, char **argv)
{
    test_driver::parse_command_line(argc, argv);
    return test_driver::run(tests);
}
