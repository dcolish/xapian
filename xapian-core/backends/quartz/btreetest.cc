/* btreetest.cc: test of the btree manager
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
#include "btreecheck.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"

#include <algorithm>
#include <fstream>
#include <string>

using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_SSTREAM
# define BTREE_CHECK(DIR, OPTS) BtreeCheck::check(DIR, OPTS, tout)
#else
// FIXME: If we don't have <sstream> and hence ostringstream, we roll our own
// which can't be passed as an ostream so we just turn off the options and send
// the output (only a one line report of any error) to cout
# define BTREE_CHECK(DIR, OPTS) BtreeCheck::check(DIR, 0, cout)
#endif

static string tmpdir;
static string datadir;

static void delete_dir(string filename)
{
    system("rm -fr " + filename);
}

static void make_dir(string filename)
{
    mkdir(filename, 0700);
}

static int process_lines(Btree & btree, ifstream &f)
{
    int count = 0;
    while (true) {
	string s;
        if (!getline(f, s)) return count;
	if (s.empty()) continue;
	if (s[0] == '+') {
	    string::size_type sp = s.find(' ');
	    btree.add(s.substr(1, min(sp - 1, Btree::max_key_len)),
		      s.substr(sp + 1));
	    ++count;
	} else if (s[0] == '-') {
	    btree.del(s.substr(1, Btree::max_key_len));
	    --count;
	} else {
	    throw "No '+' or '-' on line `" + s + "'";
	}
    }
}

static int do_update(const string & btree_dir,
		     const string & datafile,
		     bool full_compact = false)
{
    Btree btree;
    btree.open_to_write(btree_dir.c_str());

    if (full_compact) {
	tout << "Compact mode\n";
	btree.set_full_compaction(true);
    }
    
    int count;
    {
	ifstream f(datafile.c_str());
	TEST_AND_EXPLAIN(f.is_open(), "File " << datafile << " not found");
	count = process_lines(btree, f);
    }

    btree.commit(btree.revision_number + 1);

    return count;
}

static void do_create(const string & btree_dir, int block_size = 2048)
{
    delete_dir(btree_dir);
    make_dir(btree_dir);

    Btree::create(btree_dir.c_str(), block_size);
    tout << btree_dir << "/DB created with block size " << block_size << "\n";
}

/// Test playing with a btree
static bool test_simple1()
{
    string path = tmpdir + "test_simple1_";
    Btree::create(path, 8192);
    Btree btree;
    btree.open_to_read(path);

    string key = "foo";
    {
	Bcursor cursor(&btree);
	int found = cursor.find_key(key);
	TEST(!found);
    }
    {
	Bcursor cursor(&btree);
	int found = cursor.find_key(key);
	TEST(!found);
    }

    return true;
}

/// Test inserting and deleting items from a Btree
static bool test_insertdelete1()
{
    string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    if (!file_exists(datadir + "ord+") || !file_exists(datadir + "ord-"))
	SKIP_TEST("Data files not present");

    int count = do_update(btree_dir, datadir + "ord+");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);
    {
	Btree btree;
	btree.open_to_read(btree_dir.c_str());
	TEST_EQUAL(count, btree.item_count);
    }

    count += do_update(btree_dir, datadir + "ord-");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS | OPT_SHORT_TREE);
    {
	Btree btree;
	btree.open_to_read(btree_dir.c_str());
	TEST_EQUAL(btree.item_count, 0);
	TEST_EQUAL(count, btree.item_count);
    }

    return true;
}

/// Test sequential addition in a Btree
static bool test_sequent1()
{
    string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    if (!file_exists(datadir + "ordnum+") || !file_exists(datadir + "ordnum-"))
	SKIP_TEST("Data files not present");

    do_update(btree_dir, datadir + "ord+");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    do_update(btree_dir, datadir + "ordnum+");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    do_update(btree_dir, datadir + "ord-");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS | OPT_SHORT_TREE);

    do_update(btree_dir, datadir + "ordnum-");
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS | OPT_SHORT_TREE);

    Btree btree;
    btree.open_to_read(btree_dir.c_str());
    TEST_EQUAL(btree.item_count, 0);

    return true;
}

static bool test_emptykey1()
{
    string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree;
	btree.open_to_write(btree_dir.c_str());

	tout << "Setting tag to jam" << endl;
	btree.add("", "jam");
	btree.commit(btree.revision_number + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree;
	btree.open_to_write(btree_dir.c_str());
	TEST_EQUAL(btree.item_count, 0);

	tout << "Setting tag to marmite" << endl;
	btree.add("", "marmite");
	btree.commit(btree.revision_number + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree;
	btree.open_to_write(btree_dir.c_str());
	TEST_EQUAL(btree.item_count, 0);

	tout << "Deleting tag" << endl;
	btree.del("");
	btree.commit(btree.revision_number + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    {
	Btree btree;
	btree.open_to_write(btree_dir.c_str());
	TEST_EQUAL(btree.item_count, 0);

	tout << "Setting tag to butter" << endl;
	btree.add("", "butter");
	btree.add("test", "me");
	btree.commit(btree.revision_number + 1);
    }
    BTREE_CHECK(btree_dir, OPT_SHOW_STATS);

    Btree btree;
    btree.open_to_read(btree_dir.c_str());
    TEST_EQUAL(btree.item_count, 1);
 
    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================
//
// The lists of tests to perform
test_desc tests[] = {
    {"simple1",		test_simple1},
    {"insertdelete1",   test_insertdelete1},
    {"sequent1",        test_sequent1},
    {"emptykey1",       test_emptykey1},
    {0, 0}
};

int main(int argc, char **argv)
{
    char * e_tmpdir = getenv("BTREETMP");
    if (e_tmpdir) {
	tmpdir = e_tmpdir;
    } else {
	tmpdir = ".btreetmp/";
    }
    delete_dir(tmpdir);
    make_dir(tmpdir);
    test_driver::parse_command_line(argc, argv);
    datadir = test_driver::get_srcdir() + "/z_data/";
    return test_driver::run(tests);
}
