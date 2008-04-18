/* chert_record.cc: Records in chert databases
 *
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2008 Olly Betts
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
#include "chert_record.h"
#include "chert_utils.h"
#include "utils.h"
#include <xapian/error.h>
#include "omassert.h"
#include "omdebug.h"

using std::string;

string
ChertRecordTable::get_record(Xapian::docid did) const
{
    DEBUGCALL(DB, string, "ChertRecordTable::get_record", did);
    string tag;

    if (!get_exact_entry(chert_docid_to_key(did), tag)) {
	throw Xapian::DocNotFoundError("Document " + om_tostring(did) + " not found.");
    }

    RETURN(tag);
}

Xapian::doccount
ChertRecordTable::get_doccount() const
{   
    DEBUGCALL(DB, Xapian::doccount, "ChertRecordTable::get_doccount", "");
    STATIC_ASSERT_TYPE_DOMINATES(Xapian::doccount, chert_tablesize_t);
    RETURN(get_entry_count());
}

void
ChertRecordTable::replace_record(const string & data, Xapian::docid did)
{
    DEBUGCALL(DB, void, "ChertRecordTable::replace_record", data << ", " << did);
    add(chert_docid_to_key(did), data);
}

void
ChertRecordTable::delete_record(Xapian::docid did)
{
    DEBUGCALL(DB, void, "ChertRecordTable::delete_record", did);
    if (!del(chert_docid_to_key(did)))
	throw Xapian::DocNotFoundError("Can't delete non-existent document #" + om_tostring(did));
}
