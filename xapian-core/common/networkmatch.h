/* networkmatch.h: class for communicating with remote match processes
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

#ifndef OM_HGUARD_NETWORKMATCH_H
#define OM_HGUARD_NETWORKMATCH_H

#include "match.h"
#include "stats.h"
#include "net_database.h"

#include "../matcher/msetpostlist.h"

/// Class for performing match calculations remotely
class RemoteSubMatch : public SubMatch {
    private:
	bool is_prepared;

	/// The size of the query (passed to IRWeight objects)
	om_doclength querysize;
    
	const NetworkDatabase *db;

	PendingMSetPostList *postlist; // FIXME remove this crap

	/// RSet to be used (affects weightings)
	AutoPtr<RSet> rset;
    
	/// A pointer to the gatherer, to access the statistics.
	StatsGatherer *gatherer;

	/// the statistics object
	Stats remote_stats;


	// disallow copies
	RemoteSubMatch(const RemoteSubMatch &);
	void operator=(const RemoteSubMatch &);

	/// Prepare the stats object with contributed
	/// statistics from the remote end.
	void finish_query();

	/// Make a weight - default argument is used for finding extra_weight
	IRWeight * mk_weight(const OmQueryInternal *query = NULL);

    public:
	RemoteSubMatch(const Database *db_, const OmQueryInternal * query,
		       const OmRSet & omrset, const OmSettings &mopts,
		       StatsGatherer *gatherer_);

	~RemoteSubMatch();

	/// Calculate the statistics for the query
	bool prepare_match(bool nowait);

	PostList * get_postlist(om_doccount maxitems, MultiMatch *matcher);

	virtual LeafDocument * open_document(om_docid did) const {
	    return db->open_document(did);
	}

	const std::map<om_termname, OmMSet::TermFreqAndWeight> get_term_info() const {
	    Assert(postlist);
	    postlist->make_pl();
	    return postlist->pl->mset.get_all_terminfo();
	}
};   

#endif /* OM_HGUARD_NETWORKMATCH_H */
