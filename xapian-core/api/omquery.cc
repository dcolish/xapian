/* omquery.cc: External interface for running queries
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

#include "omdebug.h"
#include "omlocks.h"
#include "omqueryinternal.h"
#include "utils.h"
#include "netutils.h"

#include <om/omerror.h>
#include <om/omenquire.h>
#include <om/omoutput.h>

#include <vector>
#include <map>
#include <algorithm>
#include <math.h>

/////////////////////////
// Methods for OmQuery //
/////////////////////////

OmQuery::OmQuery(const om_termname & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery",
		 tname_ << ", " << wqf_ << ", " << term_pos_);
    internal = new OmQueryInternal(tname_, wqf_, term_pos_);
}

OmQuery::OmQuery(om_queryop op_, const OmQuery &left, const OmQuery &right)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << left << ", " << right);
    internal = new OmQueryInternal(op_,
				   *(left.internal),
				   *(right.internal));
}

OmQuery::OmQuery(om_queryop op_,
		 const vector<OmQuery *>::const_iterator qbegin,
		 const vector<OmQuery *>::const_iterator qend,
		 om_termpos window)
	: internal(0)
{
    // FIXME: display the contents of the vector
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << "vector<OmQuery *>, " << window);
    vector<OmQueryInternal *> temp;
    vector<OmQuery *>::const_iterator i = qbegin;
    while (i != qend) {
	temp.push_back((*i)->internal);
	++i;
    }
    internal = new OmQueryInternal(op_, temp.begin(), temp.end(), window);
}

OmQuery::OmQuery(om_queryop op_,
		 const vector<OmQuery>::const_iterator qbegin,
		 const vector<OmQuery>::const_iterator qend,
		 om_termpos window)
	: internal(0)
{   
    // FIXME: display the contents of the vector
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << "vector<OmQuery>, " << window);
    vector<OmQueryInternal *> temp;
    vector<OmQuery>::const_iterator i = qbegin;
    while (i != qend) {
	temp.push_back(i->internal);
	++i;
    }
    internal = new OmQueryInternal(op_, temp.begin(), temp.end(), window);
}


OmQuery::OmQuery(om_queryop op_,
		 const vector<om_termname>::const_iterator tbegin,
		 const vector<om_termname>::const_iterator tend,
		 om_termpos window)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery",
		 op_ << ", " << "vector<om_termname>, " << window);
    internal = new OmQueryInternal(op_, tbegin, tend, window);
}

// Copy constructor
OmQuery::OmQuery(const OmQuery & copyme)
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery", copyme);
    internal = new OmQueryInternal(*(copyme.internal));
}

// Assignment
OmQuery &
OmQuery::operator=(const OmQuery & copyme)
{
    DEBUGAPICALL("OmQuery::operator=", copyme);
    OmQueryInternal * temp = new OmQueryInternal(*(copyme.internal));
    swap(temp, this->internal);
    delete temp;

    DEBUGAPIRETURN(*this);
    return *this;
}

// Default constructor
OmQuery::OmQuery()
	: internal(0)
{
    DEBUGAPICALL("OmQuery::OmQuery", "");
    internal = new OmQueryInternal();
}

// Destructor
OmQuery::~OmQuery()
{
    DEBUGAPICALL("OmQuery::~OmQuery", "");
    delete internal;
}

string
OmQuery::get_description() const
{
    DEBUGAPICALL("OmQuery::get_description", "");
    OmLockSentry locksentry(internal->mutex);
    string description = "OmQuery(" + internal->get_description() + ")";
    DEBUGAPIRETURN(description);
    return description;
}

bool OmQuery::is_defined() const
{
    DEBUGAPICALL("OmQuery::is_defined", "");
    OmLockSentry locksentry(internal->mutex);
    DEBUGAPIRETURN(internal->is_defined());
    return internal->is_defined();
}

bool OmQuery::is_bool() const
{
    DEBUGAPICALL("OmQuery::is_bool", "");
    OmLockSentry locksentry(internal->mutex);
    bool ret = internal->is_bool();
    DEBUGAPIRETURN(ret);
    return ret;
}

bool OmQuery::set_bool(bool isbool_)
{
    DEBUGAPICALL("OmQuery::set_bool", isbool_);
    OmLockSentry locksentry(internal->mutex);
    bool ret = internal->set_bool(isbool_);
    DEBUGAPIRETURN(ret);
    return ret;
}

om_termcount OmQuery::get_length() const
{
    DEBUGAPICALL("OmQuery::get_length", "");
    OmLockSentry locksentry(internal->mutex);
    om_termcount ret = internal->get_length();
    DEBUGAPIRETURN(ret);
    return ret;
}

om_termcount OmQuery::set_length(om_termcount qlen_)
{
    DEBUGAPICALL("OmQuery::set_length", qlen_);
    OmLockSentry locksentry(internal->mutex);
    om_termcount ret = internal->set_length(qlen_);
    DEBUGAPIRETURN(ret);
    return ret;
}

om_termname_list OmQuery::get_terms() const
{
    DEBUGAPICALL("OmQuery::get_terms", "");
    OmLockSentry locksentry(internal->mutex);
    DEBUGAPIRETURN(internal->get_terms());
    return internal->get_terms();
}

/////////////////////////////////
// Methods for OmQueryInternal //
/////////////////////////////////

/** serialising method, for network matches.
 *
 *  The format is designed to be relatively easy
 *  to parse, as well as encodable in one line of text.
 *
 *  A null query is represented as `%N'.
 *
 *  A single-term query becomes `%T<tname>,<wqf>,<termpos>',
 *  where:
 *  	<tname> is the encoded term name
 *  	<wqf> is the decimal within query frequency,
 *  	<termpos> is the decimal term position.
 *
 *  A term name is encoded as a string of hex digits, two per
 *  byte in the term.  The first digit is the high nibble, and
 *  the second is the low nibble.
 *
 *  A compound query becomes `%(<subqueries> <op>%)', where:
 *  	<subqueries> is the space-separated list of subqueries
 *  	<op> is one of: %and, %or, %filter, %andmaybe, %andnot, %xor
 */
