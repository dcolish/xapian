/* ompositionlistiterator.cc
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

#include "om/ompositionlistiterator.h"
#include "ompositionlistiteratorinternal.h"
#include "positionlist.h"
#include "omdebug.h"

OmPositionListIterator::~OmPositionListIterator()
{
    DEBUGAPICALL(void, "OmPositionListIterator::~OmPositionListIterator", "");
}

om_termpos
OmPositionListIterator::operator *() const
{
    DEBUGAPICALL(om_termpos, "OmPositionListIterator::operator*", "");
    Assert(internal);
    RETURN(internal->positionlist->get_position());
}

OmPositionListIterator &
OmPositionListIterator::operator++()
{
    DEBUGAPICALL(OmPositionListIterator &, "OmPositionListIterator::operator++", "");
    Assert(internal);
    internal->positionlist->next();
    if (internal->positionlist->at_end()) {
	delete internal;
	internal = 0;
    }
    RETURN(*this);
}

void
OmPositionListIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmPositionListIterator::operator++(int)", "");
    Assert(internal);
    internal->positionlist->next();
    if (internal->positionlist->at_end()) {
	delete internal;
	internal = 0;
    }
}

// extra method, not required to be an input_iterator
void
OmPositionListIterator::skip_to(om_termpos pos)
{
    DEBUGAPICALL(void, "OmPositionListIterator::skip_to", pos);
    Assert(internal);
    internal->positionlist->skip_to(pos);
    if (internal->positionlist->at_end()) {
	delete internal;
	internal = 0;
    }
}    

std::string
OmPositionListIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmPositionListIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmPositionListIterator()");
}

bool
operator==(const OmPositionListIterator &a, const OmPositionListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal == 0 || b.internal == 0) return false;
    return (a.internal->positionlist == b.internal->positionlist);
}
