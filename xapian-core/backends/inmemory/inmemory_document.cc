/* inmemory_document.cc: C++ class for storing inmemory documents
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include "inmemory_document.h"
#include <om/omdocument.h>

InMemoryDocument::InMemoryDocument(const string & doc_,
				   const map<om_keyno, OmKey> &keys_)
	: doc(doc_), keys(keys_)
{
    return;
}

OmKey
InMemoryDocument::do_get_key(om_keyno keyid) const
{
    map<om_keyno, OmKey>::const_iterator k = keys.find(keyid);
    if (k != keys.end()) {
	return k->second;
    } else {
	return OmKey();
    }
}

map<om_keyno, OmKey>
InMemoryDocument::do_get_all_keys() const
{
    return keys;
}

OmData
InMemoryDocument::do_get_data() const
{
    OmData data;
    data.value = doc;
    return data;
}
