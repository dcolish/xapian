/* multimatch.h: class for performing a match
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

#ifndef OM_HGUARD_MULTIMATCH_H
#define OM_HGUARD_MULTIMATCH_H

#include "match.h"
#include "stats.h"
#include "refcnt.h"

#include "omqueryinternal.h"

#include <vector>
#include <map>
#include "om/autoptr.h"  // auto_ptr

class SubMatch;
class OmErrorHandler;

class SocketServer;

class MultiMatch
{
    friend class SocketServer; // FIXME: temporary bodge
    private:
	/// Vector of the items
	std::vector<RefCntPtr<SubMatch> > leaves;

	/// stats gatherer
	AutoPtr<StatsGatherer> gatherer;

	const OmDatabase db;

	OmSettings opts;

	/// Comparison functor for sorting MSet
	OmMSetCmp mcmp;

	/** Internal flag to note that w_max needs to be recalculated
	 *  while query is running.
	 */
        bool recalculate_w_max;

	/** Prepare all the sub matchers.
	 *
	 *  This calls prepare_match() on all the sub matchers until they
	 *  all return true to signal that they are prepared.
	 */
	void prepare_matchers();

	/// get the collapse key
	OmKey get_collapse_key(PostList *pl, const OmDatabase &db,
			       om_docid did, om_keyno keyno,
			       RefCntPtr<LeafDocument> &doc);

	/// Copying is not permitted.
	MultiMatch(const MultiMatch &);

	/// Assignment is not permitted.
	void operator=(const MultiMatch &);

    public:
	/** MultiMatch constructor.
	 *
	 *  @param db_       The database to use.
	 *
	 *  @param gatherer_ An auto_ptr to a StatsGatherer instance.
	 *                   The MultiMatch takes ownership of the
	 *                   StatsGatherer.  This argument is optional;
	 *                   the default is to use a LocalStatsGatherer,
	 *                   suitable for non-network use.
	 */
	MultiMatch(const OmDatabase &db_,
		   const OmQueryInternal * query,
		   const OmRSet & omrset,
		   const OmSettings & opts_,
		   AutoPtr<StatsGatherer> gatherer_
		       = AutoPtr<StatsGatherer>(new LocalStatsGatherer()));
	~MultiMatch();

	void get_mset(om_doccount first,
		      om_doccount maxitems,
		      OmMSet & mset,
		      const OmMatchDecider * mdecider,
		      OmErrorHandler * errorhandler);

	// FIXME: temporary bodge
	PostList *get_postlist(om_doccount first, om_doccount maxitems,
			       std::map<om_termname, OmMSet::TermFreqAndWeight> & termfreqandwts,
			       OmErrorHandler * errorhandler);
	    
	/** Called by postlists to indicate that they've rearranged themselves
	 *  and the maxweight now possible is smaller.
	 */
        void recalc_maxweight();
};

#endif /* OM_HGUARD_MULTIMATCH_H */
