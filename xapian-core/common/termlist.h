/* termlist.h
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

#ifndef OM_HGUARD_TERMLIST_H
#define OM_HGUARD_TERMLIST_H

#include <string>

#include "om/omtypes.h"
#include "om/omerror.h"
#include "refcnt.h"
#include "expandweight.h"

using namespace std;

class OmPositionListIterator;

/** Abstract base class for termlists. */
class TermList : public RefCntBase
{
    private:
	/// Copying is not allowed.
	TermList(const TermList &);

	/// Assignment is not allowed.
	void operator=(const TermList &);
    public:
	/// Standard constructor for base class.
	TermList() {}

	/// Standard destructor for base class.
	virtual ~TermList() {}

	// Gets size of termlist
	virtual om_termcount get_approx_size() const = 0;

	// Gets weighting info for current term
	virtual OmExpandBits get_weighting() const = 0;

	// Gets current termname
	virtual string get_termname() const = 0;

	// Get wdf of current term
	virtual om_termcount get_wdf() const = 0;

	// Get num of docs indexed by term
	virtual om_doccount get_termfreq() const = 0;

	// Get num of docs indexed by term
	virtual om_termcount get_collection_freq() const {
	    Assert(false);
	    return 0;
	}

	/** next() causes the TermList to move to the next term in the list.
	 *  It must be called before any other methods.
	 *  If next() returns a non-zero pointer P, then the original
	 *  termlist should be deleted, and the original pointer replaced
	 *  with P.
	 *  In LeafTermList, next() will always return NULL.
	 */
	virtual TermList * next() = 0;

        /** Skip to the given term.  If the term wasn't
	 *  found it will be positioned on the term just
	 *  after tname in the database.  This could be after the end!
	 */
	virtual TermList * skip_to(const string &tname) {
	    // naive implementation
	    TermList *p = this;
	    while (!p->at_end() && p->get_termname() < tname) {
		TermList *tmp = p->next();
		if (tmp) {
		    if (p != this) delete p;
		    p = tmp;
		}
	    }
	    if (p != this) return p;
	    return NULL;
	}

	// True if we're off the end of the list
	virtual bool at_end() const = 0;

	virtual OmPositionListIterator positionlist_begin() const {
	    throw OmInvalidOperationError("positionlist_begin not supported");
	}
};

/** Base class for termlists which are at the leaves of the termlist tree,
 *  and thus generate weighting information, rather than merging weighting
 *  information from sub termlists.
 */
class LeafTermList : public TermList
{
    private:
        // Prevent copying
        LeafTermList(const LeafTermList &);
        LeafTermList & operator=(const LeafTermList &);
    protected:
	const OmExpandWeight * wt;
    public:
	LeafTermList() : wt(NULL) {}
	~LeafTermList() {}

	// Sets term weight
	virtual void set_weighting(const OmExpandWeight * wt_) { wt = wt_; }
};

#endif /* OM_HGUARD_TERMLIST_H */
