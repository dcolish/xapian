/* sleepy_database.h: C++ class definition for sleepycat access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_SLEEPY_DATABASE_H
#define OM_HGUARD_SLEEPY_DATABASE_H

#include "utils.h"
#include "omassert.h"
#include "leafpostlist.h"
#include "termlist.h"
#include "database.h"
#include "sleepy_termcache.h"
#include <stdlib.h>

class SleepyDatabaseInternals;

class SleepyDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    private:
	SleepyDatabaseInternals * internals;
	bool opened;

	SleepyDatabaseTermCache * termcache;

	void open(const DatabaseBuilderParams & params);
	SleepyDatabase();
    public:
	~SleepyDatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;

	om_doclength get_doclength(om_docid did) const;
	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * open_post_list(const om_termname& tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;

	void make_term(const om_termname &) {
	    throw OmUnimplementedError("DADatabase::make_term() not implemented");
	}
	om_docid make_doc(const om_docname &) {
	    throw OmUnimplementedError("DADatabase::make_doc() not implemented");
	}
	void make_posting(const om_termname &, unsigned int, unsigned int) {
	    throw OmUnimplementedError("DADatabase::make_posting() not implemented");
	}
};



///////////////////////////////////////////
// Inline definitions for SleepyDatabase //
///////////////////////////////////////////

inline om_doccount
SleepyDatabase::get_doccount() const
{
    Assert(opened);
    return 1;
}

inline om_doclength
SleepyDatabase::get_avlength() const
{
    Assert(opened);
    return 1;
}

inline om_doclength
SleepyDatabase::get_doclength(om_docid did) const
{
    Assert(opened);
    return 1;
}

inline om_doccount
SleepyDatabase::get_termfreq(const om_termname &tname) const
{   
    PostList *pl = open_post_list(tname);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

inline bool
SleepyDatabase::term_exists(const om_termname &tname) const
{
    if(termcache->term_name_to_id(tname)) return true;
    return false;
}

#endif /* OM_HGUARD_SLEEPY_DATABASE_H */
