/* match.h: class for performing a match
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

#ifndef _match_h_
#define _match_h_

#include "database.h"
#include "omassert.h"
#include "omenquire.h"

class IRWeight;

#include <stack>
#include <vector>

////////////////////////////////////////////////////////////////////////////
// Comparison functions to determine the order of elements in the MSet
// Return true if a should be listed before b
// (By default, equally weighted items will be returned in reverse
// document id number.)

typedef bool (* mset_cmp)(const OMMSetItem &, const OMMSetItem &);
bool msetcmp_forward(const OMMSetItem &, const OMMSetItem &);
bool msetcmp_reverse(const OMMSetItem &, const OMMSetItem &);

// Class which encapsulates best match operation
class OMMatch
{
    private:
        IRDatabase *database;

        int min_weight_percent;
        weight max_weight;

	stack<PostList *> query;
	vector<IRWeight *> weights;

	RSet *rset;         // RSet to be used (affects weightings)

	bool do_collapse;   // Whether to perform collapsem operation
	keyno collapse_key; // Key to collapse on, if desired

	bool have_added_terms;
        bool recalculate_maxweight;

	bool query_ready; 
	PostList * build_query();

	// Make a postlist from a query object
	PostList * postlist_from_query(const OMQuery *);

	// Make a postlist from a vector of query objects (AND or OR)
	PostList * postlist_from_queries(om_queryop, const vector<OMQuery *> &);

	// Open a postlist
	DBPostList * mk_postlist(const termname& tname,
				 RSet * rset);
    public:
        OMMatch(IRDatabase *);
        ~OMMatch();

	///////////////////////////////////////////////////////////////////
	// Set the terms and operations which comprise the query
	// =====================================================

	// Sets query to use.
	void set_query(const OMQuery *);

	///////////////////////////////////////////////////////////////////
	// Set additional options for performing the query
	// ===============================================

	// Set relevance information - the RSet object should not be
	// altered after this call
        void set_rset(RSet *);

	// Set cutoff at min percentage - defaults to -1, which means no cutoff
        void set_min_weight_percent(int);

	// Add a key number to collapse by.  Each key value will appear only
	// once in the result set.  Collapsing can only be done on one key
	// number.
	void set_collapse_key(keyno);

	// Remove the collapse key
	void set_no_collapse();

	///////////////////////////////////////////////////////////////////
	// Get information about result
	// ============================

	// Get an upper bound on the possible weights (unlikely to be attained)
        weight get_max_weight();

	// Perform the match operation, and get the matching items.
	// Also returns a lower bound on the number of matching records in
	// the database (mbound).  Because of some of the optimisations
	// performed, this is likely to be much lower than the actual
	// number of matching records, but it is expensive to do the
	// exact calculation.
	//
	// It is generally considered that presenting the mbound to users
	// causes them to worry about the large number of results, rather
	// than how useful those at the top of the mset are, and is thus
	// undesirable.
	void match(doccount first,         // First item to return (start at 0)
		   doccount maxitems,      // Maximum number of items to return
		   vector<OMMSetItem> &,   // Results will be put in this vector
		   mset_cmp,               // Comparison operator to sort by
		   doccount *);            // Mbound will returned here

	///////////////////////////////////////////////////////////////////
	// Miscellaneous
	// =============

	// Called by postlists to indicate that they've rearranged themselves
	// and the maxweight now possible is smaller.
        void recalc_maxweight();
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline void
OMMatch::set_collapse_key(keyno key)
{
    do_collapse = true;
    collapse_key = key;
}

inline void
OMMatch::set_no_collapse()
{
    do_collapse = false;
}

inline void
OMMatch::set_rset(RSet *new_rset)
{
    Assert(!have_added_terms);
    query_ready = false;
    rset = new_rset;
}

inline void
OMMatch::set_min_weight_percent(int pcent)
{
    min_weight_percent = pcent;
}

inline weight
OMMatch::get_max_weight()
{
    (void) build_query();
    return max_weight;
}

#endif /* _match_h_ */
