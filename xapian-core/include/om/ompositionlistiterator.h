/* ompositionlistiterator.h
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

#ifndef OM_HGUARD_OMPOSITIONLISTITERATOR_H
#define OM_HGUARD_OMPOSITIONLISTITERATOR_H

#include <iterator>
#include "omtypes.h"

class OmPostListIterator;
class OmTermListIterator;
class OmDatabase;

class OmPositionListIterator {
    private:
	friend class OmPostListIterator; // So OmPostListIterator can construct us
	friend class OmTermListIterator; // So OmTermListIterator can construct us
	friend class OmTermListIteratorMap;
	friend class OmDatabase; // So OmDatabase can construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmPositionListIterator &a, const OmPositionListIterator &b);

	OmPositionListIterator(Internal *internal_) {
	    internal = internal_;
	}

    public:
        ~OmPositionListIterator();

	OmPositionListIterator operator=(OmPositionListIterator &o);

	const om_termpos operator *();

	OmPositionListIterator & operator++();

	void operator++(int);

	// extra method, not required for an input_iterator
	void skip_to(om_termpos pos);

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	// Allow use as an STL iterator
	typedef std::input_iterator_tag iterator_category;
	typedef om_termpos value_type;
	typedef om_termpos difference_type;
	typedef om_termpos * pointer;
	typedef om_termpos & reference;
};

inline bool operator!=(const OmPositionListIterator &a,
		       const OmPositionListIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMPOSITIONLISTITERATOR_H */
