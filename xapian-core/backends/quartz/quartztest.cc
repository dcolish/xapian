/* quartztest.cc: test of the Quartz backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include "testsuite.h"
#include "testutils.h"
#include "om/omerror.h"

#include "quartz_database.h"
#include "quartz_table.h"
#include "quartz_table_entries.h"
#include "quartz_utils.h"

#include "database_builder.h"

#include "om/autoptr.h"

#include "unistd.h"

/// Check the values returned by a table containing key/tag "hello"/"world"
static void check_table_values_hello(const QuartzDiskTable & table,
				     std::string world)
{
    QuartzDbKey key;
    QuartzDbTag tag;

    // Check exact reads
    key.value = "hello";
    tag.value = "foo";
    TEST(table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, world);

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");
    
    QuartzCursor cursor;
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(tag.value, "foo");
#endif
    
    // Check normal reads
    key.value = "hello";
    tag.value = "foo";
    TEST(table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "hello");
    TEST_EQUAL(tag.value, world);

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "hello");
    TEST_EQUAL(tag.value, world);

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");
    
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "foo");
#endif
}

/// Check the values returned by a table containing no key/tag pairs
static void check_table_values_empty(const QuartzDiskTable & table)
{
    QuartzDbKey key;
    QuartzDbTag tag;

    // Check exact reads
    key.value = "hello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_exact_entry(key, tag));
    TEST_EQUAL(tag.value, "foo");
    
    QuartzCursor cursor;
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(tag.value, "foo");
#endif
    
    // Check normal reads
    key.value = "hello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");

    key.value = "jello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");

    key.value = "bello";
    tag.value = "foo";
    TEST(!table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "");
    
#ifdef MUS_DEBUG
    key.value = "";
    tag.value = "foo";
    TEST_EXCEPTION(OmAssertionError, table.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "");
    TEST_EQUAL(tag.value, "foo");
#endif
}

/// Test making and playing with a QuartzDiskTable
static bool test_disktable1()
{
    unlink("./test_dbtable1_data_1");
    unlink("./test_dbtable1_data_2");
    {
	QuartzDiskTable table0("./test_dbtable1_", true, 0);
	TEST_EXCEPTION(OmOpeningError, table0.open());
	TEST_EXCEPTION(OmOpeningError, table0.open(10));
    }
    QuartzDiskTable table2("./test_dbtable1_", false, 8192);
    table2.open();
    QuartzDiskTable table1("./test_dbtable1_", true, 0);
    table1.open();


    quartz_revision_number_t rev1 = table1.get_open_revision_number();
    quartz_revision_number_t rev2 = table2.get_open_revision_number();

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_EQUAL(rev2, table2.get_open_revision_number());
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    std::map<QuartzDbKey, QuartzDbTag *> newentries;

    // Check adding no entries
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    // Check adding some entries
    QuartzDbKey key;
    QuartzDbTag tag;
    key.value = "hello";
    tag.value = "world";
    newentries[key] = &tag;
    
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 1);

    // Check getting the entries out again
    check_table_values_empty(table1);
    check_table_values_hello(table2, "world");

    // Check adding the same entries
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 1);

    // Check getting the entries out again
    check_table_values_empty(table1);
    check_table_values_hello(table2, "world");


    // Check adding an entry with a null key
    key.value = "";
    newentries[key] = &tag;
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
#ifdef MUS_DEBUG
    TEST_EXCEPTION(OmAssertionError,
		   table2.set_entries(newentries,
				      table2.get_latest_revision_number() + 1));
#endif

    // Check changing an entry, to a null tag
    newentries.clear();
    key.value = "hello";
    tag.value = "";
    newentries[key] = &tag;
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 1);
    
    // Check getting the entries out again
    check_table_values_empty(table1);
    check_table_values_hello(table2, "");

    // Check deleting an entry
    newentries.clear();
    key.value = "hello";
    newentries[key] = 0;
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 0);

    // Check the entries in the table
    check_table_values_empty(table1);
    check_table_values_empty(table2);
    
    // Check get_nearest_entry when looking for something between two elements
    newentries.clear();
    key.value = "hello";
    tag.value = "world";
    newentries[key] = &tag;
    key.value = "whooo";
    tag.value = "world";
    newentries[key] = &tag;
    TEST_EXCEPTION(OmInvalidOperationError,
		   table1.set_entries(newentries,
				      table1.get_latest_revision_number() + 1));
    table2.set_entries(newentries,
		       table2.get_latest_revision_number() + 1);

    TEST_EQUAL(rev1, table1.get_open_revision_number());
    TEST_NOT_EQUAL(rev2, table2.get_open_revision_number());
    rev1 = table1.get_open_revision_number();
    rev2 = table2.get_open_revision_number();
    TEST_EQUAL(table1.get_entry_count(), 0);
    TEST_EQUAL(table2.get_entry_count(), 2);

    // Check the entries in the table
    check_table_values_empty(table1);
    check_table_values_hello(table2, "world");
    
    return true;
}

/// Test making and playing with a QuartzTableEntries
static bool test_tableentries1()
{
    QuartzTableEntries entries;

    QuartzDbKey key1;

#ifdef MUS_DEBUG
    key1.value="";
    TEST_EXCEPTION(OmAssertionError, entries.have_entry(key1));
    {
	AutoPtr<QuartzDbTag> tagptr(new QuartzDbTag);
	tagptr->value = "bar";
	TEST_EXCEPTION(OmAssertionError, entries.set_tag(key1, tagptr));
    }
    TEST_EXCEPTION(OmAssertionError, entries.have_entry(key1));
    TEST_EXCEPTION(OmAssertionError, entries.get_tag(key1));
#endif

    key1.value="foo";
    TEST(!entries.have_entry(key1));
    {
	AutoPtr<QuartzDbTag> tagptr(new QuartzDbTag);
	tagptr->value = "bar";
	entries.set_tag(key1, tagptr);
    }
    TEST(entries.have_entry(key1));
    TEST_NOT_EQUAL(entries.get_tag(key1), 0);
    TEST_EQUAL(entries.get_tag(key1)->value, "bar");
    {
	AutoPtr<QuartzDbTag> tagptr(0);
	entries.set_tag(key1, tagptr);
    }
    TEST(entries.have_entry(key1));
    TEST_EQUAL(entries.get_tag(key1), 0);

    return true;
}

/// Test making and playing with a QuartzBufferedTable
static bool test_bufftable1()
{
    unlink("./test_bufftable1_data_1");
    unlink("./test_bufftable1_data_2");
    QuartzDiskTable disktable1("./test_bufftable1_", false, 8192);
    disktable1.open();
    QuartzBufferedTable bufftable1(&disktable1);

    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    QuartzDbKey key;
    key.value = "foo1";

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    TEST_EQUAL((void *)bufftable1.get_tag(key), 0);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    QuartzDbTag * tag = bufftable1.get_or_make_tag(key);
    TEST_NOT_EQUAL(tag, 0);
    TEST_EQUAL(disktable1.get_entry_count(), 0);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    quartz_revision_number_t new_revision =
	    disktable1.get_latest_revision_number() + 1;
    TEST(bufftable1.apply(new_revision));
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    tag = bufftable1.get_or_make_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    bufftable1.delete_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 0);

    key.value = "bar";
    tag = bufftable1.get_or_make_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 1);

    key.value = "bar2";
    tag = bufftable1.get_or_make_tag(key);
    TEST_EQUAL(disktable1.get_entry_count(), 1);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    new_revision += 1;
    TEST(bufftable1.apply(new_revision));

    TEST_EQUAL(disktable1.get_entry_count(), 2);
    TEST_EQUAL(bufftable1.get_entry_count(), 2);

    return true;
}

/// Test making and playing with a QuartzBufferedTable
static bool test_bufftable2()
{
    unlink("./test_bufftable2_data_1");
    unlink("./test_bufftable2_data_2");
    quartz_revision_number_t new_revision;
    quartz_revision_number_t old_revision;
    {
	// Open table and add a few documents
	QuartzDiskTable disktable("./test_bufftable2_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 0);
	TEST_EQUAL(bufftable.get_entry_count(), 0);

	QuartzDbKey key;
	QuartzDbTag tag;

	key.value = "foo1";
	bufftable.get_or_make_tag(key)->value = "bar1";
	key.value = "foo2";
	bufftable.get_or_make_tag(key)->value = "bar2";
	key.value = "foo3";
	bufftable.get_or_make_tag(key)->value = "bar3";

	new_revision = disktable.get_latest_revision_number() + 1;
	TEST(bufftable.apply(new_revision));

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	QuartzDiskTable disktable("./test_bufftable2_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 3);
	TEST_EQUAL(bufftable.get_entry_count(), 3);

	QuartzDbKey key;
	QuartzDbTag tag;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key.value = "foo";
	tag.value = "";
	QuartzCursor cursor;
	TEST(!bufftable.get_nearest_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "");
	TEST_EQUAL(tag.value, "");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo1");
	TEST_EQUAL(tag.value, "bar1");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo2");
	TEST_EQUAL(tag.value, "bar2");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");

	TEST(!bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");

	// Add a new tag
	key.value = "foo25";
	bufftable.get_or_make_tag(key)->value = "bar25";
	old_revision = new_revision;
	new_revision += 1;
	TEST(bufftable.apply(new_revision));

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Open old revision
	QuartzDiskTable disktable("./test_bufftable2_", false, 8192);
	TEST(disktable.open(old_revision));
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 3);
	TEST_EQUAL(bufftable.get_entry_count(), 3);

	QuartzDbKey key;
	QuartzDbTag tag;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(old_revision, disktable.get_open_revision_number());

	// Add a new tag
	key.value = "foo26";
	bufftable.get_or_make_tag(key)->value = "bar26";
	new_revision += 1;
	TEST(bufftable.apply(new_revision));

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());
    }
    {
	// Reopen and check that the documents are still there.
	QuartzDiskTable disktable("./test_bufftable2_", false, 8192);
	disktable.open();
	QuartzBufferedTable bufftable(&disktable);

	TEST_EQUAL(disktable.get_entry_count(), 4);
	TEST_EQUAL(bufftable.get_entry_count(), 4);

	QuartzDbKey key;
	QuartzDbTag tag;

	TEST_EQUAL(new_revision, disktable.get_latest_revision_number());
	TEST_EQUAL(new_revision, disktable.get_open_revision_number());

	key.value = "foo";
	tag.value = "";
	QuartzCursor cursor;
	TEST(!bufftable.get_nearest_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "");
	TEST_EQUAL(tag.value, "");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo1");
	TEST_EQUAL(tag.value, "bar1");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo2");
	TEST_EQUAL(tag.value, "bar2");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");

	TEST(bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo36");
	TEST_EQUAL(tag.value, "bar36");

	TEST(!bufftable.get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");
    }
    // Check that opening a nonexistant revision returns false (but doesn't
    // throw an exception).
    TEST(!disktable.open(new_revision + 10));

    return true;
}

/// Test QuartzCursors
static bool test_cursor1()
{
    unlink("./test_cursor1_data_1");
    unlink("./test_cursor1_data_2");

    QuartzDbKey key;
    QuartzDbTag tag;

    // Open table and put stuff in it.
    QuartzDiskTable disktable1("./test_cursor1_", false, 8192);
    disktable1.open();
    QuartzBufferedTable bufftable1(&disktable1);

    key.value = "foo1";
    bufftable1.get_or_make_tag(key)->value = "bar1";
    key.value = "foo2";
    bufftable1.get_or_make_tag(key)->value = "bar2";
    key.value = "foo3";
    bufftable1.get_or_make_tag(key)->value = "bar3";
    quartz_revision_number_t new_revision = disktable1.get_latest_revision_number();
    new_revision += 1;
    TEST(bufftable1.apply(new_revision));

    QuartzTable * table = &disktable1;
    int count = 2;

    QuartzCursor cursor;
    while(count != 0) {
	key.value = "foo25";
	tag.value = "";
	TEST(!table->get_nearest_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo2");
	TEST_EQUAL(tag.value, "bar2");

	TEST(table->get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");

	TEST(!table->get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");

	key.value = "foo";
	tag.value = "blank";
	TEST(!table->get_nearest_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "");
	TEST_EQUAL(tag.value, "");

	TEST(table->get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo1");
	TEST_EQUAL(tag.value, "bar1");

	key.value = "foo2";
	tag.value = "";
	TEST(table->get_nearest_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo2");
	TEST_EQUAL(tag.value, "bar2");

	TEST(table->get_next_entry(key, tag, cursor));
	TEST_EQUAL(key.value, "foo3");
	TEST_EQUAL(tag.value, "bar3");

	table = &bufftable1;
	count -= 1;
    }

    // Test cursors when we have unapplied changes
    key.value = "foo25";
    bufftable1.get_or_make_tag(key)->value = "bar25";

    key.value = "foo25";
    tag.value = "";
    TEST(!disktable1.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo2");
    TEST_EQUAL(tag.value, "bar2");

    TEST(disktable1.get_next_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo3");
    TEST_EQUAL(tag.value, "bar3");

    key.value = "foo25";
    tag.value = "";
    TEST(!bufftable1.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo25");
    TEST_EQUAL(tag.value, "bar25");

    TEST(bufftable1.get_next_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo3");
    TEST_EQUAL(tag.value, "bar3");

    key.value = "foo2";
    tag.value = "";
    TEST(!bufftable1.get_nearest_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo2");
    TEST_EQUAL(tag.value, "bar2");

    TEST(bufftable1.get_next_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo25");
    TEST_EQUAL(tag.value, "bar25");

    TEST(bufftable1.get_next_entry(key, tag, cursor));
    TEST_EQUAL(key.value, "foo3");
    TEST_EQUAL(tag.value, "bar3");

    return true;
}

/// Test opening of a quartz database
static bool test_open1()
{
    OmSettings settings;
    system("rm -fr .testdb_open1");
    settings.set("quartz_dir", ".testdb_open1");
    settings.set("backend", "quartz");

    TEST_EXCEPTION(OmOpeningError,
		   RefCntPtr<Database> database_0 =
		   DatabaseBuilder::create(settings, true));

    system("mkdir .testdb_open1");
    RefCntPtr<Database> database_w =
	    DatabaseBuilder::create(settings, false);
    RefCntPtr<Database> database_r =
	    DatabaseBuilder::create(settings, true);
    return true;
}

/** Test adding and deleting a document, and that flushing occurs in a
 *  sensible manner.
 */
