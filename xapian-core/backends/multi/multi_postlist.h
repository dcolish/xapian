/* multi_postlist.h: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_MULTI_POSTLIST_H
#define OM_HGUARD_MULTI_POSTLIST_H

#include "utils.h"
#include "omdebug.h"
#include "leafpostlist.h"
#include <stdlib.h>
#include <vector>

class MultiPostList : public LeafPostList {
    friend class OmDatabase::Internal;
    private:
	std::vector<LeafPostList *> postlists;

	const OmDatabase &this_db;

	bool   finished;
	om_docid  currdoc;

	string tname;
	mutable bool freq_initialised;
	mutable om_doccount termfreq;

	mutable bool collfreq_initialised;
	mutable om_termcount collfreq;

	om_weight termweight;

	om_doccount multiplier;

	MultiPostList(std::vector<LeafPostList *> & pls,
		      const OmDatabase &this_db_);
    public:
	~MultiPostList();

	void set_termweight(const OmWeight * wt); // Sets term weight

	om_doccount get_termfreq() const;
	om_termcount get_collection_freq() const;

	om_docid  get_docid() const;     // Gets current docid
	om_doclength get_doclength() const; // Get length of current document
        om_termcount get_wdf() const;	    // Within Document Frequency
	PositionList *read_position_list();
	PositionList * open_position_list() const;
	PostList *next(om_weight w_min);          // Moves to next docid
	PostList *skip_to(om_docid did, om_weight w_min);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

inline void
MultiPostList::set_termweight(const OmWeight * wt)
{
    // Set in base class, so that get_maxweight() works
    LeafPostList::set_termweight(wt);
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	(*i)->set_termweight(wt);
    }
}

inline om_doccount
MultiPostList::get_termfreq() const
{
    if(freq_initialised) return termfreq;
    DEBUGLINE(DB, "Calculating multiple term frequencies");

    // Calculate and remember the termfreq
    termfreq = 0;
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	termfreq += (*i)->get_termfreq();
    }

    freq_initialised = true;
    return termfreq;
}

inline om_termcount
MultiPostList::get_collection_freq() const
{
    if(collfreq_initialised) return collfreq;
    DEBUGLINE(DB, "Calculating multiple term frequencies");

    // Calculate and remember the collfreq
    collfreq = 0;
    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	collfreq += (*i)->get_collection_freq();
    }

    collfreq_initialised = true;
    return collfreq;
}

inline om_docid
MultiPostList::get_docid() const
{
    DEBUGCALL(DB, om_docid, "MultiPostList::get_docid", "");
    Assert(!at_end());
    Assert(currdoc != 0);
    RETURN(currdoc);
}

inline bool
MultiPostList::at_end() const
{
    return finished;
}

inline std::string
MultiPostList::get_description() const
{
    std::string desc = "[";

    std::vector<LeafPostList *>::const_iterator i;
    for (i = postlists.begin(); i != postlists.end(); i++) {
	desc += (*i)->get_description();
	if (i != postlists.end()) desc += ",";
    }

    return desc + "]:" + om_tostring(get_termfreq());
}

#endif /* OM_HGUARD_MULTI_POSTLIST_H */
