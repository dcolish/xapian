/* omtermlistiterator.cc
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

#include "om/omtermlistiterator.h"
#include "omtermlistiteratorinternal.h"
#include "termlist.h"
#include "omdebug.h"

OmTermListIterator::~OmTermListIterator() {
    DEBUGAPICALL(void, "OmTermListIterator::~OmTermListIterator", "");
    delete internal;
}

OmTermListIterator::OmTermListIterator(const OmTermListIterator &other)
    : internal(NULL)
{
    DEBUGAPICALL(void, "OmTermListIterator::OmTermListIterator", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmTermListIterator::operator=(const OmTermListIterator &other)
{
    DEBUGAPICALL(void, "OmTermListIterator::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmTermListIterator assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

const om_termname
OmTermListIterator::operator *() const
{
    DEBUGAPICALL(om_termname, "OmTermListIterator::operator*", "");
    RETURN(internal->termlist->get_termname());
}

om_termcount
OmTermListIterator::get_wdf() const
{
    DEBUGAPICALL(om_termcount, "OmTermListIterator::get_wdf", "");
    RETURN(internal->termlist->get_wdf());
}

om_doccount
OmTermListIterator::get_termfreq() const
{
    DEBUGAPICALL(om_doccount, "OmTermListIterator::get_termfreq", "");
    RETURN(internal->termlist->get_termfreq());
}

OmTermListIterator &
OmTermListIterator::operator++()
{
    DEBUGAPICALL(OmTermListIterator &, "OmTermListIterator::operator++", "");
    internal->termlist->next();
    RETURN(*this);
}

void
OmTermListIterator::operator++(int)
{
    DEBUGAPICALL(void, "OmTermListIterator::operator++(int)", "");
    internal->termlist->next();
}

// extra method, not required to be an input_iterator
void
OmTermListIterator::skip_to(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmTermListIterator::skip_to", tname);
    while (!internal->termlist->at_end() &&
	   internal->termlist->get_termname() < tname)
	internal->termlist->next();
}

OmPositionListIterator
OmTermListIterator::positionlist_begin()
{
    DEBUGAPICALL(OmPositionListIterator, "OmTermListIterator::positionlist_begin", "");
    RETURN(internal->database.positionlist_begin(internal->did,
						 internal->termlist->get_termname()));
}

OmPositionListIterator
OmTermListIterator::positionlist_end()
{
    DEBUGAPICALL(OmPositionListIterator, "OmTermListIterator::positionlist_end", "");
    RETURN(OmPositionListIterator(NULL));
}

std::string
OmTermListIterator::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmTermListIterator::get_description", "");
    /// \todo display contents of the object
    RETURN("OmTermListIterator()");
}


bool
operator==(const OmTermListIterator &a, const OmTermListIterator &b)
{
    if (a.internal == b.internal) return true;
    if (a.internal) {
	if (b.internal) return a.internal->termlist.get() == b.internal->termlist.get();
	return a.internal->termlist->at_end();
    }
    Assert(b.internal); // a.internal is NULL, so b.internal can't be
    return b.internal->termlist->at_end();
}

//////////////////////////////////////////////////////////////////////

bool
operator==(const OmTermListIterator &a, const OmTermListIteratorMap &b)
{
    return false;
}

bool
operator==(const OmTermListIteratorMap &a, const OmTermListIterator &b)
{
    return false;
}

bool
operator==(const OmTermListIteratorMap &a, const OmTermListIteratorMap &b)
{
    return (a.internal->it == b.internal->it);
}

OmTermListIteratorMap::~OmTermListIteratorMap() {
    DEBUGAPICALL(void, "OmTermListIteratorMap::~OmTermListIteratorMap", "");
    delete internal;
}

OmTermListIteratorMap::OmTermListIteratorMap(const OmTermListIteratorMap &other)
    : OmTermListIterator(NULL), internal(NULL)
{
    DEBUGAPICALL(void, "OmTermListIteratorMap::OmTermListIteratorMap", other);
    if (other.internal) internal = new Internal(*(other.internal));
}

void
OmTermListIteratorMap::operator=(const OmTermListIteratorMap &other)
{
    DEBUGAPICALL(void, "OmTermListIteratorMap::operator=", other);
    if (this == &other) {
	DEBUGLINE(API, "OmTermListIteratorMap assigned to itself");
	return;
    }

    Internal * newinternal = NULL;
    if (other.internal)
	newinternal = new Internal(*(other.internal));
    std::swap(internal, newinternal);
    delete newinternal;
}

const om_termname
OmTermListIteratorMap::operator *() const
{
    DEBUGAPICALL(om_termname, "OmTermListIteratorMap::operator*", "");
    RETURN(internal->it->first);
}

om_termcount
OmTermListIteratorMap::get_wdf() const
{
    DEBUGAPICALL(om_termcount, "OmTermListIteratorMap::get_wdf", "");
    RETURN(internal->it->second.wdf);
}

om_doccount
OmTermListIteratorMap::get_termfreq() const
{
    DEBUGAPICALL(om_doccount, "OmTermListIteratorMap::get_termfreq", "");
    RETURN(internal->it->second.termfreq);
}

OmTermListIteratorMap &
OmTermListIteratorMap::operator++()
{
    DEBUGAPICALL(OmTermListIteratorMap &, "OmTermListIteratorMap::operator++", "");
    internal->it++;
    RETURN(*this);
}

void
OmTermListIteratorMap::operator++(int)
{
    DEBUGAPICALL(void, "OmTermListIteratorMap::operator++(int)", "");
    internal->it++;
}

// extra method, not required to be an input_iterator
void
OmTermListIteratorMap::skip_to(const om_termname & tname)
{
    DEBUGAPICALL(void, "OmTermListIteratorMap::skip_to", tname);
    throw OmUnimplementedError("OmTermListIteratorMap::skip_to not yet there");
#if 0
    while (internal->it != internal->it_end &&
	   internal->termlist->get_termname() < tname)
	internal->termlist->next();
#endif
}

OmPositionListIterator
OmTermListIteratorMap::positionlist_begin()
{
    DEBUGAPICALL(OmPositionListIterator, "OmTermListIteratorMap::positionlist_begin", "");
    RETURN(internal->database.positionlist_begin(internal->did,
						 internal->it->first));
}

OmPositionListIterator
OmTermListIteratorMap::positionlist_end()
{
    DEBUGAPICALL(OmPositionListIterator, "OmTermListIteratorMap::positionlist_end", "");
    RETURN(OmPositionListIterator(NULL));
}

std::string
OmTermListIteratorMap::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmTermListIteratorMap::get_description", "");
    /// \todo display contents of the object
    RETURN("OmTermListIteratorMap()");
}