string
OmQueryInternal::serialise() const
{
    if (!isdefined) return "%N";

    string result;

    if (isbool) {
	result = "%B";
    }
    if (op == OM_MOP_LEAF) {
	result += "%T" + encode_tname(tname) +
		"," + om_inttostring(wqf) +
		"," + om_inttostring(term_pos);
    } else {
	result += "%(";
	for (subquery_list::const_iterator i=subqs.begin();
	     i != subqs.end();
	     ++i) {
	    result += (*i)->serialise() + " ";
	}
	switch (op) {
	    case OM_MOP_LEAF:
		Assert(false);
		break;
	    case OM_MOP_AND:
		result += "%and";
		break;
	    case OM_MOP_OR:
		result += "%or";
		break;
	    case OM_MOP_FILTER:
		result += "%filter";
		break;
	    case OM_MOP_AND_MAYBE:
		result += "%andmaybe";
		break;
	    case OM_MOP_AND_NOT:
		result += "%andnot";
		break;
	    case OM_MOP_XOR:
		result += "%xor";
		break;
	    case OM_MOP_NEAR:
		result += "%near" + om_inttostring(window);
		break;
	    case OM_MOP_PHRASE:
		result += "%phrase" + om_inttostring(window);
		break;
	} // switch(op)
	result += "%)";
    }
    return result;
}