static bool test_adddoc1()
{
    OmSettings settings;
    system("rm -fr .testdb_adddoc1");
    system("mkdir .testdb_adddoc1");
    settings.set("quartz_dir", ".testdb_adddoc1");
    settings.set("quartz_logfile", "log");
    settings.set("backend", "quartz");

    RefCntPtr<Database> database = DatabaseBuilder::create(settings, false);

    database->begin_session(0);
    TEST_EQUAL(database->get_doccount(), 0);
    TEST_EQUAL(database->get_avlength(), 0);
    OmDocumentContents document;
    om_docid did;

    did = database->add_document(document);
    TEST_EQUAL(database->get_doccount(), 1);
    TEST_EQUAL(did, 1);
    TEST_EQUAL(database->get_avlength(), 0);
    settings.set("quartz_logfile", "log_ro");
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 0);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }
    database->flush();
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 1);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }

    database->delete_document(did);
    TEST_EQUAL(database->get_doccount(), 0);
    TEST_EQUAL(database->get_avlength(), 0);
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 1);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }
    database->flush();
    {
	RefCntPtr<Database> db_readonly =
		DatabaseBuilder::create(settings, true);
	TEST_EQUAL(db_readonly->get_doccount(), 0);
	TEST_EQUAL(db_readonly->get_avlength(), 0);
    }

    did = database->add_document(document);
    TEST_EQUAL(database->get_doccount(), 1);
    TEST_EQUAL(did, 2);
    TEST_EQUAL(database->get_avlength(), 0);

    database->flush();
    database->end_session();

    return true;
}

