/* expand.cc
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

#include "expand.h"
#include "rset.h"
#include "ortermlist.h"
#include "omdebug.h"

#include <algorithm>
#include "om/autoptr.h"

#include "omdatabaseinterface.h"
#include "../api/omdatabaseinternal.h"

class OmESetCmp {
    public:
        bool operator()(const OmESetItem &a, const OmESetItem &b) {
	    if(a.wt > b.wt) return true;
	    if(a.wt != b.wt) return false;
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
OmExpand::build_tree(const RSet *rset, const OmExpandWeight *ewt)
{
    OmDatabase::Internal * internal = OmDatabase::InternalInterface::get(db);
    // Put items in priority queue, such that items with greatest size
    // are returned first.
    // This is the same idea as for a set of postlists ORed together in
    // the matcher.
    //
    // FIXME: try using a heap instead (C++ sect 18.8)?
    std::priority_queue<TermList*, std::vector<TermList*>, TLPCmpGt> pq;
    try {
	std::vector<RSetItem>::const_iterator i;
	for (i = rset->documents.begin();
	     i != rset->documents.end();
	     i++) {
	    unsigned int multiplier = internal->databases.size();
	    om_docid realdid = ((*i).did - 1) / multiplier + 1;
	    om_doccount dbnumber = ((*i).did - 1) % multiplier;

	    AutoPtr<LeafTermList> tl(internal->databases[dbnumber]->open_term_list(realdid));
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
    } catch(...) {
	while(!pq.empty()) {
	    delete pq.top();
	    pq.pop();
	}
	throw;
    }
}

void
OmExpand::expand(om_termcount max_esize,
		 OmESet & eset,
		 const RSet * rset,
		 const OmExpandDecider * decider,
		 bool use_exact_termfreq)
{
    eset.items.clear();
    eset.ebound = 0;

    DEBUGLINE(EXPAND, "OmExpand::expand()");
    if (rset->get_rsize() == 0 || max_esize == 0)
	return; // No possibility of results
    DEBUGLINE(EXPAND, "OmExpand::expand() 2");

    om_weight w_min = 0;

    // Start weighting scheme
    OmExpandWeight ewt(db, rset->get_rsize(), use_exact_termfreq);

    AutoPtr<TermList> merger(build_tree(rset, &ewt));
    if(merger.get() == 0) return;

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

	om_termname tname = merger->get_termname();
	if((*decider)(tname)) {
	    eset.ebound++;

	    OmExpandBits ebits = merger->get_weighting();
	    om_weight wt = ewt.get_weight(ebits, tname);

	    if (wt > w_min) {
		eset.items.push_back(OmESetItem(wt, tname));

		// FIXME: find balance between larger size for more efficient
		// nth_element and smaller size for better w_min optimisations
		if (eset.items.size() == max_esize * 2) {
		    // find last element we care about
		    DEBUGLINE(EXPAND, "finding nth");
		    std::nth_element(eset.items.begin(),
				eset.items.begin() + max_esize - 1,
				eset.items.end(),
				OmESetCmp());
		    // erase elements which don't make the grade
		    eset.items.erase(eset.items.begin() + max_esize,
				     eset.items.end());
		    w_min = eset.items.back().wt;
		    DEBUGLINE(EXPAND, "eset size = " << eset.items.size());
		}
	    }
	}
    }

    if (eset.items.size() > max_esize) {
	// find last element we care about
	DEBUGLINE(EXPAND, "finding nth");
	std::nth_element(eset.items.begin(),
		    eset.items.begin() + max_esize - 1,
		    eset.items.end(), OmESetCmp());
	// erase elements which don't make the grade
	eset.items.erase(eset.items.begin() + max_esize, eset.items.end());
    }
    DEBUGLINE(EXPAND, "sorting");

    // Need a stable sort, but this is provided by comparison operator
    std::sort(eset.items.begin(), eset.items.end(), OmESetCmp());

    DEBUGLINE(EXPAND, "esize = " << eset.items.size() << ", ebound = " << eset.ebound);
    if (eset.items.size()) {
	DEBUGLINE(EXPAND, "max weight in eset = " << eset.items.front().wt
		 << ", min weight in eset = " << eset.items.back().wt);
    }
}