// Introspection
string
OmQueryInternal::get_description() const
{
    if(!isdefined) return "<NULL>";
    string opstr;
    switch(op) {
	case OM_MOP_LEAF:
	    return tname;
	    break;
	case OM_MOP_AND:
	    opstr = " AND ";
	    break;
	case OM_MOP_OR:
	    opstr = " OR ";
	    break;
	case OM_MOP_FILTER:
	    opstr = " FILTER ";
	    break;
	case OM_MOP_AND_MAYBE:
	    opstr = " AND_MAYBE ";
	    break;
	case OM_MOP_AND_NOT:
	    opstr = " AND_NOT ";
	    break;
	case OM_MOP_XOR:
	    opstr = " XOR ";
	    break;
	case OM_MOP_NEAR:
	    opstr = " NEAR " + om_inttostring(window) + " ";
	    break;
	case OM_MOP_PHRASE:
	    opstr = " PHRASE " + om_inttostring(window) + " ";
	    break;
    }
    string description;
    vector<OmQueryInternal *>::const_iterator i;
    for(i = subqs.begin(); i != subqs.end(); i++) {
	if(description.size()) description += opstr;
	description += (**i).get_description();
    }
    return "(" + description + ")";
}

bool
OmQueryInternal::set_bool(bool isbool_)
{
    bool oldbool = isbool;
    isbool = isbool_;
    return oldbool;
}

om_termcount
OmQueryInternal::set_length(om_termcount qlen_)
{
    om_termcount oldqlen = qlen;
    qlen = qlen_;
    return oldqlen;
}

void
OmQueryInternal::accumulate_terms(
			vector<pair<om_termname, om_termpos> > &terms) const
{ 
    Assert(isdefined);

    if (op == OM_MOP_LEAF) {
        // We're a leaf, so just return our term.
        terms.push_back(make_pair(tname, term_pos));
    } else {
    	subquery_list::const_iterator end = subqs.end();
        // not a leaf, concatenate results from all subqueries
	for (subquery_list::const_iterator i = subqs.begin();
	     i != end;
	     ++i) {
 	    (*i)->accumulate_terms(terms);
	}
    }
}

struct LessByTermpos {
    typedef const pair<om_termname, om_termpos> argtype;
    bool operator()(argtype &left, argtype &right) {
	if (left.second != right.second) {
	    return left.second < right.second;
	} else {
	    return left.first < right.first;
	}
    }
};

om_termname_list
OmQueryInternal::get_terms() const
{
    om_termname_list result;

    vector<pair<om_termname, om_termpos> > terms;
    if (isdefined) {
        accumulate_terms(terms);
    }

    sort(terms.begin(), terms.end(), LessByTermpos());

    // remove adjacent duplicates, and return an iterator pointing
    // to just after the last unique element
    vector<pair<om_termname, om_termpos> >::iterator newlast =
	    	unique(terms.begin(), terms.end());
    // and remove the rest...  (See Stroustrup 18.6.3)
    terms.erase(newlast, terms.end());

    vector<pair<om_termname, om_termpos> >::const_iterator i;
    for (i=terms.begin(); i!= terms.end(); ++i) {
	result.push_back(i->first);
    }

    return result;
}

OmQueryInternal::OmQueryInternal()
	: mutex(), isdefined(false), qlen(0)
{}

void
OmQueryInternal::swap(OmQueryInternal &other)
{
    std::swap(isdefined, other.isdefined);
    std::swap(isbool, other.isbool);
    std::swap(op, other.op);
    subqs.swap(other.subqs);
    std::swap(qlen, other.qlen);
    std::swap(tname, other.tname);
    std::swap(term_pos, other.term_pos);
    std::swap(wqf, other.wqf);
    std::swap(max_weight, other.max_weight);
    std::swap(window, other.window);
}

void
OmQueryInternal::operator=(const OmQueryInternal &copyme)
{
    // The exception-safe assignment operator idiom:
    OmQueryInternal temp(copyme);
    this->swap(temp);
}

