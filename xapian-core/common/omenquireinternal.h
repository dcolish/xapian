/* omenquireinternal.h: Internals
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
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

#ifndef OM_HGUARD_OMENQUIREINTERNAL_H
#define OM_HGUARD_OMENQUIREINTERNAL_H

#include <xapian/enquire.h>
#include "refcnt.h"
#include <algorithm>
#include <math.h>
#include <map>
#include <set>

using namespace std;

class OmExpand;

namespace Xapian {

class ErrorHandler;
class TermIterator;

namespace Internal {
 
/** An item in the ESet.
 *  This item contains the termname, and the weight calculated for
 *  the document.
 */
class ESetItem {
    public:
	ESetItem(om_weight wt_, string tname_) : wt(wt_), tname(tname_) { }
	/// Weight calculated.
	om_weight wt;
	/// Term suggested.
	string tname;
	
	/** Returns a string representing the eset item.
	 *  Introspection method.
	 */
	string get_description() const;
};

/** An item resulting from a query.
 *  This item contains the document id, and the weight calculated for
 *  the document.
 */
class MSetItem {
    public:
	MSetItem(om_weight wt_, om_docid did_) 
		: wt(wt_), did(did_), collapse_count(0) {}

	MSetItem(om_weight wt_, om_docid did_, const string &key_)
		: wt(wt_), did(did_), collapse_key(key_), collapse_count(0) {}

	/** Weight calculated. */
	om_weight wt;

	/** Document id. */
	om_docid did;

	/** Value which was used to collapse upon.
	 *
	 *  If the collapse option is not being used, this will always
	 *  have a null value.
	 *
	 *  If the collapse option is in use, this will contain the collapse
	 *  key's value for this particular item.  If the key is not present
	 *  for this item, the value will be a null string.  Only one instance
	 *  of each key value (apart from the null string) will be present in
	 *  the items in the returned Xapian::MSet.
	 */
	string collapse_key;

	/** Count of collapses done on collapse_key so far
	 *
	 * This is normally 0, and goes up for each collapse done
	 * It is not neccessarily an indication of how many collapses
	 * might be done if an exhaustive match was done
	 */
	 om_doccount collapse_count;

	/** For use by match_sort_key option - FIXME: document if this stays */
	/* FIXME: this being mutable is a gross hack */
	/* FIXME: why not just cache the OmDocument here!?! */
	mutable string sort_key;

	/** Returns a string representing the mset item.
	 *  Introspection method.
	 */
	string get_description() const;
};

}

/** Internals of enquire system.
 *  This allows the implementation of Xapian::Enquire to be hidden and reference
 *  counted.
 */
class Enquire::Internal : public Xapian::Internal::RefCntBase {
    private:
	/// The database which this enquire object uses.
	const OmDatabase db;

	/** The user's query.
	 *  This may need to be mutable in future so that it can be
	 *  replaced by an optimised version.
	 */
	Query * query;

	/** Calculate the matching terms.
	 *  This method does the work for get_matching_terms().
	 */
	TermIterator calc_matching_terms(om_docid did) const;

	/// Copy not allowed
	Internal(const Internal &);
	/// Assignment not allowed
	void operator=(const Internal &);

    public:
	om_valueno collapse_key;

	bool sort_forward;

	percent percent_cutoff;

	om_weight weight_cutoff;

	om_valueno sort_key;
	int sort_bands;

	time_t bias_halflife;
	om_weight bias_weight;

	/** The error handler, if set.  (0 if not set).
	 */
	ErrorHandler * errorhandler;

	map<string, const OmMatchDecider *> mdecider_map;

	mutable OmWeight * weight; // mutable so get_mset can set default

	Internal(const OmDatabase &databases, ErrorHandler * errorhandler_);
	~Internal();

	/** Request a document from the database.
	 */
	void request_doc(const Xapian::Internal::MSetItem &item) const;

	/** Read a previously requested document from the database.
	 */
	OmDocument read_doc(const Xapian::Internal::MSetItem &item) const;

	void set_query(const Query & query_);
	const Query & get_query();
	Xapian::MSet get_mset(om_doccount first, om_doccount maxitems,
			const OmRSet *omrset, 
			const OmMatchDecider *mdecider) const;
	Xapian::ESet get_eset(om_termcount maxitems, const OmRSet & omrset, int flags,
			double k, const ExpandDecider *edecider) const;

