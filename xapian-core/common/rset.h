/* rset.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#ifndef OM_HGUARD_RSET_H
#define OM_HGUARD_RSET_H

#include <vector>
#include <map>
#include "omdebug.h"
#include "om/omenquire.h"
#include "omenquireinternal.h"

class Database;
class OmWeight::Internal;

class RSetItem {
    public:
	RSetItem(om_docid did_new) : did(did_new) { }
	om_docid did;
};

/** A relevance set.
 *
 * This is used internally, and performs the calculation and caching of
 * relevant term frequencies.
 */
class RSet {
    private:
	// disallow copy
	RSet(const RSet &);
	void operator=(const RSet &);

	// FIXME: should use one or the other (probably OmDatabase)
	const OmDatabase root;
	const Database *dbroot;

	std::map<string, om_doccount> reltermfreqs;
	bool calculated_reltermfreqs;
    public:
	std::vector<RSetItem> documents; // FIXME - should be encapsulated

	RSet(const OmDatabase &root_);
	RSet(const OmDatabase &root_, const OmRSet & omrset);
	RSet(const Database *dbroot_, const OmRSet & omrset);

	void add_document(om_docid did);
	void will_want_reltermfreq(string tname);

	void calculate_stats();
	void give_stats_to_statssource(OmWeight::Internal *statssource);

	om_doccount get_rsize() const;
	om_doccount get_reltermfreq(string tname) const;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

/// Empty initialisation
inline
RSet::RSet(const OmDatabase &root_)
	: root(root_), dbroot(NULL), calculated_reltermfreqs(false)
{}

/// Initialise with an OmRSet
inline
RSet::RSet(const OmDatabase &root_, const OmRSet & omrset)
	: root(root_), dbroot(NULL), calculated_reltermfreqs(false)
{
    std::set<om_docid>::const_iterator i;
    for (i = omrset.internal->items.begin();
	 i != omrset.internal->items.end(); i++) {
	add_document(*i);
    }
}

/// Initialise with an OmRSet
inline
RSet::RSet(const Database *dbroot_, const OmRSet & omrset)
	: dbroot(dbroot_), calculated_reltermfreqs(false)
{
    std::set<om_docid>::const_iterator i;
    for (i = omrset.internal->items.begin();
	 i != omrset.internal->items.end(); i++) {
	add_document(*i);
    }
}

inline void
RSet::add_document(om_docid did)
{
    // FIXME - check that document isn't already in rset
    Assert(!calculated_reltermfreqs);
    documents.push_back(RSetItem(did));
}

inline void
RSet::will_want_reltermfreq(string tname)
{
    reltermfreqs[tname] = 0;
}

inline om_doccount
RSet::get_rsize() const
{
    return documents.size();
}

inline om_doccount
RSet::get_reltermfreq(string tname) const
{
    Assert(calculated_reltermfreqs);

    std::map<string, om_doccount>::const_iterator rfreq;
    rfreq = reltermfreqs.find(tname);
    Assert(rfreq != reltermfreqs.end());

    return rfreq->second;
}

#endif /* OM_HGUARD_RSET_H */