OmQueryInternal::OmQueryInternal(const OmQueryInternal &copyme)
	: mutex(), isdefined(copyme.isdefined),
	isbool(copyme.isbool), op(copyme.op),
	subqs(subquery_list()), qlen(copyme.qlen),
	tname(copyme.tname), term_pos(copyme.term_pos),
	wqf(copyme.wqf), max_weight(copyme.max_weight), window(copyme.window)
{
    try {
	for (subquery_list::const_iterator i = copyme.subqs.begin();
	     i != copyme.subqs.end();
	     ++i) {
	    subqs.push_back(new OmQueryInternal(**i));
	}
    } catch (...) {
	// Delete the allocated subqueries if we fail
	for (subquery_list::const_iterator del = subqs.begin();
	     del != subqs.end();
	     ++del) {
	    delete *del;
	}
	// and rethrow the exception
	throw;
    }
}

OmQueryInternal::OmQueryInternal(const om_termname & tname_,
		 om_termcount wqf_,
		 om_termpos term_pos_)
	: isdefined(true), isbool(false), op(OM_MOP_LEAF),
	qlen(wqf_), tname(tname_), term_pos(term_pos_), wqf(wqf_)
{
    if(tname.size() == 0) {
	throw OmInvalidArgumentError("Termnames may not have zero length.");
    }
}

OmQueryInternal::OmQueryInternal(om_queryop op_,
				 const OmQueryInternal &left,
				 const OmQueryInternal &right)
	: isdefined(true), isbool(false), op(op_),
	  qlen(left.qlen + right.qlen)
{
    if (op == OM_MOP_LEAF) {
    	throw OmInvalidArgumentError("Invalid query operation");
    }

    if (op == OM_MOP_NEAR || op == OM_MOP_PHRASE) {
    	throw OmInvalidArgumentError("NEAR/PHRASE take window size and list of terms or queries");
    }

    // reject any attempt to make up a composite query when any sub-query
    // is a pure boolean query.  FIXME: ought to handle the different
    // operators specially.
    if ((left.isdefined && left.isbool) || (right.isdefined && right.isbool)) {
	throw OmInvalidArgumentError("Only the top-level query can be bool");
    }

    // Handle undefined sub-queries.
    // See documentation for table for result of operations when one of the
    // operands is undefined:
    //
    if(!left.isdefined || !right.isdefined) {
	switch (op) {
	    case OM_MOP_OR:
	    case OM_MOP_AND:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) initialise_from_copy(right);
		    else isdefined = false;
		}
		break;
	    case OM_MOP_FILTER:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) {
			// Pure boolean
			initialise_from_copy(right);
			isbool = true;
		    } else {
			isdefined = false;
		    }
		}
		break;
	    case OM_MOP_AND_MAYBE:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) initialise_from_copy(right);
		    else isdefined = false;
		}
		break;
	    case OM_MOP_AND_NOT:
		if (left.isdefined) {
		    initialise_from_copy(left);
		} else {
		    if (right.isdefined) {
		        throw OmInvalidArgumentError("AND NOT can't have an undefined LHS");
		    } else isdefined = false;
		}
		break;
	    case OM_MOP_XOR:
		if (!left.isdefined && !right.isdefined) {
		    isdefined = true;
		} else {
		    throw OmInvalidArgumentError("XOR can't have one undefined argument");
		}
		break;
	    case OM_MOP_LEAF:
	    case OM_MOP_NEAR:
	    case OM_MOP_PHRASE:
		Assert(false); // Shouldn't have got this far
	}
    } else {
	DebugMsg(" (" << left.get_description() << " " << (int) op <<
		 " " << right.get_description() << ") => ");

	// If sub query has same op, which is OR or AND, add to list rather
	// than makeing sub-node.  Can then optimise the list at search time.
	if(op == OM_MOP_AND || op == OM_MOP_OR) {
	    if(left.op == op && right.op == op) {
		// Both queries have same operation as top
		initialise_from_copy(left);
		vector<OmQueryInternal *>::const_iterator i;
		for(i = right.subqs.begin(); i != right.subqs.end(); i++) {
		    subqs.push_back(new OmQueryInternal(**i));
		}
	    } else if(left.op == op) {
		// Query2 has different operation (or is a leaf)
		initialise_from_copy(left);
		subqs.push_back(new OmQueryInternal(right));
		// the initialise_from_copy will have set our
		// qlen to be just left's
		qlen += right.qlen;
	    } else if(right.op == op) { // left has different operation
		// Query1 has different operation (or is a leaf)
		initialise_from_copy(right);
		subqs.push_back(new OmQueryInternal(left));
		// the initialise_from_copy will have set our
		// qlen to be just right's
		qlen += left.qlen;
	    } else {
		subqs.push_back(new OmQueryInternal(left));
		subqs.push_back(new OmQueryInternal(right));
	    }
	} else {
	    subqs.push_back(new OmQueryInternal(left));
	    subqs.push_back(new OmQueryInternal(right));
	}
	collapse_subqs();
	DebugMsg(get_description() << endl);
    }
}

