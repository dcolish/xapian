/* ompositionlistiteratorinternal.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_OMPOSITIONLISTITERATORINTERNAL_H
#define OM_HGUARD_OMPOSITIONLISTITERATORINTERNAL_H

#include "om/ompositionlistiterator.h"
#include "positionlist.h"
#include "refcnt.h"
#include "autoptr.h"

class OmPositionListIterator::Internal {
    private:
	friend class OmPositionListIterator; // allow access to positionlist
        friend bool operator==(const OmPositionListIterator &a, const OmPositionListIterator &b);

	RefCntPtr<PositionList> positionlist;
    
    public:
        Internal(AutoPtr<PositionList> positionlist_)
		: positionlist(positionlist_.get())
	{
	    // A PositionList starts before the start, iterators start at the start
	    positionlist_.release();
	    positionlist->next();
	}

	Internal(const Internal &other)
		: positionlist(other.positionlist)
	{ }
};

#endif /* OM_HGUARD_OMPOSITIONLISTITERATOR_H */