/** Test adding a document, and checking that it got added correctly.
 */
static bool test_adddoc2()
{
    OmSettings settings;
    system("rm -fr .testdb_adddoc2");
    system("mkdir .testdb_adddoc2");
    settings.set("quartz_dir", ".testdb_adddoc2");
    settings.set("quartz_logfile", "log");
    settings.set("backend", "quartz");

    om_docid did;
    OmDocumentContents document_in;
    document_in.data.value = "Foobar rising";
    document_in.keys[7] = OmKey("Key7");
    document_in.keys[13] = OmKey("Key13");
    document_in.add_posting("foobar", 1);
    document_in.add_posting("rising", 2);
    document_in.add_posting("foobar", 3);

    OmDocumentContents document_in2;
    document_in2.data.value = "Foobar falling";
    document_in2.add_posting("foobar", 1);
    document_in2.add_posting("falling", 2);
    {
	RefCntPtr<Database> database = DatabaseBuilder::create(settings, false);

	TEST_EQUAL(database->get_doccount(), 0);
	TEST_EQUAL(database->get_avlength(), 0);

	did = database->add_document(document_in);
	TEST_EQUAL(database->get_doccount(), 1);
	TEST_EQUAL(database->get_avlength(), 3);

	om_docid did2 = database->add_document(document_in2);
	TEST_EQUAL(database->get_doccount(), 2);
	TEST_NOT_EQUAL(did, did2);
	TEST_EQUAL(database->get_avlength(), 5.0/2.0);

	database->delete_document(did);
	TEST_EQUAL(database->get_doccount(), 1);
	TEST_EQUAL(database->get_avlength(), 2);

	did = database->add_document(document_in);
	TEST_EQUAL(database->get_doccount(), 2);
	TEST_EQUAL(database->get_avlength(), 5.0/2.0);
    }

    {
	settings.set("quartz_logfile", "log_ro");
	RefCntPtr<Database> database = DatabaseBuilder::create(settings, true);
	OmDocumentContents document_out = database->get_document(did);

	TEST_EQUAL(document_in.data.value, document_out.data.value);
	TEST_EQUAL(document_in.keys.size(), document_out.keys.size());
	TEST_EQUAL(document_in.terms.size(), document_out.terms.size());

	{
	    OmDocumentContents::document_keys::const_iterator i,j;
	    for (i = document_in.keys.begin(), j = document_out.keys.begin();
		 i != document_in.keys.end();
		 i++, j++) {
		TEST_EQUAL(i->first, j->first);
		TEST_EQUAL(i->second.value, j->second.value);
	    }
	}
	{
	    OmDocumentContents::document_terms::const_iterator i,j;
	    for (i = document_in.terms.begin(), j = document_out.terms.begin();
		 i != document_in.terms.end();
		 i++, j++) {
		TEST_EQUAL(i->first, j->first);
		TEST_EQUAL(i->second.tname, j->second.tname);
		TEST_EQUAL(i->second.wdf, j->second.wdf);
		TEST_NOT_EQUAL(i->second.termfreq, j->second.termfreq);
		TEST_EQUAL(0, i->second.termfreq);
		TEST_EQUAL(2, j->second.termfreq);
		TEST_EQUAL(i->second.positions.size(),
			   j->second.positions.size());
		OmDocumentTerm::term_positions::const_iterator k,l;
		for (k = i->second.positions.begin(),
		     l = j->second.positions.begin();
		     k != i->second.positions.end();
		     k++, l++) {
		    TEST_EQUAL(*k, *l);
		}
	    }
	}
    }

    return true;
}

