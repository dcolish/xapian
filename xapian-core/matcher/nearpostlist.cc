/* nearpostlist.cc: Return only items where the terms are near each other
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

#include "nearpostlist.h"

NearPostList::NearPostList(PostList *left, PostList *right, LocalMatch *matcher_,
			   om_termpos max_separation_,
			   bool order_matters_) : AndPostList(left, right, matcher)
{
    max_separation = max_separation_;
    order_matters = order_matters_;
}

PostList *
NearPostList::next(om_weight w_min)
{
    while (1) {
        AndPostList::next(w_min);
        if (at_end()) return NULL;
        // check NEAR criterion and return NULL iff satisfied
    }
}

PostList *
NearPostList::skip_to(om_docid did, om_weight w_min)
{
    if (did > get_docid()) {
        while (1) {
	   AndPostList::skip_to(did, w_min);
	   if (at_end()) return NULL;
	   // check NEAR criterion and return NULL iff satisfied
	}
    }
    return NULL;
}
