/* document.h: class with document data
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

#ifndef OM_HGUARD_DOCUMENT_H
#define OM_HGUARD_DOCUMENT_H

#include <xapian/base.h>
#include <xapian/types.h>
#include "termlist.h"
#include "documentterm.h"
#include <map>
#include <string>

using namespace std;

class Xapian::Database::Internal;

/// A document in the database, possibly plus modifications.
class Xapian::Document::Internal : public Xapian::Internal::RefCntBase {
    friend class Xapian::ValueIterator;
    public:
	/// Type to store values in.
	typedef map<om_valueno, string> document_values;

	/// Type to store terms in.
	typedef map<string, OmDocumentTerm> document_terms;

    private:
        // Prevent copying
        Internal(const Internal &);
        Internal & operator=(const Internal &);

	/// The database this document is in.
	const Xapian::Database::Internal *database;

	bool data_here;
	mutable bool values_here; // FIXME mutable is a hack
	mutable bool terms_here;

	/// The (user defined) data associated with this document.
	string data;

	/// The values associated with this document.
	mutable document_values values; // FIXME mutable is a hack

	/// The value numbers, for use by ValueIterators.
	mutable vector<om_valueno> value_nos; // FIXME mutable is a hack

	/// The terms (and their frequencies and positions) in this document.
	mutable document_terms terms;

    protected:
	/// The document ID of the document in that database.
	//
	//  If we're using multiple databases together this may not be the
	//  same as the docid in the combined database.
	om_docid did;

    private:
	// Functions for backend to implement
	virtual string do_get_value(om_valueno /*valueno*/) const { return ""; }
	virtual map<om_valueno, string> do_get_all_values() const {
	    map<om_valueno, string> none;
	    return none;
	}
	virtual string do_get_data() const { return ""; }

    public:
	/** Get value by value number.
	 *
	 *  Values are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of values, each of which
	 *  having a different valueid.  Duplicate values with the same valueid
	 *  are not supported in a single document.
	 *
	 *  Value numbers are any integer >= 0, but particular database
	 *  backends may impose a more restrictive range than that.
	 *
	 *  @param valueid  The value number requested.
	 *
	 *  @return       A string containing the specified value.  If
	 *  the value is not present in this document, the value's value will
	 *  be a zero length string
	 */
	string get_value(om_valueno valueid) const;

	/** Get all values for this document
	 *
	 *  Values are quickly accessible fields, for use during the match
	 *  operation.  Each document may have a set of values, each of which
	 *  having a different valueid.  Duplicate values with the same valueid
	 *  are not supported in a single document.
	 *
	 *  @return   A map of strings containing all the values.
	 */
	map<om_valueno, string> get_all_values() const;

	om_valueno values_count() const;
	void add_value(om_valueno, const string &);
	void remove_value(om_valueno);
	void clear_values();
	void add_posting(const string &, om_termpos, om_termcount);
	void add_term_nopos(const string &, om_termcount);
	void remove_posting(const string &, om_termpos, om_termcount);
	void remove_term(const string &);
	void clear_terms();
	om_termcount termlist_count() const;

	/** Get data stored in document.
	 *
	 *  This is a general piece of data associated with a document, and
	 *  will typically be used to store such information as text to be
	 *  displayed in the result list, and a pointer in some form
	 *  (eg, URL) to the full text of the document.
	 *
	 *  This operation can be expensive, and shouldn't normally be used
	 *  during the match operation (such as in a match decider functor):
	 *  use a value instead, if at all possible.
	 *
	 *  @return       An string containing the data for this document.
	 */
	string get_data() const;	

	void set_data(const string &);

	/** Open a term list.
	 *
	 *  This is a list of all the terms contained by a given document.
	 *
	 *  @return       A pointer to the newly created term list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	TermList * open_term_list() const;

	void need_values() const;
	void need_terms() const;

	/** Returns a string representing the object.
	 *  Introspection method.
	 */
	string get_description() const;

	/** Constructor.
	 *
	 *  In derived classes, this will typically be a private method, and
	 *  only be called by database objects of the corresponding type.
	 */
	Internal(const Xapian::Database::Internal *database_, om_docid did_)
	    : database(database_), data_here(false), values_here(false),
	      terms_here(false), did(did_) { }

        Internal()
	    : database(0), data_here(false), values_here(false),
	      terms_here(false) { }

	/** Destructor.
	 *
	 *  Note that the database object which created this document must
	 *  still exist at the time this is called.
	 */
	virtual ~Internal() { }
};

#endif  // OM_HGUARD_DOCUMENT_H
