/* omqueryinternal.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_OMQUERYINTERNAL_H
#define OM_HGUARD_OMQUERYINTERNAL_H

#include <xapian/types.h>
#include "om/omenquire.h"
#include "om/omquery.h"
#include <string>
#include <vector>

#ifdef USE_DELETER_VECTOR
#include "deleter_vector.h"
#endif

using namespace std;

class MultiMatch;
class LocalSubMatch;

///////////////////////////////////////////////////////////////////
// OmQuery::Internal class
// =====================

/// Internal class, implementing most of OmQuery
class OmQuery::Internal {
    friend class MultiMatch;
    friend class LocalSubMatch;
    friend class SortPosName;
    public:
        static const int OP_LEAF = -1;
        static const int OP_UNDEF = -2;

	/// The container type for storing pointers to subqueries
#ifdef USE_DELETER_VECTOR
	typedef deleter_vector<OmQuery::Internal *> subquery_list;
#else
	typedef vector<OmQuery::Internal *> subquery_list;
#endif

	/// Type storing the operation
	typedef int op_t;

    private:
	/// Operation to be performed at this node
	op_t op;

	/// Sub queries on which to perform operation
	subquery_list subqs;
	
	/// Length of query
	om_termcount qlen;

	/** How close terms must be for NEAR or PHRASE.
	 *  To match, all terms must occur in a window of this size.
	 */
	om_termpos window;

	/** What's the cutoff for *_CUTOFF queries.
	 */
	double cutoff;

	/** How many terms to select for the elite set, for ELITE_SET queries.
	 */
	om_termcount elite_set_size;

	/// Term that this node represents - leaf node only
	string tname;

	/// Position in query of this term - leaf node only
	om_termpos term_pos;

	/// Within query frequency of this term - leaf node only
	om_termcount wqf;

	/** swap the contents of this with another OmQuery::Internal,
	 *  in a way which is guaranteed not to throw.  This is
	 *  used with the assignment operator to make it exception
	 *  safe.
	 *  It's important to adjust swap with any addition of
	 *  member variables!
	 */
	void swap(OmQuery::Internal &other);

	/// Copy another OmQuery::Internal into self.
	void initialise_from_copy(const OmQuery::Internal & copyme);

        void accumulate_terms(
	    vector<pair<string, om_termpos> > &terms) const;

	/** Simplify the query.
	 *  For example, an AND query with only one subquery would become the
	 *  subquery itself.
	 */
	void simplify_query();

	/** Preliminary checks that query is valid. (eg, has correct number of
	 *  sub queries.) Throw an exception if not.  This is initially called
	 *  on the query before any simplifications have been made.
	 */
	void prevalidate_query() const;

	/** Check query is well formed.
	 *  Throw an exception if not.
	 *  This is called at construction time, so doesn't check parameters
	 *  which must be set separately.
	 *  This performs all checks in prevalidate_query(), and some others
	 *  as well.
	 */
	void validate_query() const;

	/** Get a string describing the given query type.
	 */
	static string get_op_name(OmQuery::Internal::op_t op);

	/** Collapse the subqueryies together if appropriate.
	 */
	void collapse_subqs();

	/** Flatten a query structure, by changing, for example,
	 *  "A NEAR (B AND C)" to "(A NEAR B) AND (A NEAR C)"
	 */
	void flatten_subqs();

    public:
	/** Copy constructor. */
	Internal(const OmQuery::Internal & copyme);

	/** Assignment. */
	void operator=(const OmQuery::Internal & copyme);

	/** A query consisting of a single term. */
	Internal(const string & tname_, om_termcount wqf_ = 1,
		 om_termpos term_pos_ = 0);

	/** Create internals given only the operator. */
	Internal(op_t op_);

	/** Default constructor: makes an undefined query which can't be used
	 *  directly.  Such queries should be thought of as placeholders:
	 *  they are provided merely for convenience.
	 *
	 *  An exception will be thrown if an attempt is made to use or run
	 *  an undefined query.
	 */
	Internal();

	/** Destructor. */
	~Internal();

	/** Add a subquery.
	 */
	void add_subquery(const OmQuery::Internal & subq);

	/** Finish off the construction.
	 */
	void end_construction();

	/** Return a string in an easily parsed form
	 *  which contains all the information in a query.
	 */
	string serialise() const;

	/** Returns a string representing the query.
	 * Introspection method.
	 */
	string get_description() const;

	/** Set window for NEAR or PHRASE queries */
	void set_window(om_termpos window);

	/** Set cutoff for *_CUTOFF queries */
	void set_cutoff(double cutoff);

	/** Set elite set size */
	void set_elite_set_size(om_termcount size);

	/** Get the length of the query, used by some ranking formulae.
	 *  This value is calculated automatically, but may be overridden
	 *  using set_length().
	 */
	om_termcount get_length() const { return qlen; }

	/** Set the length of the query.
	 *  This overrides the automatically calculated value, which may
	 *  be desirable in some situations.
	 *  Returns the old value of the query length.
	 */
	om_termcount set_length(om_termcount qlen_);

	/** Return an iterator over all the terms in the query,
	 *  in order of termpos.  If multiple terms have the same term
	 *  position, their order is unspecified.  Duplicates (same term and
	 *  termpos) will be removed.
	 */
	Xapian::TermIterator get_terms() const;
	
	/// Test is the query is empty (i.e. was set using OmQuery() or with
	//  an empty iterator ctor)
	bool is_empty() const { return op == OP_UNDEF; }
};

#endif // OM_HGUARD_OMQUERYINTERNAL_H
