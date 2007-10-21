/** @file queryoptimiser.cc
 * @brief Convert a Xapian::Query::Internal tree into an optimal PostList tree.
 */
/* Copyright (C) 2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "queryoptimiser.h"

#include "andmaybepostlist.h"
#include "andnotpostlist.h"
#include "autoptr.h"
#include "emptypostlist.h"
#include "exactphrasepostlist.h"
#include "multiandpostlist.h"
#include "multimatch.h"
#include "omassert.h"
#include "omdebug.h"
#include "omqueryinternal.h"
#include "orpostlist.h"
#include "phrasepostlist.h"
#include "postlist.h"
#include "stats.h"
#include "valuerangepostlist.h"
#include "xorpostlist.h"

#include <algorithm>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <vector>

using namespace std;

PostList *
QueryOptimiser::do_subquery(const Xapian::Query::Internal * query, double factor)
{
    DEBUGCALL(MATCH, PostList *, "QueryOptimiser::do_subquery",
	      query << ", " << factor);

    // Handle QueryMatchNothing.
    if (!query) RETURN(new EmptyPostList());

    switch (query->op) {
	case Xapian::Query::Internal::OP_LEAF:
	    RETURN(do_leaf(query, factor));

	case Xapian::Query::OP_AND:
	case Xapian::Query::OP_FILTER:
	case Xapian::Query::OP_NEAR:
	case Xapian::Query::OP_PHRASE:
	    RETURN(do_and_like(query, factor));

	case Xapian::Query::OP_OR:
	case Xapian::Query::OP_XOR:
	case Xapian::Query::OP_ELITE_SET:
	    RETURN(do_or_like(query, factor));

	case Xapian::Query::OP_AND_NOT: {
	    AssertEq(query->subqs.size(), 2);
	    PostList * l = do_subquery(query->subqs[0], factor);
	    PostList * r = do_subquery(query->subqs[1], 0.0);
	    RETURN(new AndNotPostList(l, r, matcher, db_size));
	}

	case Xapian::Query::OP_AND_MAYBE: {
	    AssertEq(query->subqs.size(), 2);
	    PostList * l = do_subquery(query->subqs[0], factor);
	    PostList * r = do_subquery(query->subqs[1], factor);
	    RETURN(new AndMaybePostList(l, r, matcher, db_size));
	}

	case Xapian::Query::OP_VALUE_RANGE: {
	    Xapian::valueno valno(query->parameter);
	    const string & range_begin = query->tname;
	    const string & range_end = query->str_parameter;
	    RETURN(new ValueRangePostList(&db, valno, range_begin, range_end));
	}

	case Xapian::Query::OP_SCALE_WEIGHT: {
	    AssertEq(query->subqs.size(), 1);
	    double sub_factor = factor * query->dbl_parameter;
	    RETURN(do_subquery(query->subqs[0], sub_factor));
	}

	default:
	    Assert(false);
	    RETURN(NULL);
    }
}

struct PosFilter {
    PosFilter(Xapian::Query::Internal::op_t op_, size_t begin_, size_t end_,
	      Xapian::termcount window_)
	: op(op_), begin(begin_), end(end_), window(window_) { }

    Xapian::Query::Internal::op_t op;

    /// Start and end indices for the PostLists this positional filter uses.
    size_t begin, end;

    Xapian::termcount window;
};

PostList *
QueryOptimiser::do_and_like(const Xapian::Query::Internal *query, double factor)
{
    DEBUGCALL(MATCH, PostList *, "QueryOptimiser::do_and_like",
	      query << ", " << factor);

    list<PosFilter> pos_filters;
    vector<PostList *> plists;
    do_and_like(query, factor, plists, pos_filters);
    AssertRel(plists.size(), >=, 2);

    PostList * pl;
    pl = new MultiAndPostList(plists.begin(), plists.end(), matcher, db_size);

    // Sort the positional filters to try to apply them in an efficient order.
    // FIXME: We need to figure out what that is!  Try applying lowest cf/tf
    // first?

    // Apply any positional filters.
    list<PosFilter>::iterator i;
    for (i = pos_filters.begin(); i != pos_filters.end(); ++i) {
	const PosFilter & filter = *i;

	// FIXME: make NearPostList, etc ctors take a pair of itors so we don't
	// need to create this temporary vector.
	vector<PostList *> terms(plists.begin() + filter.begin,
				 plists.begin() + filter.end);

	Xapian::termcount window = filter.window;
	if (filter.op == Xapian::Query::OP_NEAR) {
	    pl = new NearPostList(pl, window, terms);
	} else if (window == filter.end - filter.begin) {
	    AssertEq(filter.op, Xapian::Query::OP_PHRASE);
	    pl = new ExactPhrasePostList(pl, terms);
	} else {
	    AssertEq(filter.op, Xapian::Query::OP_PHRASE);
	    pl = new PhrasePostList(pl, window, terms);
	}
    }

    RETURN(pl);
}

inline bool is_and_like(Xapian::Query::Internal::op_t op) {
    return op == Xapian::Query::OP_AND || op == Xapian::Query::OP_FILTER ||
	   op == Xapian::Query::OP_NEAR || op == Xapian::Query::OP_PHRASE;
}

void
QueryOptimiser::do_and_like(const Xapian::Query::Internal *query, double factor,
			    vector<PostList *> & and_plists,
			    list<PosFilter> & pos_filters)
{
    DEBUGCALL(MATCH, PostList *,
	      "QueryOptimiser::do_and_like", query << ", " << factor <<
	      ", [and_plists]");

    Xapian::Query::Internal::op_t op = query->op;
    Assert(is_and_like(op));

    bool positional = false;
    if (op == Xapian::Query::OP_PHRASE || op == Xapian::Query::OP_NEAR) {
	// If this sub-database has no positional information, change
	// OP_PHRASE/OP_NEAR into OP_AND so that we actually return some
	// matches.
	if (!db.has_positions()) {
	    op = Xapian::Query::OP_AND;
	} else {
	    positional = true;
	}
    }

    const Xapian::Query::Internal::subquery_list &queries = query->subqs;
    AssertRel(queries.size(), >=, 2);

    Xapian::Query::Internal::subquery_list::const_iterator q;
    for (size_t i = 0; i != queries.size(); ++i) {
	// The second branch of OP_FILTER is always boolean.
	if (i == 1 && op == Xapian::Query::OP_FILTER) factor = 0.0;

	const Xapian::Query::Internal * subq = queries[i];
	if (is_and_like(subq->op)) {
	    do_and_like(subq, factor, and_plists, pos_filters);
	} else {
	    PostList * pl = do_subquery(subq, factor);
	    and_plists.push_back(pl);
	}
    }

    if (positional) {
	// Record the positional filter to apply higher up the tree.
	size_t end = and_plists.size();
	size_t begin = end - queries.size();
	Xapian::termcount window = query->parameter;

	pos_filters.push_back(PosFilter(op, begin, end, window));
    }
}

/// Comparison functor which orders PostList* by descending get_termfreq_est().
/** Class providing an operator which sorts postlists to select max or terms.
 *  This returns true if a has a (strictly) greater termweight than b,
 *  unless a or b contain no documents, in which case the other one is
 *  selected.
 */