/// Test packing integers into strings
static bool test_packint1()
{
    TEST_EQUAL(pack_uint(0u), std::string("\000", 1));
    TEST_EQUAL(pack_uint(1u), std::string("\001", 1));
    TEST_EQUAL(pack_uint(127u), std::string("\177", 1));
    TEST_EQUAL(pack_uint(128u), std::string("\200\001", 2));
    TEST_EQUAL(pack_uint(0xffffu), std::string("\377\377\003", 3));
    TEST_EQUAL(pack_uint(0xffffffffu), std::string("\377\377\377\377\017", 5));

    return true;
}

/// Test packing integers into strings and unpacking again
static bool test_packint2()
{
    std::string foo;

    foo += pack_uint(3u);
    foo += pack_uint(12475123u);
    foo += pack_uint(128u);
    foo += pack_uint(0xffffffffu);
    foo += pack_uint(127u);
    foo += pack_uint(0u);
    foo += pack_uint(0xffffffffu);
    foo += pack_uint(0u);
    foo += pack_uint(82134u);

    const char * p = foo.data();
    om_uint32 result;

    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 3u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 12475123u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 128u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0xffffffffu);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 127u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0xffffffffu);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 0u);
    TEST(unpack_uint(&p, foo.data() + foo.size(), &result));
    TEST_EQUAL(result, 82134u);

    return true;
}

