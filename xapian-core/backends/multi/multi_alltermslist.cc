/* multialltermslist.cc
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

#include <config.h>
#include "multialltermslist.h"

MultiAllTermsList::MultiAllTermsList(const std::vector<TermList *> &lists_)
	: lists(lists_), is_at_end(false), started(false)
{
}

MultiAllTermsList::~MultiAllTermsList()
{
    std::vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	delete *i;
    }
    lists.clear();
}

void
MultiAllTermsList::update_current()
{
    bool found_term = false;

    std::vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	if ((*i)->at_end()) {
	    continue;
	} else if (!found_term) {
	    current = (*i)->get_termname();
	    found_term = true;
	} else {
	    std::string newterm = (*i)->get_termname();
	    if (newterm < current) {
		current = newterm;
	    }
	}
    }
    if (!found_term) {
	is_at_end = true;
    }
}

om_termcount
MultiAllTermsList::get_approx_size() const
{
    om_termcount size = 0;

    std::vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	size += (*i)->get_approx_size();
    }
    return size;
}

string
MultiAllTermsList::get_termname() const
{
    Assert(started);
    return current;
}

om_doccount
MultiAllTermsList::get_termfreq() const
{
    Assert(started);
    om_doccount termfreq = 0;

    std::vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    termfreq += (*i)->get_termfreq();
	}
    }
    return termfreq;
}

om_termcount
MultiAllTermsList::get_collection_freq() const
{
    om_termcount collection_freq = 0;

    std::vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i!=lists.end(); ++i) {
	if (!(*i)->at_end() &&
	    (*i)->get_termname() == current) {
	    collection_freq += (*i)->get_collection_freq();
	}
    }
    return collection_freq;
}

TermList *
MultiAllTermsList::skip_to(const string &tname)
{
    started = true;

    std::vector<TermList *>::const_iterator i;
    for (i = lists.begin(); i != lists.end(); ++i) {
	(*i)->skip_to(tname);
    }
    update_current();

    return NULL;
}

TermList *
MultiAllTermsList::next()
{
    if (!started) {
	started = true;
	
	std::vector<TermList *>::const_iterator i;
	for (i = lists.begin(); i != lists.end(); ++i) {
	    (*i)->next();
	}
    } else {

	std::vector<TermList *>::const_iterator i;
	for (i = lists.begin(); i != lists.end(); ++i) {
	    if (!(*i)->at_end() && (*i)->get_termname() == current) {
		(*i)->next();
	    }
	}
    }
    update_current();
    return NULL;
}

bool
MultiAllTermsList::at_end() const
{
    Assert(started);

    return is_at_end;
}
