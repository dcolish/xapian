/* networkmatch.cc
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "networkmatch.h"

#include "stats.h"
#include "utils.h"

#include "msetpostlist.h"
#include "networkstats.h"
#include "net_database.h"

RemoteSubMatch::RemoteSubMatch(NetworkDatabase *db_,
			       const Xapian::Query::Internal * query,
			       Xapian::termcount qlen,
			       const Xapian::RSet & omrset,
			       Xapian::valueno collapse_key,
			       Xapian::Enquire::docid_order order,
			       Xapian::valueno sort_key,
			       Xapian::Enquire::Internal::sort_setting sort_by,
			       bool sort_value_forward,
			       int percent_cutoff, Xapian::weight weight_cutoff,
			       StatsGatherer *gatherer_,
			       const Xapian::Weight *wtscheme)
	: is_prepared(false), db(db_), gatherer(gatherer_)
{
    DEBUGCALL(MATCH, void, "RemoteSubMatch", db_ << ", " << query << ", " <<
	      qlen << ", " << omrset << ", " << collapse_key << ", " <<
	      int(order) << ", " << sort_key << ", " <<
	      int(sort_by) << ", " << sort_value_forward << ", " <<
	      percent_cutoff << ", " << weight_cutoff << ", " << gatherer_);
    Assert(db);
    Assert(query);
    Assert(gatherer_);
    statssource = new NetworkStatsSource(gatherer_, db);

    db->set_query(query, qlen, collapse_key, order,
			sort_key, sort_by, sort_value_forward,
			percent_cutoff, weight_cutoff, wtscheme, omrset);
    db->register_statssource(statssource);

    AutoPtr<RSetI> new_rset(new RSetI(db, omrset));
    rset = new_rset;
}

RemoteSubMatch::~RemoteSubMatch()
{
    DEBUGCALL(MATCH, void, "~RemoteSubMatch", "");
    db->close_end_time();
    delete statssource;
}

PostList *
RemoteSubMatch::get_postlist(Xapian::doccount maxitems, MultiMatch *matcher)
{
    DEBUGCALL(MATCH, PostList *, "RemoteSubMatch::get_postlist", maxitems << ", " << matcher);
    (void)matcher;
    postlist = new PendingMSetPostList(db, maxitems);
    RETURN(postlist);
}

bool
RemoteSubMatch::prepare_match(bool nowait)
{
    DEBUGCALL(MATCH, bool, "RemoteSubMatch::prepare_match", nowait);
    if (!is_prepared) {
	bool finished_query = db->finish_query();

	if (!finished_query) {
	    if (nowait) {
		RETURN(false);
	    } else {
		do {
		    db->wait_for_input();
		} while (!db->finish_query());
	    }
	}

	// Read the remote statistics and give them to the stats source
	//
	Stats mystats;
	bool read_remote_stats = db->get_remote_stats(mystats);
	if (!read_remote_stats) {
	    if (nowait) RETURN(false);
	    do {
		db->wait_for_input();
	    } while (!db->get_remote_stats(mystats));
	}
	statssource->take_remote_stats(mystats);

	is_prepared = true;
    }
    RETURN(true);
}

void
RemoteSubMatch::start_match(Xapian::doccount maxitems)
{
    DEBUGCALL(MATCH, void, "RemoteSubMatch::start_match", maxitems);
    Assert(is_prepared);
    db->send_global_stats(*(gatherer->get_stats()));
    Xapian::MSet mset;
    bool res = db->get_mset(0, maxitems, mset);
    (void)res;
    // FIXME: improve this
    // db->get_mset() should always return false for the first call.
    Assert(res == false);
}

const std::map<string, Xapian::MSet::Internal::TermFreqAndWeight>
RemoteSubMatch::get_term_info() const
{
    Assert(postlist);
    postlist->make_pl();
    return postlist->pl->mset.internal->termfreqandwts;
}
