%module xapian

%{
/* xapian.i: the Xapian scripting interface.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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
#undef list
#include "om/om.h"
#include <string>
#include <vector>

enum om_queryop_values {
    OM_MOP_AND = OmQuery::OP_AND,
    OM_MOP_OR = OmQuery::OP_OR,
    OM_MOP_AND_NOT = OmQuery::OP_AND_NOT,
    OM_MOP_XOR = OmQuery::OP_XOR,
    OM_MOP_AND_MAYBE = OmQuery::OP_AND_MAYBE,
    OM_MOP_FILTER = OmQuery::OP_FILTER,
    OM_MOP_NEAR = OmQuery::OP_NEAR,
    OM_MOP_PHRASE = OmQuery::OP_PHRASE
};

typedef OmQuery::op om_queryop;
%}
%include "om_util.i"
%include "omstem.i"
%include "omtypes.i"

enum om_queryop {
    OM_MOP_AND,
    OM_MOP_OR,
    OM_MOP_AND_NOT,
    OM_MOP_XOR,
    OM_MOP_AND_MAYBE,
    OM_MOP_FILTER,
    OM_MOP_NEAR,
    OM_MOP_PHRASE
};

class OmQuery {
    public:
        string get_description() const;
	// 
        /** Constructs a query consisting of single term
         *  @param tname  The name of the term.
         *  @param wqf  Within-?-frequency.
         *  @param term_pos  Position of term related to other terms but there aren't any anyway!
        */
        %name(OmQueryTerm) OmQuery(const string &tname,
				   om_termcount wqf = 1,
				   om_termpos term_pos = 0);
        /** Constructs a new query object */
	%name(OmQueryNull) OmQuery();

	%addmethods {
            /** Constructs a query from a set of queries merged with the specified operator */
	    %name (OmQueryList) OmQuery(om_queryop op,
	    	    const vector<OmQuery *> *subqs,
		    om_termpos window = 0) {
		if ((subqs->size() == 2) && (window == 0)) {
		    return new OmQuery(op, *(*subqs)[0], *(*subqs)[1]);
		} else {
		    OmQuery * query=new OmQuery(op, subqs->begin(),subqs->end());
		    query->set_window(window);
		    return query;
		}
	    }
	}

	~OmQuery();

	string get_description();
	bool is_empty() const;
	void set_window(om_termpos window);
	void set_length(om_termpos qlen);
	void set_elite_set_size(om_termpos size);
	void set_cutoff(om_weight cutoff);
	om_termcount get_length() const;
	om_termcount set_length(om_termcount qlen_);
//	om_termname_list get_terms() const;
};

// TODO: OmMatchDecider

// TODO: OmExpandDecider

class OmRSet {
    public:
	OmRSet();

	// TODO: set<om_docid> items;
	void add_document(om_docid did);
	void remove_document(om_docid did);
};

class OmESet {
    public:
	~OmESet();
	%readonly
        string get_description() const;
	om_termcount get_ebound() const;
	om_termcount size() const;
	/* Each language-specific part should include something like:
	 * %addmethods OmESet {
	 *     %readonly
	 *     LangListType items;
	 * }
	 * and define LangListType OmMSet_items_get(OmMSet *)
	 */
	%readwrite
};

#if defined(NOTDEFINED)
%typedef OmBatchEnquire::batch_result batch_result;
%typedef OmBatchEnquire::mset_batch mset_batch;
%typedef OmBatchEnquire::query_desc query_desc;
%typedef OmBatchEnquire::query_batch query_batch;

class batch_result {
    public:
	OmMSet value() const;
	bool is_valid() const;
};

class mset_batch {
    public:
	/*  Each language needs to define appropriate methods
	 *  to get at the results (using %addmethods).
	 */
};

class OmBatchEnquire {
    public:
        OmBatchEnquire(const OmDatabase &databases);
        ~OmBatchEnquire();

	void set_queries(const query_batch &queries_);

