/* localmatch.cc
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

#include "config.h"
#include "localmatch.h"
#include "omdebug.h"

#include "andpostlist.h"
#include "orpostlist.h"
#include "xorpostlist.h"
#include "andnotpostlist.h"
#include "andmaybepostlist.h"
#include "filterpostlist.h"
#include "phrasepostlist.h"
#include "emptypostlist.h"
#include "leafpostlist.h"
#include "mergepostlist.h"
#include "extraweightpostlist.h"
#include "weightcutoffpostlist.h"

#include "document.h"
#include "rset.h"
#include "omqueryinternal.h"

#include "../api/omdatabaseinternal.h"

#include "match.h"
#include "stats.h"
#include "irweight.h"

#include <algorithm>
#include "autoptr.h"
#include <queue>

/////////////////////////////////////////////
// Comparison operators which we will need //
/////////////////////////////////////////////

/** Class providing an operator which returns true if a has a (strictly)
 *  greater number of postings than b.
 */
class PLPCmpGt {
    public:
	/** Return true if and only if a has a strictly greater number of
	 *  postings than b.
	 */
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq_est() > b->get_termfreq_est();
        }
};

/** Class providing an operator which returns true if a has a (strictly)
 *  smaller number of postings than b.
 */
class PLPCmpLt {
    public:
	/** Return true if and only if a has a strictly smaller number of
	 *  postings than b.
	 */
        bool operator()(const PostList *a, const PostList *b) {
            return a->get_termfreq_est() < b->get_termfreq_est();
        }
};

/** Class providing an operator which sorts postlists to select max or terms.
 *  This returns true if a has a (strictly) greater termweight than b,
 *  unless a or b contain no documents, in which case the other one is
 *  selected.
 */
class CmpMaxOrTerms {
    public:
	/** Return true if and only if a has a strictly greater termweight
	 *  than b; with the proviso that if the termfrequency
	 *  of the a or b is 0, the termweight is considered to be 0.
	 *
	 *  We use termfreq_max() because we really don't want to exclude a
	 *  postlist which has a low but non-zero termfrequency: the estimate
	 *  is quite likely to be zero in this case.
	 */
	bool operator()(const PostList *a, const PostList *b) {
	    om_weight amax, bmax;
	    if (a->get_termfreq_max() == 0)
		amax = 0;
	    else
		amax = a->get_maxweight();

	    if (b->get_termfreq_max() == 0)
		bmax = 0;
	    else
		bmax = b->get_maxweight();

	    DEBUGLINE(MATCH, "termweights are: " << amax << " and " << bmax);
	    return amax > bmax;
	}
};

// FIXME: postlists passed by reference and needs to be kept "nice"
// for NearPostList - so if there's a zero freq term it needs to be
// empty, else it needs to have all the postlists in (though the order
// can be different) - this is ultra-icky
PostList *
LocalSubMatch::build_xor_tree(std::vector<PostList *> &postlists,
			      MultiMatch *matcher)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::build_xor_tree", "[postlists], " << matcher);
    // Build nice tree for XOR-ed postlists
    // Want postlists with most entries to be at top of tree, to reduce
    // average number of nodes an entry gets "pulled" through.
    //
    // Put postlists into a priority queue, such that those with greatest
    // term frequency are returned first.
    // FIXME: not certain that this is the best way to organise an XOR tree

    std::priority_queue<PostList *, std::vector<PostList *>, PLPCmpGt> pq;
    std::vector<PostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	pq.push(*i);
    }
    postlists.clear();

    // If none of the postlists had any entries, return an EmptyPostList.
    if (pq.empty()) {
	EmptyPostList *pl = new EmptyPostList();
	return pl;
    }

    // Build a tree balanced by the term frequencies
    // (similar to building a huffman encoding tree).
    //
    // This scheme reduces the number of objects common terms
    // get "pulled" through, reducing the amount of work done which
    // speeds things up.
    while (true) {
	PostList *pl = pq.top();
	pq.pop();
	if (pq.empty()) return pl;
	// NB right is always <= left - we can use this to optimise
	pl = new XorPostList(pq.top(), pl, matcher, db->get_doccount());
	pq.pop();
	pq.push(pl);
    }
}

