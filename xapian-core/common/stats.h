/* stats.h: Handling of statistics needed for the search.
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

#ifndef OM_HGUARD_STATS_H
#define OM_HGUARD_STATS_H

#include "om/omtypes.h"
#include "omdebug.h"
#include "refcnt.h"
#include <map>
#include <vector>

/** Class to hold statistics for a given collection. */
class Stats {
    public:
	/** Number of documents in the collection. */
	om_doccount collection_size;

	/** Number of relevant documents in the collection. */
	om_doccount rset_size;

	/** Average length of documents in the collection. */
	om_doclength average_length;

	/** Map of term frequencies for the collection. */
	std::map<om_termname, om_doccount> termfreq;

	/** Map of relevant term frequencies for the collection. */
	std::map<om_termname, om_doccount> reltermfreq;


	Stats() : collection_size(0),
		  rset_size(0),
		  average_length(1.0)
	{}

	/** Add in the supplied statistics from a sub-database.
	 */
	Stats & operator +=(const Stats & inc);
};

class StatsSource;

/** Statistics collector: gathers statistics from sub-databases, puts them
 *  together to build statistics for whole collection, and returns the
 *  overall statistics to each sub-database.
 */
class StatsGatherer {
    protected:
	/** Flag to say that statistics have been gathered.
	 *  Some entries in stats are only valid after this.
	 *  Stats should also not be modified before this.
	 */
	mutable bool have_gathered;

	/** A collection of StatsSource children.
	 *  The Gatherer uses this to make sure that each child
	 *  has contributed before it will give out its statistics.
	 */
	mutable std::vector<StatsSource *> sources;

	/** The total statistics gathered for the whole collection.
	 */
	mutable Stats total_stats;
    public:
	StatsGatherer();
	virtual ~StatsGatherer() {};

	/** Set the global collection statistics.
	 *  Should be called before the match is performed.
	 */
	virtual void set_global_stats(om_doccount rset_size);

	/** Add a StatsSource object to this gatherer.
	 *  The gatherer will include the source's statistics
	 *  into its own summary statistics.
	 */
	void add_child(StatsSource *source);

	/** Contribute some statistics to the overall statistics.
	 *  Should only be called once by each sub-database.
	 */
	void contrib_stats(const Stats & extra_stats);

	/** Get the collected statistics for the whole collection.
	 */
	virtual const Stats * get_stats() const = 0;
};

/** The top-level StatsGatherer.
 */
class LocalStatsGatherer : public StatsGatherer {
    public:
	const Stats *get_stats() const;
};

/** Statistics source: gathers notifications of statistics which will be
 *  needed, and passes them on in bulk to a StatsGatherer.
 */
class StatsSource {
    protected:
	/** The gatherer which we report our information to, and ask
	 *  for the global information from.
	 */
	StatsGatherer * gatherer;

	/** The stats to give to the StatsGatherer.
	 */
	Stats my_stats;

	/** The collection statistics, held by the StatsGatherer.
	 *  0 before these have been retrieved.
	 */
	mutable const Stats * total_stats;

	/** Perform the request for the needed information.  This involves
	 *  passing our information to the gatherer, and then getting the
	 *  result back.
	 */
	void perform_request() const;
    public:
	/// Constructor
	StatsSource(StatsGatherer *gatherer_);

	/// Virtual destructor
	virtual ~StatsSource() {};

	/// Contribute all the statistics that don't depend on global
	/// stats.  Used by StatsGatherer.
	virtual void contrib_my_stats() = 0;

	/* ################################################################
	 * # Give statistics about yourself.  These are used to generate, #
	 * # or check, the global information.                            #
	 * ################################################################
	 */

	/** Set stats about this sub-database: the number of documents and
	 *  average length of a document.
	 */
	void take_my_stats(om_doccount csize, om_doclength avlen);

	/** Set the term frequency in the sub-database which this stats
	 *  object represents.  This is the number of documents in
	 *  the sub-database indexed by the given term.
	 */
	void my_termfreq_is(const om_termname & tname, om_doccount tfreq);

	/** Set the relevant term-frequency in the sub-database which this
	 *  stats object represents.  This is the number of relevant
	 *  documents in the sub-database indexed by the given term.
	 */
	void my_reltermfreq_is(const om_termname & tname, om_doccount rtfreq);



	/* #################################################################
	 * # Get the statistics back.  The result of each of the following #
	 * # methods may be an approximation.                              #
	 * #################################################################
	 */

	/** Get the number of documents in the whole collection.
	 */
	om_doccount get_total_collection_size() const;

	/** Get the number of documents marked relevant in the collection.
	 */
	om_doccount get_total_rset_size() const;

	/** Get the average length of documents in the collection.
	 */
	om_doclength get_total_average_length() const;

	/** Get the term frequency over the whole collection, for the
	 *  given term.  This is "n_t", the number of documents in the
	 *  collection indexed by the given term.
	 */
	om_doccount get_total_termfreq(const om_termname & tname) const;

