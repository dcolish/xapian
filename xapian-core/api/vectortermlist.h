/* vectortermlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2005 Olly Betts
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

#ifndef OM_HGUARD_VECTORTERMLIST_H
#define OM_HGUARD_VECTORTERMLIST_H

#include <xapian/error.h>
#include "omassert.h"
#include "termlist.h"

#include <list>
#include <vector>

using namespace std;

class VectorTermList : public TermList {
    private:
	vector<string> terms;
	vector<string>::size_type offset;
	bool before_start;

    public:
	VectorTermList(vector<string>::const_iterator begin,
		       vector<string>::const_iterator end)
	    : terms(begin, end), offset(0), before_start(true)
	{
	}

	VectorTermList(list<string>::const_iterator begin,
		       list<string>::const_iterator end)
#ifdef __SUNPRO_CC
	    : offset(0), before_start(true)
	{
	    while (begin != end) terms.push_back(*begin++);
	}
#else
	    : terms(begin, end), offset(0), before_start(true)
	{
	}
#endif

	// Gets size of termlist
	Xapian::termcount get_approx_size() const {
	    return terms.size();
	}

	// Gets weighting info for current term
	OmExpandBits get_weighting() const {
	    Assert(false); // should never get called
            throw Xapian::InvalidOperationError("VectorTermList::get_weighting() not supported");
	}
	    
	// Gets current termname
	string get_termname() const {
	    Assert(!before_start && offset < terms.size());
	    return terms[offset];
	}

	// Get wdf of current term
	Xapian::termcount get_wdf() const {
	    Assert(!before_start && offset < terms.size());
	    return 1; // FIXME: or is Xapian::InvalidOperationError better?
	}

	// Get num of docs indexed by term
	Xapian::doccount get_termfreq() const {
            throw Xapian::InvalidOperationError("VectorTermList::get_termfreq() not supported");
	}

	/** next() causes the TermList to move to the next term in the list.
	 *  It must be called before any other methods.
	 *  If next() returns a non-zero pointer P, then the original
	 *  termlist should be deleted, and the original pointer replaced
	 *  with P.
	 *  In LeafTermList, next() will always return 0.
	 */
	TermList * next() {
	    Assert(!at_end());
	    if (before_start)
		before_start = false;
	    else
		offset++;
	    return NULL;
	}

	TermList *skip_to(const string &/*tname*/) {
	    Assert(!at_end());
	    // termlist not ordered
	    Assert(false);
            throw Xapian::InvalidOperationError("VectorTermList::skip_to() not supported");
	}
	
	// True if we're off the end of the list
	bool at_end() const {
	    return !before_start && offset == terms.size();
	}
};

#endif // OM_HGUARD_VECTORTERMLIST_H
