/* testsuite.h: a generic test suite engine
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_TESTSUITE_H
#define OM_HGUARD_TESTSUITE_H

#include "omstringstream.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** Class which is thrown when a test case fails.
 */
class TestFail { };

/** Class which is thrown when a test case is to be skipped.
 *
 *  This happens when something can't be tested for some reason, but
 *  that reason isn't grounds for causing the test to fail.
 */
class TestSkip { };

/** Macro used to build a TestFail object and throw it.
 */
// Don't bracket a, because it may have <<'s in it
#define FAIL_TEST(a) do { TestFail testfail; \
                          if (verbose) { tout << a << '\n'; } \
		          throw testfail; } while (0)

/** Macro used to build a TestSkip object and throw it.
 */
// Don't bracket a, because it may have <<'s in it
#define SKIP_TEST(a) do { TestSkip testskip; \
                          if (verbose) { tout << a << '\n'; } \
		          throw testskip; } while (0)

/// Type for a test function.
typedef bool (*test_func)();

/// Structure holding a description of a test.
struct test_desc {
    /// The name of the test.
    const char *name;

    /// The function to run to perform the test.
    test_func run;
};

/// The global verbose flag.
//
//  If verbose is set, then the test harness will display diagnostic output
//  for tests which fail or skip.  Individual tests may use this flag to avoid
//  needless generation of diagnostic output in cases when it's expensive.
extern bool verbose;

/// The exception type we were expecting in TEST_EXCEPTION.
//  Used to detect if such an exception was mishandled by a the
//  compiler/runtime.
extern const char * expected_exception;

/** The output stream.  Data written to this stream will only appear
 *  when a test fails.
 */
extern om_ostringstream tout;

/// The test driver.  This class takes care of running the tests.
class test_driver {
    friend void report_totals();

    public:
	/** A structure used to report the summary of tests passed
	 *  and failed.
	 */
	struct result {
	    /// The number of tests which succeeded.
	    unsigned int succeeded;

	    /// The number of tests which failed.
	    unsigned int failed;

	    /// The number of tests which were skipped
	    unsigned int skipped;
	};

	/** Add a test-specific command line option.
	 *
	 *  The recognised option will be described as:
	 *
	 *   -<s>=<l>
	 *
	 *  And any value set will be put into arg.
	 */
	static void add_command_line_option(const std::string &l, char s,
					    std::string * arg);

	/** Parse the command line arguments.
	 *
	 *  @param  argc	The argument count passed into ::main()
	 *  @param  argv	The argument list passed into ::main()
	 */
	static void parse_command_line(int argc, char **argv);

	static void usage();

	static int run(const test_desc *tests);

	/** The constructor, which sets up the test driver.
	 *
	 *  @param tests The zero-terminated array of tests to run.
	 */
	test_driver(const test_desc *tests_);

	/** Run all the tests supplied and return the results
	 */
	result run_tests();

	/** Run the tests in the list and return the results
	 */
	result run_tests(std::vector<std::string>::const_iterator b,
			 std::vector<std::string>::const_iterator e);

	/** Read srcdir from environment and if not present, make a valiant
	 *  attempt to guess a value
	 */
	static std::string get_srcdir();

	// running total for a test run
	static result total;

    private:
	/** Prevent copying */
	test_driver(const test_driver &);
	test_driver & operator = (const test_driver &);

	typedef enum { PASS = 1, FAIL = 0, SKIP = -1 } test_result;

	static std::map<int, std::string *> short_opts;

	static std::string opt_help;

	static std::vector<std::string> test_names;

	/** Runs the test function and returns its result.  It will
	 *  also trap exceptions and some memory leaks and force a
	 *  failure in those cases.
	 *
	 *  @param test A description of the test to run.
	 */
	test_result runtest(const test_desc *test);

	/** The implementation used by run_tests.
	 *  it runs test(s) (with runtest()), prints out messages for
	 *  the user, and tracks the successes and failures.
	 *
	 *  @param b, e  If b != e, a vector of the test(s) to run.
	 *               If b == e, all tests will be run.
	 */
	result do_run_tests(std::vector<std::string>::const_iterator b,
			    std::vector<std::string>::const_iterator e);

	/// print summary of tests passed, failed, and skipped
	static void report(const test_driver::result &r, const std::string &desc);

	// abort tests at the first failure
	static bool abort_on_error;

	// the default stream to output to
	std::ostream out;

	// the list of tests to run.
	const test_desc *tests;

	// how many test runs we've done - no summary if just one run
	static int runs;

	// program name
	static std::string argv0;

	// strings to use for colouring - empty if output isn't a tty
	static std::string col_red, col_green, col_yellow, col_reset;
};

#ifndef STRINGIZE
/** STRINGIZE converts a piece of code to a string, so it can be displayed.
 *
 *  The 2nd level of the stringize definition here is not needed for the use we
 *  put this to in this file (since we always use it within a macro here) but
 *  is required in general  (#N doesn't work outside a macro definition)
 */
#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N
#endif

/// Display the location at which a testcase occured, with an explanation
#define TESTCASE_LOCN(a) __FILE__":"STRINGIZE(__LINE__)": "STRINGIZE(a)

/** Test a condition, and display the test with an extra explanation if
 *  the condition fails.
 *  NB: uses an else clause to avoid dangling else damage
 */
#define TEST_AND_EXPLAIN(a, b) \
    if (a) { } else \
	FAIL_TEST(TESTCASE_LOCN(a) << std::endl << b << std::endl)

/// Test a condition, without an additional explanation for failure.
#define TEST(a) TEST_AND_EXPLAIN(a, "")

/// Test for equality of two things.
#define TEST_EQUAL(a, b) TEST_AND_EXPLAIN(((a) == (b)), \
	"Expected `"STRINGIZE(a)"' and `"STRINGIZE(b)"' to be equal:" \
	" were " << (a) << " and " << (b))

#include <float.h> // for DBL_EPSILON
#include <math.h> // for fabs
#define TEST_EQUAL_DOUBLE(a, b) TEST_AND_EXPLAIN((fabs((a) - (b)) < DBL_EPSILON), \
	"Expected `"STRINGIZE(a)"' and `"STRINGIZE(b)"' to be (nearly) equal:" \
	" were " << (a) << " and " << (b))

/// Test for non-equality of two things.
#define TEST_NOT_EQUAL(a, b) TEST_AND_EXPLAIN(((a) != (b)), \
	"Expected `"STRINGIZE(a)"' and `"STRINGIZE(b)"' not to be equal:" \
	" were " << (a) << " and " << (b))

#endif // OM_HGUARD_TESTSUITE_H