	mset_batch get_msets() const;

	const OmDocument get_document(om_docid did) const;

	om_termname_list get_matching_terms(om_docid did) const;
};
#endif

#if defined(NOTDEFINED)
class OmSettings {
    public:
	OmSettings();
	~OmSettings();

	void set_value(const string &key, const string &value);

	string get_value(const string &key) const;
	// TODO: make this look like a Dict/hash/whatever?
};
#endif

#if defined(NOTDEFINED)
struct OmDocumentTerm {
    OmDocumentTerm(const string & tname_, om_termpos tpos = 0);

    string tname;
    om_termcount wdf;

    //TODO: sort out access to term_positions
    typedef vector<om_termpos> term_positions;
    term_positions positions;
    om_doccount termfreq;
    void add_posting(om_termpos tpos = 0);
};
#endif

class OmDocument {
  public:
    ~OmDocument();

    // OmKey and OmData are both strings as far as scripting languages
    // see them.
    OmKey get_key(om_keyno key) const;
    OmData get_data() const;

    %addmethods {
        OmDocumentContents() {
	    return new OmDocumentContents();
	};

        void set_data(string data_) {
	    self->set_data(data_);
	}
    }

    /** Type to store keys in. */
//    typedef map<om_keyno, OmKey> document_keys;

    /** The keys associated with this document. */
//    document_keys keys;

    %addmethods {
	void add_key(int keyno, string value) {
	    self->add_key(keyno, OmKey(value));
	}
    }

    // TODO: sort out access to the maps somehow.
    /** Type to store terms in. */
//    typedef map<string, OmDocumentTerm> document_terms;

    /** The terms (and their frequencies and positions) in this document. */
//    document_terms terms;

    /** Add an occurrence of a term to the document.
     *
     *  Multiple occurrences of the term at the same position are represented
     *  only once in the positional information, but do increase the wdf.
     *
     *  @param tname  The name of the term.
     *  @param tpos   The position of the term.
     */
    void add_posting(const string & tname, om_termpos tpos = 0);
};

class OmDatabase {
    public:
	OmDatabase(const OmSettings &params);
	virtual ~OmDatabase();

	%name(add_dbargs) void add_database(const OmSettings &params);
	
	void add_database(const OmDatabase & database);
};

class OmWritableDatabase : public OmDatabase {
    public:
	OmWritableDatabase(const OmSettings & params);
	virtual ~OmWritableDatabase();

	void flush();

	void begin_transaction();
	void commit_transaction();
	void cancel_transaction();

	om_docid add_document(const OmDocument & document);
	void delete_document(om_docid did);
	void replace_document(om_docid did,
			      const OmDocument & document);

	OmDocument get_document(om_docid did);
	string get_description() const;
};

class OmEnquire {
    public:
        OmEnquire(const OmDatabase &databases);
	~OmEnquire();

	void set_query(const OmQuery &query);

	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset = 0,
			const OmSettings *moptions = 0,
			const OmMatchDecider *mdecider = 0);

	OmESet get_eset(om_termcount maxitems,
			const OmRSet &omrset,
			const OmSettings *eoptions = 0,
			const OmExpandDecider *edecider = 0) const;

//	om_termname_list get_matching_terms(om_docid did);
};

class OmMSet {
    public:
	OmMSet();
	~OmMSet();

	int convert_to_percent(om_weight wt) const;
//	int convert_to_percent(const OmMSetItem & item) const;
	om_weight get_termfreq(string tname) const;
	om_doccount get_termweight(string tname) const;
//	om_doccount get_firstitem();
	om_weight get_max_possible();
	om_weight get_max_attained();

	string get_description();

	%readonly
	/* Each language-specific part should include something like:
	 * %addmethods OmMSet {
	 *     %readonly
	 *     LangListType items;
	 * }
	 * and define LangListType OmMSet_items_get(OmMSet *)
	 */


	%readwrite
};
