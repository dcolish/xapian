/* branchpostlist.h: virtual base class for branched types of postlist
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

#ifndef OM_HGUARD_BRANCHPOSTLIST_H
#define OM_HGUARD_BRANCHPOSTLIST_H

#include "multimatch.h"
#include "postlist.h"

/** Base class for postlists which are generated by merging two
 *  sub-postlists.
 *
 *  These postlists form a tree which is used to perform a sum over all the
 *  terms in the query for each document, in order to calculate the score
 *  for that document.
 */
class BranchPostList : public PostList {
    protected:
	/** Utility method, to call recalc_maxweight() and do the pruning
	 *  if a next() or skip_to() returns non-NULL result.
	 */
	void handle_prune(PostList *&kid, PostList *ret);

	/// Left sub-postlist
        PostList *l;

	/// Right sub-postlist
	PostList *r;

	/** The object which is using this postlist to perform
	 *  a match.  This object needs to be notified when the
	 *  tree changes such that the maximum weights need to be
	 *  recalculated.
	 */
        MultiMatch *matcher;
    public:
        virtual ~BranchPostList();

	/** Most branch postlists won't be able to supply position lists.
	 *  If get_position_list() is called on such a branch postlist,
	 *  an OmUnimplementedError exception will be thrown.
	 */
	virtual PositionList *get_position_list();
};

inline
BranchPostList::~BranchPostList()
{
    if (l) delete l;
    if (r) delete r;
}

inline void
BranchPostList::handle_prune(PostList *&kid, PostList *ret)
{
    if (ret) {
	delete kid;
	kid = ret;

	// now tell matcher that maximum weights need recalculation.
	matcher->recalc_maxweight();
    }
}

inline PositionList *
BranchPostList::get_position_list()
{
    throw OmUnimplementedError("BranchPostList::get_position_list() unimplemented");
}

#endif /* OM_HGUARD_BRANCHPOSTLIST_H */