struct CmpMaxOrTerms {
    /** Return true if and only if a has a strictly greater termweight
     *  than b; with the proviso that if the termfrequency
     *  of the a or b is 0, the termweight is considered to be 0.
     *
     *  We use termfreq_max() because we really don't want to exclude a
     *  postlist which has a low but non-zero termfrequency: the estimate
     *  is quite likely to be zero in this case.
     */
    bool operator()(const PostList *a, const PostList *b) {
	if (a->get_termfreq_max() == 0) return false;
	if (b->get_termfreq_max() == 0) return true;
	return a->get_maxweight() > b->get_maxweight();
    }
};

template<class CLASS> void delete_ptr(CLASS *p) { delete p; }

/// Comparison functor which orders PostList* by descending get_termfreq_est().
struct ComparePostListTermFreqAscending {
    /// Order by descending get_termfreq_est().
    bool operator()(const PostList *a, const PostList *b) {
	return a->get_termfreq_est() > b->get_termfreq_est();
    }
};

PostList *
QueryOptimiser::do_or_like(const Xapian::Query::Internal *query, double factor)
{
    // FIXME: we could optimise by merging OP_ELITE_SET and OP_OR like we do
    // for AND-like operations.
    Xapian::Query::Internal::op_t op = query->op;
    Assert(op == Xapian::Query::OP_ELITE_SET || op == Xapian::Query::OP_OR ||
	   op == Xapian::Query::OP_XOR);

    const Xapian::Query::Internal::subquery_list &queries = query->subqs;
    AssertRel(queries.size(), >=, 2);

    vector<PostList *> postlists;
    Xapian::Query::Internal::subquery_list::const_iterator q;
    for (q = queries.begin(); q != queries.end(); ++q) {
	postlists.push_back(do_subquery(*q, factor));
    }

    if (op == Xapian::Query::OP_ELITE_SET) {
	// Select the best elite_set_size terms.
	Xapian::termcount elite_set_size = query->parameter;
	Assert(elite_set_size > 0);

	if (postlists.size() > elite_set_size) {
	    // Call recalc_maxweight() as otherwise get_maxweight()
	    // may not be valid before next() or skip_to()
	    for_each(postlists.begin(), postlists.end(),
		     mem_fun(&PostList::recalc_maxweight));

	    nth_element(postlists.begin(),
			postlists.begin() + elite_set_size - 1,
			postlists.end(), CmpMaxOrTerms());

	    for_each(postlists.begin() + elite_set_size, postlists.end(),
		     delete_ptr<PostList>);

	    if (elite_set_size == 1) RETURN(postlists[0]);

	    postlists.resize(elite_set_size);
	}
    }

    // Make postlists into a heap so that the postlist with the greatest term
    // frequency is at the top of the heap.
    make_heap(postlists.begin(), postlists.end(),
	      ComparePostListTermFreqAscending());

    // Now build a tree of binary OrPostList or XorPostList objects.  The
    // algorithm used to build the tree is like that used to build an
    // optimal Huffman coding tree.  If we called next() repeatedly, this
    // arrangement would minimise the number of method calls.  Generally we
    // don't actually do that, but this arrangement is still likely to be a
    // good one, and it does minimise the work in the worst case.
    AssertRel(postlists.size(), >=, 2);
    while (true) {
	// We build the tree such that at each branch:
	//
	//   l.get_termfreq_est() >= r.get_termfreq_est()
	//
	// We do this so that the OrPostList and XorPostList classes can be
	// optimised assuming that this is the case.
	PostList * r = postlists.front();
	pop_heap(postlists.begin(), postlists.end(),
		 ComparePostListTermFreqAscending());
	postlists.pop_back();
	PostList * l = postlists.front();

	PostList * pl;
	if (op == Xapian::Query::OP_XOR) {
	    pl = new XorPostList(l, r, matcher, db_size);
	} else {
	    pl = new OrPostList(l, r, matcher, db_size);
	}

	if (postlists.size() == 1) RETURN(pl);

	pop_heap(postlists.begin(), postlists.end(),
		 ComparePostListTermFreqAscending());
	postlists.back() = pl;
	push_heap(postlists.begin(), postlists.end(),
		  ComparePostListTermFreqAscending());
    }
}