// FIXME: postlists passed by reference and needs to be kept "nice"
// for NearPostList - so if there's a zero freq term it needs to be
// empty, else it needs to have all the postlists in (though the order
// can be different) - this is ultra-icky
PostList *
LocalSubMatch::build_and_tree(std::vector<PostList *> &postlists,
			      MultiMatch *matcher)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::build_and_tree", "[postlists], " << matcher);
    // Build nice tree for AND-ed terms
    // SORT list into ascending freq order
    // AND last two elements, then AND with each subsequent element

// FIXME: if we throw away zero frequency postlists at this point,
// max_weight will come out lower...
#if 0
    std::vector<PostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	if ((*i)->get_termfreq_max() == 0) {
	    // a zero freq term => the AND has zero freq
	    for (i = postlists.begin(); i != postlists.end(); i++)
		delete *i;
	    postlists.clear();
	    break;
	}
    }
#endif

    if (postlists.empty()) {
	EmptyPostList *pl = new EmptyPostList();
	return pl;
    }

    std::stable_sort(postlists.begin(), postlists.end(), PLPCmpLt());

    int j = postlists.size() - 1;
    PostList *pl = postlists[j];
    while (j > 0) {
	j--;
	// NB right is always <= left - we use this to optimise.
	pl = new AndPostList(postlists[j], pl, matcher, db->get_doccount());
    }
    return pl;
}

PostList *
LocalSubMatch::build_or_tree(std::vector<PostList *> &postlists,
			     MultiMatch *matcher)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::build_or_tree", "[postlists], " << matcher);
    // Build nice tree for OR-ed postlists
    // Want postlists with most entries to be at top of tree, to reduce
    // average number of nodes an entry gets "pulled" through.
    //
    // Put postlists into a priority queue, such that those with greatest
    // term frequency are returned first.
    // FIXME: try using a heap instead (C++ sect 18.8)?

    std::priority_queue<PostList *, std::vector<PostList *>, PLPCmpGt> pq;
    std::vector<PostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
// FIXME: if we throw away zero frequency postlists at this point,
// max_weight will come out lower...
#if 0
	// for an OR, we can just ignore zero freq terms
	if ((*i)->get_termfreq_max() == 0) {
	    delete *i;
	} else {
	    pq.push(*i);
	}
#else
	pq.push(*i);
#endif
    }
    postlists.clear();

    // If none of the postlists had any entries, return an EmptyPostList.
    if (pq.empty()) {
	EmptyPostList *pl = new EmptyPostList();
	return pl;
    }

    // Build a tree balanced by the term frequencies
    // (similar to building a huffman encoding tree).
    //
    // This scheme reduces the number of objects common terms
    // get "pulled" through, reducing the amount of work done which
    // speeds things up.
    while (true) {
	PostList *pl = pq.top();
	pq.pop();
	if (pq.empty()) return pl;
	// NB right is always <= left - we can use this to optimise
	pl = new OrPostList(pq.top(), pl, matcher, db->get_doccount());
	pq.pop();
	pq.push(pl);
    }
}

