/* rset.cc
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

#include "rset.h"
#include "termlist.h"
#include "stats.h"
#include "omdebug.h"

#include <memory>

void
RSet::calculate_stats()
{
    Assert(!calculated_reltermfreqs);
    DEBUGLINE(WTCALC, "RSet::calculate_stats(): ");
    std::vector<RSetItem>::const_iterator doc;
    for (doc = documents.begin();
	 doc != documents.end();
	 doc++) {
	DEBUGLINE(WTCALC, "document " << doc->did << " [ ");
	std::auto_ptr<TermList> tl =
	    std::auto_ptr<TermList>(root->open_term_list(doc->did));
	tl->next();
	while(!(tl->at_end())) {
	    // FIXME - can this lookup be done faster?
	    // Store termnames in a hash for each document, rather than
	    // a list?
	    om_termname tname = tl->get_termname();
	    DEBUGLINE(WTCALC, tname << ", ");
	    if(reltermfreqs.find(tname) != reltermfreqs.end())
		reltermfreqs[tname] ++;
	    tl->next();
	}
	DEBUGLINE(WTCALC, "] ");
    }
    DEBUGLINE(WTCALC, "done");
    calculated_reltermfreqs = true;
}

void
RSet::give_stats_to_statssource(StatsSource &statssource)
{
    Assert(calculated_reltermfreqs);

    std::map<om_termname, om_doccount>::const_iterator reltermfreq;
    for (reltermfreq = reltermfreqs.begin();
	 reltermfreq != reltermfreqs.end();
	 reltermfreq++) {
	statssource.my_reltermfreq_is(reltermfreq->first,
				      reltermfreq->second);
    }
}
