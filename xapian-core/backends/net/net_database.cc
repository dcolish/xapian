/* net_database.cc: interface to network database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003 Olly Betts
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
#include <cstdlib>
#include "net_database.h"
#include "net_termlist.h"
#include "net_document.h"
#include "netclient.h"
#include "omdebug.h"
#include "utils.h"

#include "xapian/error.h"

///////////////////////////
// Actual database class //
///////////////////////////

NetworkDatabase::NetworkDatabase(Xapian::Internal::RefCntPtr<NetClient> link_) : link(link_)
{
    Assert(link.get() != 0);
}

NetworkDatabase::~NetworkDatabase() {
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }
}

void
NetworkDatabase::keep_alive() const
{
    link->keep_alive();
}

Xapian::doccount
NetworkDatabase::get_doccount() const
{
    return link->get_doccount();
}

Xapian::doclength
NetworkDatabase::get_avlength() const
{
    return link->get_avlength();
}

LeafPostList *
NetworkDatabase::do_open_post_list(const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::do_open_post_list() not implemented");
}

LeafTermList *
NetworkDatabase::open_term_list(Xapian::docid did) const {
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    vector<NetClient::TermListItem> items;
    link->get_tlist(did, items);
    return new NetworkTermList(get_avlength(), get_doccount(), items,
			       Xapian::Internal::RefCntPtr<const NetworkDatabase>(this));
}

Xapian::Document::Internal *
NetworkDatabase::open_document(Xapian::docid did, bool /*lazy*/) const
{
    // ignore lazy (for now at least - FIXME: can we sensibly pass it?)
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    string doc;
    map<Xapian::valueno, string> values;
    link->get_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

PositionList * 
NetworkDatabase::open_position_list(Xapian::docid /*did*/,
				    const string & /*tname*/) const
{
    throw Xapian::UnimplementedError("Network databases do not support opening positionlist");
}

void
NetworkDatabase::request_document(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    link->request_doc(did);
}

Xapian::Document::Internal *
NetworkDatabase::collect_document(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");
    string doc;
    map<Xapian::valueno, string> values;
    link->collect_doc(did, doc, values);
    return new NetworkDocument(this, did, doc, values);
}

Xapian::doclength
NetworkDatabase::get_doclength(Xapian::docid /*did*/) const
{
    throw Xapian::UnimplementedError("NetworkDatabase::get_doclength() not implemented");
}

bool
NetworkDatabase::term_exists(const string & tname) const
{
    Assert(!tname.empty());
    // FIXME: have cache of termfreqs?
    return link->term_exists(tname);
}

Xapian::doccount
NetworkDatabase::get_termfreq(const string & tname) const
{
    Assert(!tname.empty());
    // FIXME: have cache of termfreqs?
    return link->get_termfreq(tname);
}

TermList *
NetworkDatabase::open_allterms() const
{
    throw Xapian::UnimplementedError("open_allterms() not implemented yet");
}
