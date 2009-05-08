/** @file postingsource.cc
 * @brief External sources of posting information
 */
/* Copyright (C) 2008,2009 Olly Betts
 * Copyright (C) 2008,2009 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "xapian/postingsource.h"

#include "autoptr.h"

#include "database.h"
#include "document.h"
#include "matcher/externalpostlist.h"

#include "xapian/document.h"
#include "xapian/error.h"
#include "xapian/queryparser.h" // For sortable_unserialise().

#include "omassert.h"
#include "serialise.h"
#include "serialise-double.h"
#include "str.h"

#include <cfloat>

using namespace std;

namespace Xapian {

void
PostingSource::notify_new_maxweight()
{
    if (externalpl)
	externalpl->notify_new_maxweight();
}

PostingSource::~PostingSource() { }

Xapian::weight
PostingSource::get_maxweight() const
{
    return 0;
}

Xapian::weight
PostingSource::get_weight() const
{
    return 0;
}

void
PostingSource::skip_to(Xapian::docid did, Xapian::weight min_wt)
{
    while (!at_end() && get_docid() < did) {
	next(min_wt);
    }
}

bool
PostingSource::check(Xapian::docid did, Xapian::weight min_wt)
{
    skip_to(did, min_wt);
    return true;
}

PostingSource *
PostingSource::clone() const
{
    return NULL;
}

string
PostingSource::name() const
{
    return string();
}

string
PostingSource::serialise() const
{
    throw Xapian::InvalidOperationError("serialise() not supported for this PostingSource");
}

PostingSource *
PostingSource::unserialise(const string &) const
{
    throw Xapian::InvalidOperationError("unserialise() not supported for this PostingSource");
}

string
PostingSource::get_description() const
{
    return "Xapian::PostingSource subclass";
}


ValuePostingSource::ValuePostingSource(Xapian::valueno slot_)
	: slot(slot_)
{
}

Xapian::doccount
ValuePostingSource::get_termfreq_min() const
{
    return termfreq_min;
}

Xapian::doccount
ValuePostingSource::get_termfreq_est() const
{
    return termfreq_est;
}

Xapian::doccount
ValuePostingSource::get_termfreq_max() const
{
    return termfreq_max;
}

Xapian::weight
ValuePostingSource::get_maxweight() const
{
    return max_weight;
}

void
ValuePostingSource::next(Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	value_it = db.valuestream_begin(slot);
    } else {
	++value_it;
    }

    if (value_it == db.valuestream_end(slot)) return;

    if (min_wt > max_weight) {
	value_it = db.valuestream_end(slot);
	return;
    }
}

void
ValuePostingSource::skip_to(Xapian::docid min_docid,
				  Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	value_it = db.valuestream_begin(slot);

	if (value_it == db.valuestream_end(slot)) return;
    }

    if (min_wt > max_weight) {
	value_it = db.valuestream_end(slot);
	return;
    }
    value_it.skip_to(min_docid);
}

bool
ValuePostingSource::check(Xapian::docid min_docid,
				Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	value_it = db.valuestream_begin(slot);

	if (value_it == db.valuestream_end(slot)) return true;
    }

    if (min_wt > max_weight) {
	value_it = db.valuestream_end(slot);
	return true;
    }
    return value_it.check(min_docid);
}

bool
ValuePostingSource::at_end() const
{
    return started && value_it == db.valuestream_end(slot);
}

Xapian::docid
ValuePostingSource::get_docid() const
{
    return value_it.get_docid();
}

void
ValuePostingSource::init(const Database & db_)
{
    db = db_;
    started = false;
    max_weight = DBL_MAX;
    try {
	termfreq_max = db.get_value_freq(slot);
	termfreq_est = termfreq_max;
	termfreq_min = termfreq_max;
    } catch (const Xapian::UnimplementedError &) {
	termfreq_max = db.get_doccount();
	termfreq_est = termfreq_max / 2;
	termfreq_min = 0;
    }
}


ValueWeightPostingSource::ValueWeightPostingSource(Xapian::valueno slot_)
	: ValuePostingSource(slot_)
{
}

Xapian::weight
ValueWeightPostingSource::get_weight() const
{
    Assert(!at_end());
    Assert(started);
    return sortable_unserialise(*value_it);
}

ValueWeightPostingSource *
ValueWeightPostingSource::clone() const
{
    return new ValueWeightPostingSource(slot);
}

string
ValueWeightPostingSource::name() const
{
    return string("Xapian::ValueWeightPostingSource");
}

string
ValueWeightPostingSource::serialise() const
{
    return encode_length(slot);
}

ValueWeightPostingSource *
ValueWeightPostingSource::unserialise(const string &s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::valueno new_valno = decode_length(&p, end, false);
    if (p != end) {
	throw Xapian::NetworkError("Bad serialised ValueWeightPostingSource - junk at end");
    }

    return new ValueWeightPostingSource(new_valno);
}

void
ValueWeightPostingSource::init(const Database & db_)
{
    ValuePostingSource::init(db_);

    try {
    	max_weight = sortable_unserialise(db.get_value_upper_bound(slot));
    } catch (const Xapian::UnimplementedError &) {
	max_weight = DBL_MAX;
    }
}

string
ValueWeightPostingSource::get_description() const
{
    string desc("Xapian::ValueWeightPostingSource(slot=");
    desc += str(slot);
    desc += ")";
    return desc;
}


ValueMapPostingSource::ValueMapPostingSource(Xapian::valueno slot_)
	: ValuePostingSource(slot_),
	  default_weight(0.0),
	  max_weight_in_map(0.0)
{
}

void
ValueMapPostingSource::add_mapping(const string & key, double weight)
{
    weight_map[key] = weight;
    max_weight_in_map = max(weight, max_weight_in_map);
}

void
ValueMapPostingSource::clear_mappings()
{
    weight_map.clear();
    max_weight_in_map = 0.0;
}

void
ValueMapPostingSource::set_default_weight(double wt)
{
    default_weight = wt;
}

Xapian::weight
ValueMapPostingSource::get_weight() const
{
    map<string, double>::const_iterator wit = weight_map.find(*value_it);
    if (wit == weight_map.end()) {
	return default_weight;
    }
    return wit->second;
}

ValueMapPostingSource *
ValueMapPostingSource::clone() const
{
    AutoPtr<ValueMapPostingSource> res(new ValueMapPostingSource(slot));
    map<string, double>::const_iterator i;
    for (i = weight_map.begin(); i != weight_map.end(); ++i) {
	res->add_mapping(i->first, i->second);
    }
    res->set_default_weight(default_weight);
    return res.release();
}

string
ValueMapPostingSource::name() const
{
    return string("Xapian::ValueMapPostingSource");
}

string
ValueMapPostingSource::serialise() const
{
    string result = encode_length(slot);
    result += serialise_double(default_weight);

    map<string, double>::const_iterator i;
    for (i = weight_map.begin(); i != weight_map.end(); ++i) {
	result.append(encode_length(i->first.size()));
	result.append(i->first);
	result.append(serialise_double(i->second));
    }

    return result;
}

ValueMapPostingSource *
ValueMapPostingSource::unserialise(const string &s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::valueno new_slot = decode_length(&p, end, false);
    AutoPtr<ValueMapPostingSource> res(new ValueMapPostingSource(new_slot));
    res->set_default_weight(unserialise_double(&p, end));
    while (p != end) {
	size_t keylen = decode_length(&p, end, true);
	string key(p, keylen);
	p += keylen;
	res->add_mapping(key, unserialise_double(&p, end));
    }
    return res.release();
}

void
ValueMapPostingSource::init(const Database & db_)
{
    ValuePostingSource::init(db_);
    max_weight = max(max_weight_in_map, default_weight);
}

string
ValueMapPostingSource::get_description() const
{
    string desc("Xapian::ValueMapPostingSource(slot=");
    desc += str(slot);
    desc += ")";
    return desc;
}


FixedWeightPostingSource::FixedWeightPostingSource(Xapian::weight wt_)
	: wt(wt_),
	  started(false)
{
}

Xapian::doccount
FixedWeightPostingSource::get_termfreq_min() const
{
    return termfreq;
}

Xapian::doccount
FixedWeightPostingSource::get_termfreq_est() const
{
    return termfreq;
}

Xapian::doccount
FixedWeightPostingSource::get_termfreq_max() const
{
    return termfreq;
}

Xapian::weight
FixedWeightPostingSource::get_maxweight() const
{
    return wt;
}

Xapian::weight
FixedWeightPostingSource::get_weight() const
{
    return wt;
}

void
FixedWeightPostingSource::next(Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.postlist_begin(string());
    } else {
	++it;
    }

    if (it == db.postlist_end(string())) return;

    if (check_docid) {
	it.skip_to(check_docid + 1);
	check_docid = 0;
    }

    if (min_wt > wt) {
	it = db.postlist_end(string());
    }
}

void
FixedWeightPostingSource::skip_to(Xapian::docid min_docid,
				  Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.postlist_begin(string());

	if (it == db.postlist_end(string())) return;
    }

    if (check_docid) {
	if (min_docid < check_docid)
	    min_docid = check_docid + 1;
	check_docid = 0;
    }

    if (min_wt > wt) {
	it = db.postlist_end(string());
	return;
    }
    it.skip_to(min_docid);
}

bool
FixedWeightPostingSource::check(Xapian::docid min_docid,
				Xapian::weight)
{
    // We're guaranteed not to be called if the document doesn't
    // exist, so just remember the docid passed, and return true.
    check_docid = min_docid;
    return true;
}

bool
FixedWeightPostingSource::at_end() const
{
    if (check_docid != 0) return false;
    return started && it == db.postlist_end(string());
}

Xapian::docid
FixedWeightPostingSource::get_docid() const
{
    if (check_docid != 0) return check_docid;
    return *it;
}

FixedWeightPostingSource *
FixedWeightPostingSource::clone() const
{
    return new FixedWeightPostingSource(wt);
}

string
FixedWeightPostingSource::name() const
{
    return string("Xapian::FixedWeightPostingSource");
}

string
FixedWeightPostingSource::serialise() const
{
    return serialise_double(wt);
}

FixedWeightPostingSource *
FixedWeightPostingSource::unserialise(const string &s) const
{
    const char * p = s.data();
    const char * s_end = p + s.size();
    double new_wt = unserialise_double(&p, s_end);
    if (p != s_end) {
	throw Xapian::NetworkError("Bad serialised ValueWeightPostingSource - junk at end");
    }
    return new FixedWeightPostingSource(new_wt);
}

void
FixedWeightPostingSource::init(const Xapian::Database & db_)
{
    db = db_;
    termfreq = db_.get_doccount();
    started = false;
    check_docid = 0;
}

string
FixedWeightPostingSource::get_description() const
{
    string desc("Xapian::FixedWeightPostingSource(wt=");
    desc += str(wt);
    desc += ")";
    return desc;
}

}