	TermIterator get_matching_terms(om_docid did) const;
	TermIterator get_matching_terms(const Xapian::MSetIterator &it) const;

	void register_match_decider(const string &name,
				    const OmMatchDecider *mdecider);
    
	string get_description() const;
};

class MSet::Internal : public Xapian::Internal::RefCntBase {
    private:
	/// Factor to multiply weights by to convert them to percentages.
	double percent_factor;

	/// The set of documents which have been requested but not yet
	/// collected.
	mutable set<om_doccount> requested_docs;

	/// Cache of documents, indexed by MSet index.
	mutable map<om_doccount, OmDocument> indexeddocs;

	/// Read and cache the documents so far requested.
	void read_docs() const;

	/// Copy not allowed
	Internal(const Internal &);
	/// Assignment not allowed
	void operator=(const Internal &);

    public:
	/// Xapian::Enquire reference, for getting documents.
	Xapian::Internal::RefCntPtr<const Enquire::Internal> enquire;

	/** A structure containing the term frequency and weight for a
	 *  given query term.
	 */
	struct TermFreqAndWeight {
	    om_doccount termfreq;
	    om_weight termweight;
	};

	/** The term frequencies and weights returned by the match process.
	 * 
	 *  This map will contain information for each term which was in
	 *  the query.
	 */
	map<string, TermFreqAndWeight> termfreqandwts;

	/// A list of items comprising the (selected part of the) mset.
	vector<Xapian::Internal::MSetItem> items;

	/// Rank of first item in Mset.
	om_doccount firstitem;

	om_doccount matches_lower_bound;

	om_doccount matches_estimated;

	om_doccount matches_upper_bound;

	om_weight max_possible;

	om_weight max_attained;

	Internal()
		: percent_factor(0),
		  firstitem(0),
		  matches_lower_bound(0),
		  matches_estimated(0),
		  matches_upper_bound(0),
		  max_possible(0),
		  max_attained(0) {}

	Internal(om_doccount firstitem_,
	     om_doccount matches_upper_bound_,
	     om_doccount matches_lower_bound_,
	     om_doccount matches_estimated_,
	     om_weight max_possible_,
	     om_weight max_attained_,
	     const vector<Xapian::Internal::MSetItem> &items_,
	     const map<string, TermFreqAndWeight> &termfreqandwts_,
	     om_weight percent_factor_)
		: percent_factor(percent_factor_),
		  termfreqandwts(termfreqandwts_),
		  items(items_),
		  firstitem(firstitem_),
		  matches_lower_bound(matches_lower_bound_),
		  matches_estimated(matches_estimated_),
		  matches_upper_bound(matches_upper_bound_),
		  max_possible(max_possible_),
		  max_attained(max_attained_) {}

	/// get a document by index in mset, via the cache.
	OmDocument get_doc_by_index(om_doccount index) const;

	/// Converts a weight to a percentage weight
	percent convert_to_percent_internal(om_weight wt) const;

	/** Returns a string representing the mset.
	 *  Introspection method.
	 */
	string get_description() const;

	/** Fetch items specified into the document cache.
	 */
	void fetch_items(om_doccount first, om_doccount last) const;
};

class ESet::Internal {
    friend class ESet;
    friend class ESetIterator;
    friend class OmExpand;
    private:
	/// A list of items comprising the (selected part of the) eset.
	vector<Xapian::Internal::ESetItem> items;

	/** A lower bound on the number of terms which are in the full
	 *  set of results of the expand.  This will be greater than or
	 *  equal to items.size()
	 */
	om_termcount ebound;

    public:
	Internal() : ebound(0) {}

	/** Returns a string representing the eset.
	 *  Introspection method.
	 */
	string get_description() const;
};

}

class RSet;

class OmRSet::Internal {
    friend class OmRSet;
    friend class RSet;
    friend class OmExpand;
    friend class MultiMatch;
    friend string omrset_to_string(const OmRSet &omrset);

    private:
	/// Items in the relevance set.
	set<om_docid> items;

    public:
	/** Returns a string representing the rset.
	 *  Introspection method.
	 */
	string get_description() const;
};

#endif // OM_HGUARD_OMENQUIREINTERNAL_H