// Make a postlist from a vector of query objects.
// Operation must be either AND or OR.
// Optimise query by building tree carefully.
PostList *
LocalSubMatch::postlist_from_queries(OmQuery::Internal::op_t op,
				const OmQuery::Internal::subquery_list &queries,
				om_termpos window,
				om_termcount elite_set_size,
				MultiMatch *matcher,
				bool is_bool)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::postlist_from_queries", op << ", [queries], " << window << ", " << elite_set_size << ", " << matcher << ", " << is_bool);
    Assert(op == OmQuery::OP_OR || op == OmQuery::OP_AND ||
	   op == OmQuery::OP_XOR ||
	   op == OmQuery::OP_NEAR || op == OmQuery::OP_PHRASE ||
	   op == OmQuery::OP_ELITE_SET);
    Assert(queries.size() >= 2);

    // Open a postlist for each query, and store these postlists in a vector.
    std::vector<PostList *> postlists;
    postlists.reserve(queries.size());

    OmQuery::Internal::subquery_list::const_iterator q;
    for (q = queries.begin(); q != queries.end(); q++) {
	postlists.push_back(postlist_from_query(*q, matcher, is_bool));
	DEBUGLINE(MATCH, "Made postlist for " << (*q)->get_description() <<
		  ": termfreq is: (min, est, max) = (" <<
		  postlists.back()->get_termfreq_min() << ", " <<
		  postlists.back()->get_termfreq_est() << ", " <<
		  postlists.back()->get_termfreq_max() << ")");
    }

    // Build tree
    switch (op) {
	case OmQuery::OP_XOR:
	    return build_xor_tree(postlists, matcher);

	case OmQuery::OP_AND:
	    return build_and_tree(postlists, matcher);

	case OmQuery::OP_OR:
	    return build_or_tree(postlists, matcher);

	case OmQuery::OP_NEAR:
	{
	    PostList *res = build_and_tree(postlists, matcher);
	    // FIXME: handle EmptyPostList return specially?
	    return new NearPostList(res, window, postlists);
	}

	case OmQuery::OP_PHRASE:
	{
	    // build_and_tree reorders postlists, but the order is
	    // important for phrase, so we need to keep a copy
	    // FIXME: there must be a cleaner way for this to work...
	    std::vector<PostList *> postlists_orig = postlists;
	    PostList *res = build_and_tree(postlists, matcher);
	    // FIXME: handle EmptyPostList return specially?
	    return new PhrasePostList(res, window, postlists_orig);
	}

	case OmQuery::OP_ELITE_SET:
	{
	    // Select top terms
	    DEBUGLINE(API, "Selecting top " << elite_set_size <<
		      " terms, out of " << postlists.size() << ".");

	    if (elite_set_size <= 0) {
		std::vector<PostList *>::iterator i;
		for (i = postlists.begin(); i != postlists.end(); i++)
		    delete *i;
		postlists.clear();
		return new EmptyPostList();
	    }

	    if (postlists.size() > elite_set_size) {
		// Call recalc_maxweight() as otherwise get_maxweight()
		// may not be valid before next() or skip_to()
		std::vector<PostList *>::iterator j;
		for (j = postlists.begin(); j != postlists.end(); j++)
		    (*j)->recalc_maxweight();
		std::nth_element(postlists.begin(),
				 postlists.begin() + elite_set_size - 1,
				 postlists.end(), CmpMaxOrTerms());
		DEBUGLINE(MATCH, "Discarding " <<
			  (postlists.size() - elite_set_size) <<
			  " terms.");

		std::vector<PostList *>::const_iterator i;
		for (i = postlists.begin() + elite_set_size;
		     i != postlists.end(); i++) {
		    delete *i;
		}
		postlists.erase(postlists.begin() + elite_set_size,
				postlists.end());
	    }

	    return build_or_tree(postlists, matcher);
	}

	default:
	    Assert(0);
    }
    Assert(0);
    return NULL;
}