OmQueryInternal::OmQueryInternal(om_queryop op_,
		 const vector<OmQueryInternal *>::const_iterator qbegin,
		 const vector<OmQueryInternal *>::const_iterator qend,
		 om_termpos window)
	: isdefined(true), isbool(false), op(op_)
{   
    initialise_from_vector(qbegin, qend, window);
    collapse_subqs();
}

OmQueryInternal::OmQueryInternal(om_queryop op_,
		 const vector<om_termname>::const_iterator tbegin,
		 const vector<om_termname>::const_iterator tend,
		 om_termpos window)
	: isdefined(true), isbool(false), op(op_)
{
    vector<OmQueryInternal *> subqueries;
    vector<om_termname>::const_iterator i;
    try {
	for(i = tbegin; i != tend; i++) {
	    subqueries.push_back(new OmQueryInternal(*i));
	}
	initialise_from_vector(subqueries.begin(), subqueries.end(), window);
	collapse_subqs();
    } catch (...) {
	// this code would be in a finally clause if there were one...
	// could also go in a destructor.
	for (vector<OmQueryInternal *>::iterator i=subqueries.begin();
	     i!= subqueries.end();
	     ++i) {
	    delete *i;
	}
	throw;
    };
    // same code as above.
    // FIXME: use a destructor instead.
    for (vector<OmQueryInternal *>::iterator i=subqueries.begin();
	 i!= subqueries.end();
	 ++i) {
	delete *i;
    }
}

OmQueryInternal::~OmQueryInternal()
{
    vector<OmQueryInternal *>::const_iterator i;
    for(i = subqs.begin(); i != subqs.end(); i++) {
	delete *i;
    }
    subqs.clear();
}

// Copy an OmQueryInternal object into self
void
OmQueryInternal::initialise_from_copy(const OmQueryInternal &copyme)
{
    isdefined = copyme.isdefined;
    isbool = copyme.isbool;
    op = copyme.op;
    qlen = copyme.qlen;
    if(op == OM_MOP_LEAF) {
	tname = copyme.tname;
	term_pos = copyme.term_pos;
	wqf = copyme.wqf;
    } else {
	vector<OmQueryInternal *>::const_iterator i;
	for(i = subqs.begin(); i != subqs.end(); i++) {
	    delete *i;
	}
	subqs.clear();
	for(i = copyme.subqs.begin(); i != copyme.subqs.end(); i++) {
	    subqs.push_back(new OmQueryInternal(**i));
	}
    }
}

