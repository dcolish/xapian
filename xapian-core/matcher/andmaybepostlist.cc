/* andmaybepostlist.cc: Merged postlist; items from one list, weights from both
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
#include "andmaybepostlist.h"
#include "andpostlist.h"
#include "omdebug.h"

inline PostList *
AndMaybePostList::process_next_or_skip_to(om_weight w_min, PostList *ret)
{
    DEBUGCALL(MATCH, PostList *, "AndMaybePostList::process_next_or_skip_to", w_min << ", " << ret);
    handle_prune(l, ret);
    if (l->at_end()) {
	// once l is over, so is the AND MAYBE
	lhead = 0;
	RETURN(NULL);
    }

    lhead = l->get_docid();
    if (lhead <= rhead) RETURN(NULL);

    skip_to_handling_prune(r, lhead, w_min - lmax, matcher);
    if (r->at_end()) {
	PostList *ret = l;
	l = NULL;
	RETURN(ret);
    }
    rhead = r->get_docid();
    RETURN(NULL);
}

PostList *
AndMaybePostList::next(om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "AndMaybePostList::next", w_min);
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	DEBUGLINE(MATCH, "AND MAYBE -> AND");
	ret = new AndPostList(l, r, matcher, dbsize, true);
	l = r = NULL;
	skip_to_handling_prune(ret, std::max(lhead, rhead) + 1, w_min, matcher);
	RETURN(ret);
    }
    RETURN(process_next_or_skip_to(w_min, l->next(w_min - rmax)));
}

PostList *
AndMaybePostList::skip_to(om_docid did, om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "AndMaybePostList::skip_to", did << ", " << w_min);
    if (w_min > lmax) {
	// we can replace the AND MAYBE with an AND
	PostList *ret;
	DEBUGLINE(MATCH, "AND MAYBE -> AND (in skip_to)");
	ret = new AndPostList(l, r, matcher, dbsize, true);
	did = std::max(did, std::max(lhead, rhead));
	l = r = NULL;
	skip_to_handling_prune(ret, did, w_min, matcher);
	RETURN(ret);
    }

    // exit if we're already past the skip point (or at it)
    if (did <= lhead) RETURN(NULL);

    RETURN(process_next_or_skip_to(w_min, l->skip_to(did, w_min - rmax)));
}