/// Test unpacking integers from strings
static bool test_unpackint1()
{
    std::string foo;
    const char *p;
    om_uint32 result;
    bool success;
    
    p = foo.data();
    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    foo = std::string("\000\002\301\001", 4);
    result = 1;
    p = foo.data();

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0);
    TEST_EQUAL((void *)p, (void *)(foo.data() + 1));

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 2);
    TEST_EQUAL(p, foo.data() + 2);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 65 + 128);
    TEST_EQUAL(p, foo.data() + 4);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    foo = std::string("\377\377\377\377\017\377\377\377\377\020\007\200\200\200\200\200\200\200\000\200\200\200\200\200\200\001\200\200\200\200\200\200", 32);
    result = 1;
    p = foo.data();

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 0xffffffff);
    TEST_EQUAL(p, foo.data() + 5);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 10);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(success);
    TEST_EQUAL(result, 7);
    TEST_EQUAL(p, foo.data() + 11);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 19);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, foo.data() + 26);

    success = unpack_uint(&p, foo.data() + foo.size(), &result);
    TEST(!success);
    TEST_EQUAL(p, 0);

    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================
//
// Tests to write:
//
// Check behaviour of attributes - write same attribute twice, test reading
// single attributes which exist and don't exist / have been deleted.

/// The lists of tests to perform
test_desc tests[] = {
    {"quartzdisktable1",	test_disktable1},
    {"quartztableentries1",	test_tableentries1},
    {"quartzbufftable1",	test_bufftable1},
    {"quartzbufftable2",	test_bufftable2},
    {"quartzcursor1",		test_cursor1},
    {"quartzopen1",		test_open1},
    {"quartzadddoc1",		test_adddoc1},
    {"quartzadddoc2",		test_adddoc2},
    {"quartzpackint1",		test_packint1},
    {"quartzpackint2",		test_packint2},
    {"quartzunpackint1",	test_unpackint1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
