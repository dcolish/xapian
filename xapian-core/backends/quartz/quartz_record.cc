/* quartz_record.cc: Records in quartz databases
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
#include "quartz_record.h"
#include "quartz_utils.h"
#include "utils.h"
#include "xapian/error.h"
#include "omassert.h"
#include "omdebug.h"
using std::string;

// Magic key (which corresponds to an invalid docid) is used to store the
// next free docid and total length of all documents
static const string METAINFO_KEY("", 1);

string
QuartzRecordManager::get_record(QuartzTable & table, om_docid did)
{
    DEBUGCALL_STATIC(DB, string, "QuartzRecordManager::get_record", "[table], " << did);
    string key(quartz_docid_to_key(did));
    string tag;

    if (!table.get_exact_entry(key, tag)) {
	throw Xapian::DocNotFoundError("Document " + om_tostring(did) + " not found.");
    }

    RETURN(tag);
}


om_doccount
QuartzRecordManager::get_doccount(QuartzTable & table)
{   
    DEBUGCALL_STATIC(DB, om_doccount, "QuartzRecordManager::get_doccount", "[table]");
    // Check that we can't overflow (the unsigned test is actually too
    // strict as we can typically assign an unsigned short to a signed long,
    // but this shouldn't actually matter here).
    CASSERT(sizeof(om_doccount) >= sizeof(quartz_tablesize_t));
    CASSERT((om_doccount)(-1) > 0);
    CASSERT((quartz_tablesize_t)(-1) > 0);
    om_doccount entries = table.get_entry_count();
    RETURN(entries ? entries - 1 : 0);
}

om_docid
QuartzRecordManager::get_newdocid(QuartzBufferedTable & table)
{
    DEBUGCALL_STATIC(DB, om_docid, "QuartzRecordManager::get_newdocid", "[table]");
    string * tag = table.get_or_make_tag(METAINFO_KEY);

    om_docid did;
    quartz_totlen_t totlen;
    if (tag->empty()) {
	did = 1u;
	totlen = 0u;
    } else {
	const char * data = tag->data();
	const char * end = data + tag->size();
	if (!unpack_uint(&data, end, &did)) {
	    throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
	}
	if (!unpack_uint_last(&data, end, &totlen)) {
	    throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
	}
	++did;
	if (did == 0) {
	    throw Xapian::RangeError("Next document number is out of range.");
	}
    }
    *tag = pack_uint(did);
    *tag += pack_uint_last(totlen);

    RETURN(did);
}

om_docid
QuartzRecordManager::add_record(QuartzBufferedTable & table,
				const string & data)
{
    DEBUGCALL_STATIC(DB, om_docid, "QuartzRecordManager::add_record", "[table], " << data);
    om_docid did = get_newdocid(table);

    string key(quartz_docid_to_key(did));
    string * tag = table.get_or_make_tag(key);
    *tag = data;

    RETURN(did);
}

void
QuartzRecordManager::replace_record(QuartzBufferedTable & table,
				    const string & data,
				    om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::replace_record", "[table], " << data << ", " << did);
    string key(quartz_docid_to_key(did));
    string * tag = table.get_or_make_tag(key);
    *tag = data;
}

void
QuartzRecordManager::modify_total_length(QuartzBufferedTable & table,
					 quartz_doclen_t old_doclen,
					 quartz_doclen_t new_doclen)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::modify_total_length", "[table], " << old_doclen << ", " << new_doclen);
    string * tag = table.get_or_make_tag(METAINFO_KEY);

    om_docid did;
    quartz_totlen_t totlen;
    if (tag->empty()) {
	did = 1u;
	totlen = 0u;
    } else {
	const char * data = tag->data();
	const char * end = data + tag->size();
	if (!unpack_uint(&data, end, &did)) {
	    throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
	}
	if (!unpack_uint_last(&data, end, &totlen)) {
	    throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
	}
    }
    
    if (totlen < old_doclen)
	throw Xapian::DatabaseCorruptError("Total document length is less than claimed old document length");

    totlen -= old_doclen;
    quartz_totlen_t newlen = totlen + new_doclen;

    if (newlen < totlen)
	throw Xapian::RangeError("New total document length is out of range.");

    *tag = pack_uint(did);
    *tag += pack_uint_last(newlen);
}

// FIXME: probably want to cache the average length (but not miss updates)
om_doclength
QuartzRecordManager::get_avlength(QuartzTable & table)
{
    DEBUGCALL_STATIC(DB, om_doclength, "QuartzRecordManager::get_avlength", "QuartzTable &");
    om_doccount docs = get_doccount(table);
    if (docs == 0) RETURN(0);

    string tag;
    if (!table.get_exact_entry(METAINFO_KEY, tag)) RETURN(0u);

    om_docid did;
    quartz_totlen_t totlen;
    const char * data = tag.data();
    const char * end = data + tag.size();
    if (!unpack_uint(&data, end, &did)) {
	throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
    }
    if (!unpack_uint_last(&data, end, &totlen)) {
	throw Xapian::DatabaseCorruptError("Record containing meta information is corrupt.");
    }
    RETURN((double)totlen / docs);
}

void
QuartzRecordManager::delete_record(QuartzBufferedTable & table,
				   om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzRecordManager::delete_record", "[table], " << did);
    table.delete_tag(quartz_docid_to_key(did));
}
