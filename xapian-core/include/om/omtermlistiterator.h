/* omtermlistiterator.h
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

#ifndef OM_HGUARD_OMTERMLISTITERATOR_H
#define OM_HGUARD_OMTERMLISTITERATOR_H

#include <iterator>
#include "omtypes.h"

class OmDatabase;
class OmPositionListIterator;

class OmTermListIterator {
    private:
	friend class OmDatabase; // So OmDatabase can construct us
	friend class OmDocument; // So OmDocument can construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmTermListIterator &a, const OmTermListIterator &b);

	OmTermListIterator(Internal *internal_);

    public:
        ~OmTermListIterator();

        /** Copying is allowed.  The internals are reference counted, so
	 *  copying is also cheap.
	 */
	OmTermListIterator(const OmTermListIterator &other);

        /** Assignment is allowed.  The internals are reference counted,
	 *  so assignment is also cheap.
	 */
	void operator=(const OmTermListIterator &other);

	om_termname operator *() const;

	OmTermListIterator & operator++();

	void operator++(int);

	// extra method, not required for an input_iterator
	void skip_to(const om_termname & tname);

	om_termcount get_wdf() const;
	om_doccount get_termfreq() const;

    	// allow iteration of positionlist for current document
	OmPositionListIterator positionlist_begin();
	OmPositionListIterator positionlist_end();
    
	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;

	/// Allow use as an STL iterator
	//@{
	typedef std::input_iterator_tag iterator_category;
	typedef om_termname value_type;
	typedef om_termcount difference_type;
	typedef om_termname * pointer;
	typedef om_termname & reference;
	//@}
};

inline bool operator!=(const OmTermListIterator &a,
		       const OmTermListIterator &b)
{
    return !(a == b);
}

#endif /* OM_HGUARD_OMTERMLISTITERATOR_H */
