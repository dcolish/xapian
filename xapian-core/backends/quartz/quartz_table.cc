/* quartz_table.cc: A table in a quartz database
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
#include "omdebug.h"

#include "quartz_table.h"
#include "om/omerror.h"
#include "utils.h"
#include <string.h>
#include <errno.h>

// FIXME: just temporary
#include <stdio.h>

static std::string
readline(FILE *fp)
{
    std::string res;

    while(1) {
	int ch = fgetc(fp);
	if (ch == EOF) break;
	if (ch == '\0') break;
	if (ch == '\1') {
	    ch = fgetc(fp);
	    if (ch == EOF) break;
	}
	res += std::string(&((char)ch), 1);
    }

    return res;
}

static void
writeline(FILE *fp, std::string data)
{
    std::string::const_iterator i;
    for (i = data.begin(); i != data.end(); i++) {
	if (*i == '\0' || *i == '\1') {
	    fputc('\1', fp);
	}
	fputc(*i, fp);
    }
    fputc('\0', fp);
}

static void
writefile(std::string filename,
	  std::map<QuartzDbKey, QuartzDbTag> & data,
	  quartz_revision_number_t rev)
{
    FILE * fp = fopen(filename.c_str(), "w+");

    if (fp == 0) {
	throw OmDatabaseCorruptError(std::string("Can't access database: ") +
				     strerror(errno));
    }

    size_t items;
    items = fwrite((const void *) &rev,
		   sizeof(quartz_revision_number_t),
		   1,
		   fp);
    if (items != 1) {
	fclose(fp);
	throw OmDatabaseCorruptError("Can't write to Quartz table (" + filename + ")" + strerror(errno));
    }

    std::map<QuartzDbKey, QuartzDbTag>::const_iterator i;
    for (i = data.begin(); i != data.end(); i++) {
	writeline(fp, i->first.value);
	writeline(fp, i->second.value);
    }

    fclose(fp);
}

static void
readfile(std::string filename,
	 std::map<QuartzDbKey, QuartzDbTag> & data,
	 quartz_revision_number_t * rev,
	 bool readonly)
{
    FILE * fp = fopen(filename.c_str(), "r");

    if (fp == 0) {
	if(readonly)
	    throw OmOpeningError("Table `" + filename + "' does not exist.");
	*rev = 0;
	data.clear();
	writefile(filename, data, *rev);
	return;
    }

    size_t items;
    items = fread((void *) rev, sizeof(quartz_revision_number_t), 1, fp);
    if (items != 1) {
	fclose(fp);
	throw OmDatabaseCorruptError("Can't open Quartz table (" + filename + ")" + strerror(errno));
    }

    while(!feof(fp)) {
	QuartzDbKey key;
	QuartzDbTag tag ;
	key.value = readline(fp);
	if (feof(fp) && key.value != "") {
	    fclose(fp);
	    throw OmDatabaseCorruptError("Can't open Quartz table (" + filename + ") - no tag for key `" + key.value + "': " + strerror(errno));
	}

	tag.value = readline(fp);
	if (!feof(fp)) {
	    data[key] = tag;
	}
    }
    fclose(fp);
}

QuartzDiskTable::QuartzDiskTable(std::string path_,
				 bool readonly_,
				 unsigned int blocksize_)
	: path(path_),
	  blocksize(blocksize_),
	  opened(false),
          readonly(readonly_)
{
}

void
QuartzDiskTable::open()
{
    opened = false;

    if (readonly) {
	btree_for_reading.reset(Btree_open_to_read(path.c_str()));
	if (btree_for_reading.get() == 0) {
	    // FIXME: explain why
	    throw OmOpeningError("Cannot open table `"+path+"' for reading.");
	}
	opened = true;
	return;
    }

    // Create database if needed
    // FIXME: use btree library to check if table exists yet.
    if (!file_exists(path + "/DB")) {
	if (!Btree_create(path.c_str(), blocksize)) {
	    // FIXME: explain why
	    throw OmOpeningError("Cannot create table `" + path + "'.");
	}
    }

    btree_for_writing.reset(Btree_open_to_write(path.c_str()));
    if (btree_for_writing.get() == 0) {
	// FIXME: explain why
	throw OmOpeningError("Cannot open table `"+path+"' for writing.");
    }

    btree_for_reading.reset(Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number));
    if (btree_for_reading.get() == 0) {
	btree_for_writing.reset(0);
	throw OmOpeningError("Cannot open table `" + path +
			     "' for reading and writing.");
    }

    opened = true;
}

bool
QuartzDiskTable::open(quartz_revision_number_t revision)
{
    opened = false;

    if (readonly) {
	btree_for_reading.reset(Btree_open_to_read_revision(path.c_str(),
							    revision));
	if (btree_for_reading.get() == 0) {
	    // FIXME: throw an exception if it's not just this revision which
	    // unopenable.
	    return false;
	}
	opened = true;
	return true;
    }

    // Create database if needed
    // FIXME: use btree library to check if table exists yet.
    if (!file_exists(path + "/DB")) {
	if (!Btree_create(path.c_str(), blocksize)) {
	    // FIXME: explain why
	    throw OmOpeningError("Cannot create table `" + path + "'.");
	}
    }

    btree_for_writing.reset(Btree_open_to_write_revision(path.c_str(),
							 revision));
    if (btree_for_writing.get() == 0) {
	// FIXME: throw an exception if it's not just this revision which
	// unopenable.
	return false;
    }

    Assert(btree_for_writing->revision_number = revision);

    btree_for_reading.reset(Btree_open_to_read_revision(path.c_str(),
				btree_for_writing->revision_number));
    if (btree_for_reading.get() == 0) {
	btree_for_writing.reset(0);
	// FIXME: throw an exception if it's not just this revision which
	// unopenable.
	return false;
    }

    Assert(btree_for_reading->revision_number = revision);

    opened = true;
    return true;
}

QuartzDiskTable::~QuartzDiskTable()
{
}

quartz_revision_number_t
QuartzDiskTable::get_open_revision_number() const
{
    Assert(opened);
    return btree_for_reading->revision_number;
}

quartz_revision_number_t
QuartzDiskTable::get_latest_revision_number() const
{
    // FIXME: implement with a call to martin's code
}

quartz_tablesize_t
QuartzDiskTable::get_entry_count() const
{
    Assert(opened);
    return btree_for_reading->item_count;
}

bool
QuartzDiskTable::get_nearest_entry(QuartzDbKey &key,
				   QuartzDbTag &tag,
				   QuartzCursor &cursor) const
{
    Assert(opened);
    Assert(!(key.value.empty()));

    /// FIXME: replace with calls to martin's code
    std::map<QuartzDbKey, QuartzDbTag>::const_iterator j;
    j = data.lower_bound(key);

    if (j != data.end() && j->first.value == key.value) {
	// Exact match
	tag.value = j->second.value;
	return true;
    }

    if (j == data.begin()) {
	// Nothing before this match
	key.value = "";
	tag.value = "";
	return false;
    }
    
    // Make j point to match _before_ that searched for.
    j--;

    key.value = (j->first).value;
    tag.value = (j->second).value;
    return false;
}

bool
QuartzDiskTable::get_next_entry(QuartzDbKey &key,
				QuartzDbTag &tag,
				QuartzCursor &cursor) const
{
    Assert(opened);
}

bool
QuartzDiskTable::get_exact_entry(const QuartzDbKey &key, QuartzDbTag & tag) const
{
    Assert(opened);
    Assert(!(key.value.empty()));

    /// FIXME: replace with calls to martin's code
    std::map<QuartzDbKey, QuartzDbTag>::const_iterator j = data.find(key);
    if (j == data.end()) {
	return false;
    }
    tag.value = (j->second).value;
    return true;
}

bool
QuartzDiskTable::set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries,
			     quartz_revision_number_t new_revision)
{
    Assert(opened);
    if(readonly) throw OmInvalidOperationError("Attempt to set entries in a readonly table.");

    // Find out which table is not opened
    std::map<QuartzDbKey, QuartzDbTag> data1;
    quartz_revision_number_t rev1;
    readfile(path + "data_1", data1, &rev1, readonly);

    std::map<QuartzDbKey, QuartzDbTag> data2;
    quartz_revision_number_t rev2;
    readfile(path + "data_2", data2, &rev2, readonly);

    data1.clear();
    data2.clear();


    // FIXME: replace with calls to martin's code
    {
	std::map<QuartzDbKey, QuartzDbTag *>::const_iterator i;
	for (i = entries.begin(); i != entries.end(); i++) {
	    Assert(!((i->first).value.empty()));
	    std::map<QuartzDbKey, QuartzDbTag>::iterator j;
	    j = data.find(i->first);
	    if (i->second == 0) {
		// delete j
		if (j != data.end()) {
		    data.erase(j);
		}
	    } else {
		if (j == data.end()) {
		    data.insert(make_pair(i->first, *(i->second)));
		} else {
		    if ((j->second).value != (*(i->second)).value) {
			j->second = *(i->second);
		    }
		}
	    }
	}
    }


    // Write data
    if(revision == rev1) {
	revision = new_revision;
	writefile(path + "data_2", data, revision);
    } else {
	Assert(revision == rev2);
	revision = new_revision;
	writefile(path + "data_1", data, revision);
    }

    return true;
}



QuartzBufferedTable::QuartzBufferedTable(QuartzDiskTable * disktable_)
	: disktable(disktable_),
	  entry_count(disktable->get_entry_count())
{
}

QuartzBufferedTable::~QuartzBufferedTable()
{
}

bool
QuartzBufferedTable::apply(quartz_revision_number_t new_revision)
{
    bool result;
    try {
	result = disktable->set_entries(changed_entries.get_all_entries(),
					new_revision);
    } catch (...) {
	changed_entries.clear();
	throw;
    }
    changed_entries.clear();
    Assert(entry_count == disktable->get_entry_count());
    return result;
}

void
QuartzBufferedTable::cancel()
{
    changed_entries.clear();
    entry_count = disktable->get_entry_count();
}

bool
QuartzBufferedTable::is_modified()
{
    return !changed_entries.empty();
}

QuartzDbTag *
QuartzBufferedTable::get_tag(const QuartzDbKey &key)
{
    if (changed_entries.have_entry(key)) {
	return changed_entries.get_tag(key);
    } else {
	AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
	QuartzDbTag * tagptr = tag.get();

	bool found = disktable->get_exact_entry(key, *tagptr);

	if (found) {
	    changed_entries.set_tag(key, tag);
	    Assert(changed_entries.get_tag(key) == tagptr);
	} else {
	    tagptr = 0;
	}

	return tagptr;
    }
}

QuartzDbTag *
QuartzBufferedTable::get_or_make_tag(const QuartzDbKey &key)
{
    QuartzDbTag * tagptr;

    // Check cache first
    if (changed_entries.have_entry(key)) {
	tagptr = changed_entries.get_tag(key);
	if (tagptr == 0) {
	    // make new empty tag
	    AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
	    tagptr = tag.get();
	    changed_entries.set_tag(key, tag);
	    entry_count += 1;

	    Assert(changed_entries.get_tag(key) == tagptr);
	    Assert(tag.get() == 0);

	    return tagptr;
	} else {
	    return tagptr;
	}
    }

    AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
    tagptr = tag.get();

    bool found = disktable->get_exact_entry(key, *tag);

    changed_entries.set_tag(key, tag);
    if (found && tagptr == 0) {
	Assert(entry_count != 0);
	entry_count -= 1;
    } else if (!found && tagptr != 0) {
	entry_count += 1;
    }
    Assert(changed_entries.have_entry(key));
    Assert(changed_entries.get_tag(key) == tagptr);
    Assert(tag.get() == 0);

    return tagptr;
}

void
QuartzBufferedTable::delete_tag(const QuartzDbKey &key)
{
    // This reads the tag to check if it currently exists, so we can keep
    // track of the number of entries in the table.
    if (get_tag(key) != 0) {
	entry_count -= 1;
    }
    changed_entries.set_tag(key, AutoPtr<QuartzDbTag>(0));
}

quartz_tablesize_t
QuartzBufferedTable::get_entry_count() const
{
    return entry_count;
}

bool
QuartzBufferedTable::get_nearest_entry(QuartzDbKey &key,
				       QuartzDbTag &tag,
				       QuartzCursor &cursor) const
{
    // FIXME: look up in changed_entries too.
    return disktable->get_nearest_entry(key, tag, cursor);
}

bool
QuartzBufferedTable::get_next_entry(QuartzDbKey &key,
				    QuartzDbTag &tag,
				    QuartzCursor &cursor) const
{
}

bool
QuartzBufferedTable::get_exact_entry(const QuartzDbKey &key,
				     QuartzDbTag & tag) const
{
    if (changed_entries.have_entry(key)) {
	tag = *(changed_entries.get_tag(key));
	return true;
    }

    return disktable->get_exact_entry(key, tag);
}

