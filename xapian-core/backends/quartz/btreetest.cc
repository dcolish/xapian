/* btreetest.cc: test of the btree manager
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
#include <stdio.h>   /* fprintf etc */
#include "btree.h"
#include "testsuite.h"
#include "testutils.h"
#include "utils.h"
#include <string>
using std::string;

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

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

static int min(int i, int max)
{
    return i > max ? max : i;
}

static void process_lines(Btree & btree, FILE * f)
{
    int line_count = 0;
    unsigned char * s = (unsigned char *) malloc(10000);

    while (true) {
	int i = 0;
        int j = 0;
        int mode = 0;
        while (true) {
	    int ch = getc(f);
            if (ch == EOF) { free(s); return; }
            if (ch == ' ') {
		if (i == 0) continue;
                if (j == 0) j = i;
            }
            if (ch == '+' && i == 0) { mode = 1; continue; }
            if (ch == '-' && i == 0) { mode = 2; continue; }
            if (ch == '\n') {
                line_count++;
                if (mode == 0) {
		    if (i == 0) break;
		    fprintf(stderr, "No '+' or '-' on line %d\n", line_count);
		    exit(1);
                } else if (mode == 1) {
		    /*if (i > 0)*/
                        if (j > 0) {
			    btree.add(s, min(j, btree.max_key_len), s + j + 1, i - j - 1);
			} else {
			    btree.add(s, min(i, btree.max_key_len), s, 0);
			}
                    break;
                } else {
		    /*if (i > 0)*/
                        if (j > 0) {
			    btree.delete_(s, min(j, btree.max_key_len));
			} else {
			    btree.delete_(s, min(i, btree.max_key_len));
			}
                    break;
                }
            }
            s[i++] = ch;
        }
    }
}

static void do_update(const string & btree_dir,
		     const string & datafile,
		     bool full_compact = false)
{
    Btree btree;
    btree.open_to_write(btree_dir.c_str());

    if (full_compact) {
	tout << "Compact mode\n";
	btree.set_full_compaction(true);
    }
    
    FILE * f = fopen(datafile.c_str(), "r");
    TEST_AND_EXPLAIN(f != 0, "File " << datafile << " not found");

    process_lines(btree, f);
    fclose(f);

    btree.commit(btree.revision_number + 1);
}

static void do_check(const string & btree_dir, const string & args)
{
    Btree::check(btree_dir.c_str(), args.c_str());
}

static void do_create(const string & btree_dir, int block_size = 1024)
{
    delete_dir(btree_dir);
    make_dir(btree_dir);

    Btree::create(btree_dir.c_str(), block_size);
    tout << btree_dir << "/DB created with block size " << block_size << "\n";
}

/// Test making and playing with a QuartzBufferedTable
static bool test_insertdelete1()
{
    string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    do_check(btree_dir, "v");

    if (!file_exists(datadir + "ord+") || !file_exists(datadir + "ord-"))
	SKIP_TEST("Data files not present");

    do_update(btree_dir, datadir + "ord+");
    do_check(btree_dir, "v");

    do_update(btree_dir, datadir + "ord-");
    do_check(btree_dir, "vt");

    Btree btree;
    btree.open_to_read(btree_dir.c_str());
    TEST_EQUAL(btree.item_count, 0);

    return true;
}

#if 0 // FIXME: implement this!  Currently it's a copy of test_insertdelete1()
/// Test making and playing with a QuartzBufferedTable
/// try to pass the 2G boundry.  Should succeed if LFS is enabled
static bool test_LFSinsertdelete1()
{
    bool LFSunlikely=sizeof(off_t)==4;

    string btree_dir = tmpdir + "/B/";
    do_create(btree_dir);
    do_check(btree_dir, "v");

    if (!file_exists(datadir + "ord+") || !file_exists(datadir + "ord-"))
	SKIP_TEST("Data files not present");

    do_update(btree_dir, datadir + "ord+");
    do_check(btree_dir, "v");

    do_update(btree_dir, datadir + "ord-");
    do_check(btree_dir, "vt");

    Btree btree;
    btree.open_to_read(btree_dir.c_str());
    TEST_EQUAL(btree.item_count, 0);

    return true;
}
#endif

// ================================
// ========= END OF TESTS =========
// ================================
//
// The lists of tests to perform
test_desc tests[] = {
    {"insertdelete1",         test_insertdelete1},
// FIXME:    {"LFSinsertdelete1",      test_LFSinsertdelete1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    char * e_tmpdir = getenv("BTREETMP");
    if (e_tmpdir) {
	tmpdir = e_tmpdir;
    } else {
	tmpdir = ".btreetmp/";
    }
    delete_dir(tmpdir);
    make_dir(tmpdir);
    datadir = test_driver::get_srcdir(argv[0]) + "/z_data/";
    return test_driver::main(argc, argv, tests);
}
