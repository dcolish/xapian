/* internaltest.cc: test of the Omsee internals
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#include "config.h"
#include <iostream>
#include <string>
#include <dlfcn.h>
using std::cout;
using std::endl;

#include "om/om.h"
#include "testsuite.h"
#include "refcnt.h"
#include "omstringstream.h"

#ifdef MUS_BUILD_BACKEND_SLEEPYCAT
#include "../backends/sleepycat/sleepycat_list.h"
#endif

// always succeeds
bool test_trivial();
// always fails (for testing the framework)
bool test_alwaysfail();
// test the test framework
bool test_testsuite1();
bool test_testsuite2();
bool test_testsuite3();
bool test_testsuite4();

bool test_trivial()
{
    return true;
}

bool test_alwaysfail()
{
    return false;
}

bool test_skip()
{
    SKIP_TEST("skip test");
}

bool test_except1()
{
    try {
	throw 1;
    } catch (int) {
    }
    return true;
}

char *duff_allocation = 0;
char *duff_allocation_2 = 0;

bool test_duffnew()
{
    // make an unfreed allocation
    duff_allocation_2 = duff_allocation;
    duff_allocation = new char[7];
    return true;
}

char *duff_malloc_allocation = 0;
char *duff_malloc_allocation_2 = 0;

bool test_duffmalloc()
{
    // make an unfreed allocation
    duff_malloc_allocation_2 = duff_malloc_allocation;
    duff_malloc_allocation = (char *)malloc(7);
    return true;
}

bool test_testsuite1()
{
    test_desc mytests[] = {
	{"test0", test_skip},
	{"test1", test_alwaysfail},
	{"test2", test_trivial},
	{0, 0}
    };

    test_driver driver(mytests);
    driver.set_abort_on_error(false);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 1 && res.failed == 1 && res.skipped == 1,
		     res.succeeded << " succeeded, "
		     << res.failed << " failed,"
		     << res.skipped << " skipped.");

    return true;
}

bool test_testsuite2()
{
    test_desc mytests[] = {
	{"test1", test_alwaysfail},
	{"test2", test_trivial},
	{0, 0}
    };

    test_driver driver(mytests);
    driver.set_abort_on_error(true);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 0 && res.failed == 1,
		     res.succeeded << " succeeded, "
		     << res.failed << " failed.");

    return true;
}

/* In some systems <dlfcn.h> doesn't define RTLD_DEFAULT */
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT (void *)0
#endif

// test the memory leak tests
bool test_testsuite3()
{
    test_desc mytests[] = {
	{"duff_new", test_duffnew},
	{0, 0}
    };

    test_driver driver(mytests);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 0 && res.failed == 1,
		     "Memory leak checking with new/delete doesn't work");

    // clean up after test_duffnew()
    delete duff_allocation;
    duff_allocation = 0;
    delete duff_allocation_2;
    duff_allocation_2 = 0;

    return true;
}

// test the malloc() memory leak tests
bool test_testsuite4()
{
    test_desc mytests[] = {
	{"duff_malloc", test_duffmalloc},
	{0, 0}
    };

    if (!dlsym(RTLD_DEFAULT, "malloc_allocdata")) {
	SKIP_TEST("malloc tracking library not installed");
    }

    test_driver driver(mytests);
    if (!verbose) {
	driver.set_quiet(true);
    }

    test_driver::result res = driver.run_tests();
    TEST_AND_EXPLAIN(res.succeeded == 0 && res.failed == 1,
		     "Memory leak checking with malloc()/free() doesn't work");

    // clean up after test_duffnew()
    if (duff_malloc_allocation) {
	free(duff_malloc_allocation);
	duff_malloc_allocation = 0;
    }
    if (duff_malloc_allocation_2) {
	free(duff_malloc_allocation_2);
	duff_malloc_allocation_2 = 0;
    }

    return true;
}

class Test_Exception {
    public:
	int value;
	Test_Exception(int value_) : value(value_) {}
};

bool test_exception1()
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

#ifdef HAVE_NO_ACCESS_CONTROL
// ###########################################
// # Tests of the reference counted pointers #
// ###########################################

class test_refcnt : public RefCntBase {
    private:
	bool &deleted;
    public:
	test_refcnt(bool &deleted_) : deleted(deleted_) {
	    if (verbose) {
	        cout << " constructor ";
	    }
	}
	RefCntPtr<const test_refcnt> test() {
	    return RefCntPtr<const test_refcnt>(RefCntPtrToThis(), this);
	}
	~test_refcnt() {
	    deleted = true;
	    if (verbose) {
		cout << " destructor ";
	    }
	}
};