	/** Get the relevant term-frequency over the whole collection, for
	 *  the given term.  This is "r_t", the number of relevant documents
	 *  in the collection indexed by the given term.
	 */
	om_doccount get_total_reltermfreq(const om_termname & tname) const;
};

/** LocalStatsSource: the StatsSource object which provides methods
 *  to access the statistics.  A LocalSubMatch object uses it to report
 *  on its local statistics and retrieve the global statistics after
 *  the gathering process is complete.
 */
class LocalStatsSource : public StatsSource {
    private:
    public:
	/// Constructor
	LocalStatsSource(StatsGatherer * gatherer_);

	/// Destructor
	~LocalStatsSource();

	/// Contribute all the statistics that don't depend on global
	/// stats.
	void contrib_my_stats();
};

/////////////////////////////////////////
// Inline method definitions for Stats //
/////////////////////////////////////////

inline Stats &
Stats::operator +=(const Stats & inc)
{
    // Set the new collection size and average length.
    om_doccount new_collection_size = collection_size + inc.collection_size;
    if(new_collection_size != 0) {
	// Cope with adding in a collection of zero size at the beginning:
	// perhaps we have multiple databases, but some are not yet populated
	average_length = (average_length * collection_size +
			  inc.average_length * inc.collection_size) /
			 new_collection_size;
    }
    collection_size = new_collection_size;

    // rset_size is set at the top level, we don't want
    // to pass it back up again.
    Assert(inc.rset_size == 0);

    // Add termfreqs and reltermfreqs
    std::map<om_termname, om_doccount>::const_iterator i;
    for(i = inc.termfreq.begin(); i != inc.termfreq.end(); i++) {
	termfreq[i->first] += i->second;
    }
    for(i = inc.reltermfreq.begin(); i != inc.reltermfreq.end(); i++) {
	reltermfreq[i->first] += i->second;
    }
    return *this;
}

/////////////////////////////////////////////////
// Inline method definitions for StatsGatherer //
/////////////////////////////////////////////////

inline
StatsGatherer::StatsGatherer()
	: have_gathered(false)
{}

inline void
StatsGatherer::set_global_stats(om_doccount rset_size)
{
    total_stats.rset_size = rset_size;
}

///////////////////////////////////////////////
// Inline method definitions for StatsSource //
///////////////////////////////////////////////


    
inline
StatsSource::StatsSource(StatsGatherer *gatherer_)
	: gatherer(gatherer_), total_stats(0)
{
    gatherer->add_child(this);
}

inline void
LocalStatsSource::contrib_my_stats()
{
    gatherer->contrib_stats(my_stats);
}

inline void
StatsSource::take_my_stats(om_doccount csize, om_doclength avlen)
{
    Assert(total_stats == 0);
    my_stats.collection_size = csize;
    my_stats.average_length = avlen;
}

inline void
StatsSource::my_termfreq_is(const om_termname & tname, om_doccount tfreq)
{
    Assert(total_stats == 0);
    // Can be called a second time, if a term occurs multiple times in the
    // query.
    Assert(my_stats.termfreq.find(tname) == my_stats.termfreq.end() ||
	   my_stats.termfreq.find(tname)->second == tfreq);
    my_stats.termfreq[tname] = tfreq;
}

inline void
StatsSource::my_reltermfreq_is(const om_termname & tname, om_doccount rtfreq)
{
    Assert(total_stats == 0);
    // Can be called a second time, if a term occurs multiple times in the
    // query.
    Assert(my_stats.reltermfreq.find(tname) == my_stats.reltermfreq.end() ||
	   my_stats.reltermfreq.find(tname)->second == rtfreq);
    my_stats.reltermfreq[tname] = rtfreq;
}

inline om_doccount
StatsSource::get_total_collection_size() const
{
    if(total_stats == 0) perform_request();
    return total_stats->collection_size;
}

inline om_doccount
StatsSource::get_total_rset_size() const
{
    if(total_stats == 0) perform_request();
    return total_stats->rset_size;
}

inline om_doclength
StatsSource::get_total_average_length() const
{
    if(total_stats == 0) perform_request();
    return total_stats->average_length;
}

inline om_doccount
StatsSource::get_total_termfreq(const om_termname & tname) const
{
    if(total_stats == 0) perform_request();

    // To get the statistics about a given term, we have to have
    // supplied our own ones first.
    Assert(my_stats.termfreq.find(tname) != my_stats.termfreq.end());

    std::map<om_termname, om_doccount>::const_iterator tfreq;
    tfreq = total_stats->termfreq.find(tname);
    Assert(tfreq != total_stats->termfreq.end());
    return tfreq->second;
}

inline om_doccount
StatsSource::get_total_reltermfreq(const om_termname & tname) const
{
    if(total_stats == 0) perform_request();
    std::map<om_termname, om_doccount>::const_iterator rtfreq;
    rtfreq = total_stats->reltermfreq.find(tname);
    Assert(rtfreq != total_stats->reltermfreq.end());
    return rtfreq->second;
}

#endif /* OM_HGUARD_STATS_H */