void
OmQueryInternal::initialise_from_vector(
			const vector<OmQueryInternal *>::const_iterator qbegin,
			const vector<OmQueryInternal *>::const_iterator qend,
                        om_termpos window_)
{
    bool merge_ok = false; // set if merging with subqueries is valid
    switch (op) {
	case OM_MOP_AND:
	case OM_MOP_OR:
	    if (window_ != 0)
		throw OmInvalidArgumentError("window parameter not valid for AND or OR");
	    merge_ok = true;
	    break;
	case OM_MOP_NEAR:
	case OM_MOP_PHRASE:
	    break;
	default:
	    throw OmInvalidArgumentError("Vector query op must be AND, OR, NEAR, or PHRASE");
    }
    // FIXME: if NEAR/PHRASE and window_ == 0 default to number of subqueries
    window = window_;
    qlen = 0;

    subquery_list::const_iterator i;
    // reject any attempt to make up a composite query when any sub-query
    // is a pure boolean query.  FIXME: ought to handle the different
    // operators specially.
    for (i = qbegin; i != qend; ++i) {
	if ((*i)->isbool) {
	    throw OmInvalidArgumentError("Only the top-level query can be pure boolean");
	}
    }
    
    try {
	for (i = qbegin; i != qend; i++) {
	    if((*i)->isdefined) {
		/* if the subqueries have the same operator, then we
		 * merge them in, rather than just adding the query.
		 * There's no need to recurse any deeper, since the
		 * sub-queries will all have gone through this process
		 * themselves already.
		 */
		if (merge_ok && (*i)->op == op) {
		    for (subquery_list::const_iterator j = (*i)->subqs.begin();
			 j != (*i)->subqs.end();
			 ++j) {
			subqs.push_back(new OmQueryInternal(**j));
		    }
		} else {
		    // sub-sub query has different op, just add
		    // it in.
		    subqs.push_back(new OmQueryInternal(**i));
		}
		qlen += (*i)->qlen;
	    }
	}
    } catch (...) {
	for (subquery_list::iterator i=subqs.begin();
	     i != subqs.end();
	     ++i) {
	    delete *i;
	}
        throw;
    }

    if(subqs.size() == 0) {
	isdefined = false;
    } else if(subqs.size() == 1) {
	// Should just have copied into self
	OmQueryInternal * copyme = *(subqs.begin());
	subqs.clear();
	initialise_from_copy(*copyme);
	delete copyme;
    }
}

struct Collapse_PosNameLess {
    bool operator()(const pair<om_termpos, om_termname> &left,
		    const pair<om_termpos, om_termname> &right) {
	if (left.first != right.first) {
	    return left.first < right.first;
	} else {
	    return left.second < right.second;
	}
    }
};

void OmQueryInternal::collapse_subqs()
{
    // We must have more than one query item for collapsing to make sense
    // For the moment, we only collapse ORs and ANDs.
    if ((subqs.size() > 1) && ((op == OM_MOP_OR) || (op == OM_MOP_AND))) {
	typedef map<pair<om_termpos, om_termname>,
	            OmQueryInternal *,
		    Collapse_PosNameLess> subqtable;

	subqtable sqtab;
	subquery_list::iterator sq = subqs.begin();
	while (sq != subqs.end()) {
	    if ((*sq)->op == OM_MOP_LEAF) {
		subqtable::key_type key(make_pair((*sq)->term_pos,
						  (*sq)->tname));
		subqtable::iterator s = sqtab.find(key);
		if (s == sqtab.end()) {
		    sqtab[key] = *sq;
		    ++sq;
		} else {
		    s->second->wqf += (*sq)->wqf;
		    // rather than incrementing, delete the current
		    // element, as it has been merged into the other
		    // equivalent term.
		    delete (*sq);
	 	    sq = subqs.erase(sq);
		}
	    } else {
		++sq;
	    }
	}

	// We should never lose all the subqueries
	Assert(subqs.size() > 0);

	// If we have only one subquery, move it into ourself
	if (subqs.size() == 1) {
	    OmQueryInternal *only_child = *subqs.begin();
	    subqs.clear();
	    initialise_from_copy(*only_child);
	    delete only_child;
	}
    }
}
