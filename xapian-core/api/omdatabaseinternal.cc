/* omdatabaseinternal.cc
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
#include "utils.h"
#include "omdatabaseinternal.h"
#include "alltermslist.h"
#include "emptyalltermslist.h"
#include "multialltermslist.h"

#include "../backends/multi/multi_postlist.h"
#include "../backends/multi/multi_termlist.h"

#include "omdebug.h"
#include "om/omoutput.h"
#include <vector>

/////////////////////////////////////
// Methods of OmDatabase::Internal //
/////////////////////////////////////

OmDatabase::Internal::Internal(const OmSettings &params, bool readonly)
{
    add_database(params, readonly);
}

void
OmDatabase::Internal::add_database(const OmSettings & params, bool readonly)
{
    // Open database (readonly) and add it to the list
    RefCntPtr<Database> newdb(DatabaseBuilder::create(params, readonly));
    databases.push_back(newdb);
}

void
OmDatabase::Internal::add_database(const OmSettings & params)
{
    add_database(params, true);
}

void
OmDatabase::Internal::add_database(RefCntPtr<Database> newdb)
{
    databases.push_back(newdb);
}

om_doclength
OmDatabase::Internal::get_avlength() const
{
    om_doccount docs = 0;
    om_doclength totlen = 0;

    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); i++) {
	om_doccount db_doccount = (*i)->get_doccount();
	docs += db_doccount;
	totlen += (*i)->get_avlength() * db_doccount;
    }
    if (docs == 0) return 0.0;

    return totlen / docs;
}

LeafPostList *
OmDatabase::Internal::open_post_list(const om_termname & tname,
				     const OmDatabase &db) const
{
    // Don't bother checking that the term exists first.  If it does, we
    // just end up doing more work, and if it doesn't, we save very little
    // work.
    std::vector<LeafPostList *> pls;
    try {
	std::vector<RefCntPtr<Database> >::const_iterator i;
	for (i = databases.begin(); i != databases.end(); i++) {
	    pls.push_back((*i)->open_post_list(tname));
	    pls.back()->next();
	}
	Assert(pls.begin() != pls.end());
    } catch (...) {
	std::vector<LeafPostList *>::iterator i;
	for (i = pls.begin(); i != pls.end(); i++) {
	    delete *i;
	    *i = 0;
	}
	throw;
    }

    return new MultiPostList(pls, db);
}

LeafTermList *
OmDatabase::Internal::open_term_list(om_docid did, const OmDatabase &db) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    TermList *newtl = databases[dbnumber]->open_term_list(realdid);
    return new MultiTermList(newtl, databases[dbnumber], db);
}

Document *
OmDatabase::Internal::open_document(om_docid did) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return databases[dbnumber]->open_document(realdid);
}

AutoPtr<PositionList>
OmDatabase::Internal::open_position_list(om_docid did,
					 const om_termname &tname) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    // FIXME: databases should return these positionlists as AutoPtrs
    return AutoPtr<PositionList>(databases[dbnumber]->open_position_list(realdid, tname));
}

TermList *
OmDatabase::Internal::open_allterms(const OmDatabase &db) const
{
    if (databases.empty()) return new EmptyAllTermsList();
    
    std::vector<TermList *> lists;

    std::vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); ++i) {
	lists.push_back((*i)->open_allterms());
    }

    if (lists.size() == 1) return lists[0];

    return new MultiAllTermsList(lists);
}