// Make a postlist from a query object - this is called recursively down
// the query tree.
PostList *
LocalSubMatch::postlist_from_query(const OmQuery::Internal *query,
				   MultiMatch *matcher, bool is_bool)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::postlist_from_query", query << ", " << matcher << ", " << is_bool);
    switch (query->op) {
	case OmQuery::Internal::OP_UNDEF: {
	    LeafPostList *pl = new EmptyPostList();
	    pl->set_termweight(new BoolWeight(opts));
	    return pl;
	}

	case OmQuery::Internal::OP_LEAF: {
	    // Make a postlist for a single term
	    Assert(query->subqs.size() == 0);
	    OmMSet::Internal::Data::TermFreqAndWeight info;
	
	    // FIXME: pass the weight type and the info needed to create it to the
	    // postlist instead
	    IRWeight * wt;
	    if (is_bool) {
		wt = new BoolWeight(opts);
	    } else {
		wt = mk_weight(query);
	    }
	    info.termweight = wt->get_maxpart();

	    // MULTI - this statssource should be the combined one...
	    info.termfreq = statssource->get_total_termfreq(query->tname);

	    DEBUGLINE(MATCH, " weight = " << info.termweight <<
		      ", frequency = " << info.termfreq);

	    term_info.insert(std::make_pair(query->tname, info));

	    // MULTI
	    LeafPostList * pl = db->open_post_list(query->tname);
	    pl->set_termweight(wt);
	    return pl;
	}
	case OmQuery::OP_AND:
	case OmQuery::OP_OR:
	case OmQuery::OP_XOR:
	case OmQuery::OP_PHRASE:
	case OmQuery::OP_NEAR:
	case OmQuery::OP_ELITE_SET:
	    // Build a tree of postlists for AND, OR, XOR, PHRASE, NEAR, or
	    // ELITE_SET
	    return postlist_from_queries(query->op, query->subqs,
					 query->window, query->elite_set_size,
					 matcher, is_bool);
	case OmQuery::OP_FILTER:
	    Assert(query->subqs.size() == 2);
	    return new FilterPostList(postlist_from_query(query->subqs[0], matcher, is_bool),
				      postlist_from_query(query->subqs[1], matcher, true),
				      matcher,
				      db->get_doccount());
	case OmQuery::OP_AND_NOT:
	    Assert(query->subqs.size() == 2);
	    return new AndNotPostList(postlist_from_query(query->subqs[0], matcher, is_bool),
				      postlist_from_query(query->subqs[1], matcher, true),
				      matcher,
				      db->get_doccount());
	case OmQuery::OP_AND_MAYBE:
	    Assert(query->subqs.size() == 2);
	    return new AndMaybePostList(postlist_from_query(query->subqs[0], matcher, is_bool),
					postlist_from_query(query->subqs[1], matcher, is_bool),
					matcher,
					db->get_doccount());

	case OmQuery::OP_WEIGHT_CUTOFF:
	    Assert(query->subqs.size() == 1);
	    // FIXME: if is_bool, then do what?
	    return new WeightCutoffPostList(postlist_from_query(query->subqs[0], matcher, is_bool),
					    query->cutoff,
					    matcher);
	case OmQuery::OP_PERCENT_CUTOFF:
	    Assert(query->subqs.size() == 1);
	    throw OmUnimplementedError("Percentage cutoffs in query tree not yet implemented.");
    }
    Assert(false);
    return NULL;
}

////////////////////////
// Building the query //
////////////////////////

bool
LocalSubMatch::prepare_match(bool nowait)
{
    DEBUGCALL(MATCH, bool, "LocalSubMatch::prepare_match", nowait);
    if (!is_prepared) {
	DEBUGLINE(MATCH, "LocalSubMatch::prepare_match() - Gathering my statistics");
	OmTermIterator terms = users_query.get_terms();
	OmTermIterator terms_end(NULL);
	for ( ; terms != terms_end; terms++) {
	    // MULTI
	    register_term(*terms);
	    if (rset.get() != 0) rset->will_want_reltermfreq(*terms);
	}

	if (rset.get() != 0) {
	    rset->calculate_stats();
	    rset->give_stats_to_statssource(statssource.get());
	}
	is_prepared = true;
    }
    return true;
}

PostList *
LocalSubMatch::get_postlist(om_doccount maxitems, MultiMatch *matcher)
{
    DEBUGCALL(MATCH, PostList *, "LocalSubMatch::get_postlist", maxitems << ", " << matcher);
    PostList *pl = postlist_from_query(&users_query, matcher, false);
    IRWeight *wt = mk_weight();
    // don't bother with an ExtraWeightPostList if there's no extra weight
    // contribution.
    if (wt->get_maxextra() == 0) {
	delete wt;
	return pl;
    }
    return new ExtraWeightPostList(pl, wt, matcher);
}



IRWeight *
LocalSubMatch::mk_weight(const OmQuery::Internal *query_)
{
    DEBUGCALL(MATCH, IRWeight *, "LocalSubMatch::mk_weight", query_);
    om_termname tname = "";
    om_termcount wqf = 1;
    if (query_) {
	tname = query_->tname;
	wqf = query_->wqf;
    }
    IRWeight * wt = IRWeight::create_new(opts);
    wt->set_stats(statssource.get(), querysize, wqf, tname);
#ifdef MUS_DEBUG_PARANOID
    if (!tname.empty()) {
	AutoPtr<IRWeight> extra_weight(mk_weight());
	// Check that max_extra weight is really right
	AssertEqDouble(wt->get_maxextra(), extra_weight->get_maxextra());
    }
#endif /* MUS_DEBUG_PARANOID */
    return wt;
}
