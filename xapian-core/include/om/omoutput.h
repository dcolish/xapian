/* omoutput.h: Functions for outputting strings describing OM objects.
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

#ifndef OM_HGUARD_OMOUTPUT_H
#define OM_HGUARD_OMOUTPUT_H

#include <iostream>

#define OUTPUT_FUNCTION(a) \
inline std::ostream & \
operator<<(std::ostream & os, const a & obj) { \
    return os << obj.get_description(); \
}

#include "omdatabase.h"
OUTPUT_FUNCTION(OmDatabase)
OUTPUT_FUNCTION(OmWritableDatabase)

#include "omdocument.h"
OUTPUT_FUNCTION(OmData)
OUTPUT_FUNCTION(OmKey)
OUTPUT_FUNCTION(OmDocument)

#include "omenquire.h"
OUTPUT_FUNCTION(OmQuery)
OUTPUT_FUNCTION(OmRSet)
OUTPUT_FUNCTION(OmMSetItem)
OUTPUT_FUNCTION(OmMSet)
OUTPUT_FUNCTION(OmESetItem)
OUTPUT_FUNCTION(OmESet)
OUTPUT_FUNCTION(OmEnquire)
OUTPUT_FUNCTION(OmBatchEnquire)

#include "omstem.h"
OUTPUT_FUNCTION(OmStem)

#include "omsettings.h"
OUTPUT_FUNCTION(OmSettings)

#include "ompostlistiterator.h"
OUTPUT_FUNCTION(OmPostListIterator)

#include "ompositionlistiterator.h"
OUTPUT_FUNCTION(OmPositionListIterator)

#include "omtermlistiterator.h"
OUTPUT_FUNCTION(OmTermListIterator)

#include "omkeylistiterator.h"
OUTPUT_FUNCTION(OmKeyListIterator)

#include "omindexermessage.h"
OUTPUT_FUNCTION(OmIndexerData)

inline std::ostream &
operator<<(std::ostream & os, const om_termname_list & obj) {
    os << "om_termname_list(";
    copy(obj.begin(), obj.end(),
	 std::ostream_iterator<om_termname>(os, ", "));
    os << ")";
    return os;
}

#endif /* OM_HGUARD_OMOUTPUT_H */