bool test_refcnt1()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    TEST_EQUAL(p->ref_count, 0);

    {
	RefCntPtr<test_refcnt> rcp(p);

	TEST_EQUAL(rcp->ref_count, 1);
	
	{
	    RefCntPtr<test_refcnt> rcp2;
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
// to if it was the reference count was 1 and you assigned it to itself.
bool test_refcnt2()
{
    bool deleted = false;

    test_refcnt *p = new test_refcnt(deleted);

    RefCntPtr<test_refcnt> rcp(p);
    
    rcp = rcp;
    
    TEST_AND_EXPLAIN(!deleted, "Object deleted by self-assignment");

    return true;
}

#endif /* HAVE_NO_ACCESS_CONTROL */

// test string comparisions
bool test_stringcomp1()
{
    bool success = true;

    std::string s1;
    std::string s2;

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

bool test_omstringstream1()
{
    om_ostringstream oss;
    oss << "foo" << 4 << "bar";
    TEST_EQUAL(oss.str(), "foo4bar");

    return true;
}

#ifdef MUS_BUILD_BACKEND_SLEEPYCAT
// test whether a SleepycatList packs and unpacks correctly
bool test_sleepycatpack1()
{
    bool success = true;

    SleepycatListItem::id_type id = 7;
    om_doccount termfreq = 92;
    om_termcount wdf = 81;
    om_doclength doclen = 75;
    std::vector<om_termpos> positions;
    positions.push_back(6u);
    positions.push_back(16u);

    SleepycatListItem item1(id, wdf, positions, termfreq, doclen);
    std::string packed1 = item1.pack(true);
    SleepycatListItem item2(packed1, true);
    std::string packed2 = item2.pack(true);

    if(packed1 != packed2) {
	success = false;
	if(verbose) {
	    cout << "Packed items were not equal ('" << packed1 <<
		    "' and '" << packed2 << "'" << endl;
	}
    }
    if(item1.id != item2.id) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (ids '" << item1.id <<
		    "' and '" << item2.id << "')" << endl;
	}
    }
    if(item1.termfreq != item2.termfreq) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (termfreqs '" <<
		    item1.termfreq << "' and '" << item2.termfreq << "')" <<
		    endl;
	}
    }
    if(item1.wdf != item2.wdf) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (wdfs '" << item1.wdf <<
		    "' and '" << item2.wdf << "')" << endl;
	}
    }
    if(item1.positions != item2.positions) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal" << endl;
	}
    }
    if(item1.doclength != item2.doclength) {
	success = false;
	if(verbose) {
	    cout << "Unpacked items were not equal (doclengths '" <<
		    item1.doclength << "' and '" << item2.doclength <<
		    "')" << endl;
	}
    }

    return success;
}
#endif

// ####################################
// # test the behaviour of OmSettings #
// ####################################
bool
test_omsettings1()
{
    OmSettings settings;

    settings.set("K1", "V1");
    settings.set("K2", "V2");
    settings.set("K1", "V3");

    TEST_EQUAL(settings.get("K1"), "V3");
    TEST_EQUAL(settings.get("K2"), "V2");
    return true;
}

bool
test_omsettings2()
{
    bool success = false;

    OmSettings settings;
    try {
	settings.get("nonexistant");

	if (verbose) {
	    cout << "get() didn't throw with invalid key" << endl;
	}
    } catch (OmRangeError &e) {
	success = true;
    }

    return success;
}

bool
test_omsettings3()
{
    bool success = true;

    // test copy-on-write behaviour.
    OmSettings settings1;

    settings1.set("FOO", "BAR");
    settings1.set("MOO", "COW");

    OmSettings settings2(settings1);

    if (settings2.get("FOO") != "BAR" ||
	settings2.get("MOO") != "COW") {
	success = false;
	if (verbose) {
	    cout << "settings weren't copied properly." << endl;
	}
    }

    if (settings1.get("FOO") != "BAR" ||
	settings1.get("MOO") != "COW") {
	success = false;
	if (verbose) {
	    cout << "settings destroyed when copied." << endl;
	}
    }

    settings2.set("BOO", "AAH");

    try {
	settings1.get("BOO");
	// should throw

	success = false;
	if (verbose) {
	    cout << "Changes leaked to original" << endl;
	}
    } catch (OmRangeError &) {
    }

    settings1.set("FOO", "RAB");

    if (settings2.get("FOO") != "BAR") {
	success = false;

	if (verbose) {
	    cout << "Changes leaked to copy" << endl;
	}
    }

    return success;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"except1",			test_except1},
    {"testsuite1",		test_testsuite1},
    {"testsuite2",		test_testsuite2},
    {"testsuite3",		test_testsuite3},
    {"testsuite4",		test_testsuite4},
    {"exception1",              test_exception1},
#ifdef HAVE_NO_ACCESS_CONTROL
    {"refcnt1",			test_refcnt1},
    {"refcnt2",			test_refcnt2},
#endif // HAVE_NO_ACCESS_CONTROL
    {"stringcomp1",		test_stringcomp1},
#ifdef MUS_BUILD_BACKEND_SLEEPYCAT
    {"sleepycatpack1",		test_sleepycatpack1},
#endif
    {"omstringstream1",		test_omstringstream1},
    {"omsettings1",		test_omsettings1},
    {"omsettings2",		test_omsettings2},
    {"omsettings3",		test_omsettings3},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
