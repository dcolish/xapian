/* ombatchenquire.cc: External interface for running query batches
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
#include "config.h"

#include "om/omenquire.h"
#include "omenquireinternal.h"
#include <algorithm>
#include <math.h>
#include "omdebug.h"

/////////////////////////////////
// Internals of OmBatchEnquire //
/////////////////////////////////

class OmBatchEnquire::Internal {
    private:
        mutable OmEnquire::Internal enquire;

	query_batch queries;

    public:
	Internal(const OmDatabase &databases,
		 OmErrorHandler * errorhandler_ = 0)
		: enquire(databases, errorhandler_) {};
	~Internal() {};

	void set_queries(const query_batch &queries_);
        mset_batch get_msets() const;

	const OmDocument get_doc(const OmMSetIterator &it) const;
	const OmDocument get_doc(om_docid did) const;
	OmTermIterator get_matching_terms(const OmMSetIterator &it) const;
	OmTermIterator get_matching_terms(om_docid did) const;

	std::string get_description() const;
};

////////////////////////////
// OmBatchEnquire methods //
////////////////////////////

OmBatchEnquire::OmBatchEnquire(const OmDatabase &databases) : internal(0)
{
    internal = new Internal(databases);
}

OmBatchEnquire::~OmBatchEnquire()
{
    delete internal;
}

void
OmBatchEnquire::set_queries(const query_batch &queries_)
{
    internal->set_queries(queries_);
}

OmBatchEnquire::mset_batch
OmBatchEnquire::get_msets() const
{
    return internal->get_msets();
}

const OmDocument
OmBatchEnquire::get_doc(om_docid did) const
{
    return internal->get_doc(did);
}

const OmDocument
OmBatchEnquire::get_doc(const OmMSetIterator &it) const
{
    return internal->get_doc(it);
}

OmTermIterator
OmBatchEnquire::get_matching_terms(om_docid did) const
{
    return internal->get_matching_terms(did);
}

OmTermIterator
OmBatchEnquire::get_matching_terms(const OmMSetIterator &it) const
{
    return internal->get_matching_terms(it);
}

std::string
OmBatchEnquire::get_description() const
{
    DEBUGCALL(INTRO, std::string, "Calling OmBatchEnquire::get_description()", "");
    RETURN("OmBatchEnquire(" + internal->get_description() + ")");
}

/////////////////////////////////////////
// Methods of OmBatchEnquire::Internal //
/////////////////////////////////////////

void
OmBatchEnquire::Internal::set_queries(const query_batch &queries_)
{
    queries = queries_;
}

OmBatchEnquire::mset_batch
OmBatchEnquire::Internal::get_msets() const
{
    mset_batch result;

    query_batch::const_iterator q = queries.begin();
    while (q != queries.end()) {
	try {
	    enquire.set_query(q->query);
	    OmMSet mset = enquire.get_mset(q->first,
					   q->maxitems,
					   q->omrset,
					   q->moptions,
					   q->mdecider);
	    result.push_back(batch_result(mset, true));
	} catch (OmInvalidArgumentError &err) {
	    /* if it's a query-specific error (ie bad arguments),
	     * then put an invalid placeholder into the result
	     * and keep processing.
	     */
            result.push_back(batch_result(OmMSet(), false));
	}
	++q;
    }

    Assert(result.size() == queries.size());

    return result;
}

const OmDocument
OmBatchEnquire::Internal::get_doc(om_docid did) const
{
    return enquire.get_doc(did);
}

const OmDocument
OmBatchEnquire::Internal::get_doc(const OmMSetIterator &it) const
{
    return enquire.get_doc(it);
}

OmTermIterator
OmBatchEnquire::Internal::get_matching_terms(om_docid did) const
{
    return enquire.get_matching_terms(did);
}

OmTermIterator
OmBatchEnquire::Internal::get_matching_terms(const OmMSetIterator &it) const
{
    return enquire.get_matching_terms(it);
}

std::string
OmBatchEnquire::Internal::get_description() const
{
    std::string description;

    description = enquire.get_description() + ", " +
	    om_tostring(queries.size()) + "queries";

    return description;
}

////////////////////////////////////////
// OmBatchQuery::batch_result methods //
////////////////////////////////////////

OmBatchEnquire::batch_result::batch_result(const OmMSet &mset,
					   bool isvalid_)
	: isvalid(isvalid_), result(mset)
{}

OmMSet
OmBatchEnquire::batch_result::value() const
{
    if (isvalid) {
	return result;
    } else {
	throw OmInvalidResultError("Query was not successfully run.");
    }
}
