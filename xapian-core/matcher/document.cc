/* document.cc: class with document data
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

#include <om/omtypes.h>
#include "refcnt.h"
#include "omlocks.h"
#include "document.h"
#include <om/omdocument.h>

OmKey
Document::get_key(om_keyno keyid) const
{
    OmLockSentry locksentry(mutex);
    return do_get_key(keyid);
}

std::map<om_keyno, OmKey>
Document::get_all_keys() const
{
    OmLockSentry locksentry(mutex);
    return do_get_all_keys();
}

OmData
Document::get_data() const
{
    OmLockSentry locksentry(mutex);
    return do_get_data();
}

LeafTermList *
Document::open_term_list() const
{
    OmLockSentry locksentry(mutex);
    return database->open_term_list(did);
}
