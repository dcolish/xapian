/* expand.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <config.h>
#include "expand.h"
#include "rset.h"
#include "ortermlist.h"
#include "omdebug.h"

#include <algorithm>
#include "autoptr.h"

#include "omenquireinternal.h"

using std::nth_element;
using std::priority_queue;
using std::sort;
using std::vector;

class OmESetCmp {
    public:
        bool operator()(const Xapian::Internal::ESetItem &a, const Xapian::Internal::ESetItem &b) {
	    if (a.wt > b.wt) return true;
	    if (a.wt != b.wt) return false;
	    return a.tname > b.tname;
        }
};

class TLPCmpGt {
    public:
	bool operator()(const TermList *a, const TermList *b) {
	    return a->get_approx_size() > b->get_approx_size();
	}
};

AutoPtr<TermList>
OmExpand::build_tree(const RSetI *rset, const OmExpandWeight *ewt)
{
    // Put items in priority queue, such that items with greatest size
    // are returned first.
    // This is the same idea as for a set of postlists ORed together in
    // the matcher.
    //
    // FIXME: try using a heap instead (C++ sect 18.8)?
    priority_queue<TermList*, vector<TermList*>, TLPCmpGt> pq;
    try {
	set<Xapian::docid>::const_iterator i;
	for (i = rset->documents.begin(); i != rset->documents.end(); ++i) {
	    unsigned int multiplier = db.internal.size();
	    om_docid realdid = (*i - 1) / multiplier + 1;
	    om_doccount dbnumber = (*i - 1) % multiplier;

	    AutoPtr<LeafTermList> tl(db.internal[dbnumber]->open_term_list(realdid));
	    tl->set_weighting(ewt);
	    pq.push(tl.get());
	    tl.release();
	}

	if (pq.empty()) {
	    return AutoPtr<TermList>(0);
	}

	// Build a tree balanced by the term frequencies
	// (similar to building a huffman encoding tree).
	//
	// This scheme reduces the number of objects terms from large docs
	// get "pulled" through, reducing the amount of work done which
	// speeds things up.
	while (true) {
	    AutoPtr<TermList> tl(pq.top());
	    pq.pop();

	    DEBUGLINE(EXPAND,
		      "OmExpand: adding termlist " << tl.get() << " to tree");
	    if (pq.empty()) {
		return tl;
	    }

	    // NB right is always <= left - we can use this to optimise
	    AutoPtr<OrTermList> newtl(new OrTermList(pq.top(), tl.get()));
	    tl.release();
	    pq.pop();
	    pq.push(newtl.get());
	    newtl.release();
	}
    } catch (...) {
	while (!pq.empty()) {
	    delete pq.top();
	    pq.pop();
	}
	throw;
    }
}

void
OmExpand::expand(om_termcount max_esize,
		 Xapian::ESet & eset,
		 const RSetI * rset,
		 const Xapian::ExpandDecider * decider,
		 bool use_exact_termfreq,
		 double expand_k )
{
    DEBUGCALL(MATCH, void, "OmExpand::expand", max_esize << ", " << eset << ", " << rset << ", " << decider << ", " << use_exact_termfreq << ", " << expand_k);
    eset.internal->items.clear();
    eset.internal->ebound = 0;

    DEBUGLINE(EXPAND, "OmExpand::expand()");
    if (rset->get_rsize() == 0 || max_esize == 0)
	return; // No possibility of results
    DEBUGLINE(EXPAND, "OmExpand::expand() 2");

    om_weight w_min = 0;

    // Start weighting scheme
    OmExpandWeight ewt(db, rset->get_rsize(), 
		       use_exact_termfreq,
		       expand_k );

    AutoPtr<TermList> merger(build_tree(rset, &ewt));
    if (merger.get() == 0) return;

    DEBUGLINE(EXPAND, "ewt.get_maxweight() = " << ewt.get_maxweight());
    while (1) {
	{
	    TermList *ret = merger->next();
	    if (ret) {
		DEBUGLINE(EXPAND, "*** REPLACING ROOT");
		AutoPtr<TermList> newmerger(ret);
		merger = newmerger;
	    }
	}

	if (merger->at_end()) break;

	string tname = merger->get_termname();
	if ((*decider)(tname)) {
	    eset.internal->ebound++;

	    OmExpandBits ebits = merger->get_weighting();
	    om_weight wt = ewt.get_weight(ebits, tname);

	    if (wt > w_min) {
		eset.internal->items.push_back(Xapian::Internal::ESetItem(wt, tname));

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better w_min optimisations
		// Or perhaps better, switch to using minheap like matcher now
		// uses.
		if (eset.internal->items.size() == max_esize * 2) {
		    // find last element we care about
		    DEBUGLINE(EXPAND, "finding nth");
		    nth_element(eset.internal->items.begin(),
				eset.internal->items.begin() + max_esize - 1,
				eset.internal->items.end(),
				OmESetCmp());
		    // erase elements which don't make the grade
		    eset.internal->items.erase(eset.internal->items.begin() + max_esize,
				     eset.internal->items.end());
		    w_min = eset.internal->items.back().wt;
		    DEBUGLINE(EXPAND, "eset size = " << eset.internal->items.size());
		}
	    }
	}
    }

    if (eset.internal->items.size() > max_esize) {
	// find last element we care about
	DEBUGLINE(EXPAND, "finding nth");
	nth_element(eset.internal->items.begin(),
		    eset.internal->items.begin() + max_esize - 1,
		    eset.internal->items.end(), OmESetCmp());
	// erase elements which don't make the grade
	eset.internal->items.erase(eset.internal->items.begin() + max_esize, eset.internal->items.end());
    }
    DEBUGLINE(EXPAND, "sorting");

    // Need a stable sort, but this is provided by comparison operator
    sort(eset.internal->items.begin(), eset.internal->items.end(), OmESetCmp());

    DEBUGLINE(EXPAND, "esize = " << eset.internal->items.size() << ", ebound = " << eset.internal->ebound);
    if (eset.internal->items.size()) {
	DEBUGLINE(EXPAND, "max weight in eset = " << eset.internal->items.front().wt
		 << ", min weight in eset = " << eset.internal->items.back().wt);
    }
}
