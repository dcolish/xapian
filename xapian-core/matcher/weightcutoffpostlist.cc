/* weightcutoffpostlist.cc: Part of a postlist with a score greater than cutoff
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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
#include "weightcutoffpostlist.h"
#include "branchpostlist.h"

WeightCutoffPostList::WeightCutoffPostList(PostList * pl_,
					   om_weight cutoff_,
					   MultiMatch * matcher_)
	: pl(pl_), cutoff(cutoff_), matcher(matcher_)
{
    DEBUGCALL(MATCH, void, "WeightCutoffPostList", pl_ << ", " << cutoff_ << ", " << matcher_);
}

PostList *
WeightCutoffPostList::next(om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "WeightCutoffPostList::next", w_min);
    if (w_min < cutoff) w_min = cutoff;
    do {
	(void) next_handling_prune(pl, w_min, matcher);
    } while ((!pl->at_end()) && pl->get_weight() < w_min);
    RETURN(NULL);
}

PostList *
WeightCutoffPostList::skip_to(om_docid did, om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "WeightCutoffPostList::skip_to", did << ", " << w_min);
    if (w_min < cutoff) w_min = cutoff;
    do {
	// skip to guarantees skipping to at least docid did, but not that
	// it moves forward past did, or that the weight is taken into account.
	(void) skip_to_handling_prune(pl, did, w_min, matcher);

	if (pl->at_end()) RETURN(NULL);
	did += 1; // To ensure that skipping further ahead happens.
    } while (pl->get_weight() < w_min);

    RETURN(NULL);
}

