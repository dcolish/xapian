/* da_database.h: C++ class definition for DA access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_DA_DATABASE_H
#define OM_HGUARD_DA_DATABASE_H

#include <stdio.h>
#include <map>
#include <vector>
#include <errno.h>
#include "document.h"

#include "utils.h"
#include "omdebug.h"

#include "leafpostlist.h"
#include "termlist.h"
#include "database.h"

// FIXME - try and sort it out so that we don't need to include this.
#include "daread.h"

/** A posting list for a DA Database */
class DAPostList : public LeafPostList {
    friend class DADatabase;
    private:
	struct DA_postings * postlist;
	om_docid  currdoc;

	string tname;
	om_doccount termfreq;

	RefCntPtr<const DADatabase> this_db; // Just used to keep a reference

	DAPostList(const string & tname_,
		   struct DA_postings * postlist_,
		   om_doccount termfreq_,
		   RefCntPtr<const DADatabase> this_db_);
    public:
	~DAPostList();

	om_doccount get_termfreq() const;

	om_docid  get_docid() const;     // Gets current docid
	om_doclength get_doclength() const; // Get length of current document
        om_termcount get_wdf() const;    // Within Document Frequency
	PositionList *read_position_list(); // Gets positions
	PositionList * open_position_list() const; // Gets positions
	PostList *next(om_weight w_min);          // Moves to next docid
	PostList *skip_to(om_docid did, om_weight w_min);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

inline om_doccount
DAPostList::get_termfreq() const
{
    DEBUGLINE(DB, "DAPostList::get_termfreq() = " << termfreq);
    return termfreq;
}

inline om_docid
DAPostList::get_docid() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    DEBUGLINE(DB, "DAPostList::get_docid() = " << currdoc);
    return currdoc;
}

inline om_doclength
DAPostList::get_doclength() const
{
    // FIXME: return database->get_doclength()
    DEBUGLINE(DB, "DAPostList::get_doclength() = " << 1.0);
    return 1;
}

inline om_termcount
DAPostList::get_wdf() const
{
    DEBUGLINE(DB, "DAPostList::get_wdf() = " << postlist->wdf);
    return postlist->wdf;
}

inline bool
DAPostList::at_end() const
{
    DEBUGCALL(DB, bool, "DAPostList::at_end()", "");
    Assert(currdoc != 0);
    RETURN(currdoc == MAXINT);
}

inline std::string
DAPostList::get_description() const
{
    return tname + ":" + om_tostring(termfreq);
}



class DATermListItem {
    public:
	string tname;
	om_termcount wdf;
	om_doccount termfreq;

	DATermListItem(string tname_, om_termcount wdf_,
		       om_doccount termfreq_)
		: tname(tname_), wdf(wdf_), termfreq(termfreq_)
	{ }
};

/** A term list for a DA Database */
class DATermList : public LeafTermList {
    friend class DADatabase;
    private:
	std::vector<DATermListItem>::iterator pos;
	std::vector<DATermListItem> terms;
	bool have_started;
	om_doccount dbsize;

	RefCntPtr<const DADatabase> this_db;

	DATermList(struct termvec *tv, om_doccount dbsize_,
		   RefCntPtr<const DADatabase> this_db_);
    public:
	om_termcount get_approx_size() const;

	OmExpandBits get_weighting() const; // Gets weight info of current term
	string get_termname() const;
	om_termcount get_wdf() const; // Number of occurences of term in current doc
	om_doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline om_termcount DATermList::get_approx_size() const
{
    return terms.size();
}

inline string DATermList::get_termname() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->tname;
}

inline om_termcount DATermList::get_wdf() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->wdf;
}

inline om_doccount DATermList::get_termfreq() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->termfreq;
}

inline TermList * DATermList::next()
{
    if(have_started) {
	Assert(!at_end());
	pos++;
    } else {
	have_started = true;
    }
    return NULL;
}

inline bool DATermList::at_end() const
{
    DEBUGCALL(DB, bool, "DATermList::at_end()", "");
    Assert(have_started);
    RETURN(pos == terms.end());
}




class DATerm : public RefCntBase {
    friend class DADatabase;
    private:
	DATerm(struct DA_term_info * ti_,
	       string tname_,
	       struct DA_file * DA_t_ = NULL);
        struct DA_term_info * get_ti() const;

	mutable bool terminfo_initialised;
        mutable struct DA_term_info ti;
        mutable struct DA_file * DA_t;
    public:
	string tname;
};

inline
DATerm::DATerm(struct DA_term_info * ti_,
	       string tname_,
	       struct DA_file * DA_t_)
	: terminfo_initialised(false)
{
    if (ti_) {
	ti = *ti_;
	terminfo_initialised = true;
    }
    tname = tname_;
    DA_t = DA_t_;
}

inline struct DA_term_info *
DATerm::get_ti() const
{
    if (!terminfo_initialised) {
	DEBUGLINE(DB, "Getting terminfo");
	std::string::size_type len = tname.length();
	if(len > 255) abort();
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw std::bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	int found = DA_term(k, &ti, DA_t);
	free(k);

	if(found == 0) abort();
	terminfo_initialised = true;
    }
    return &ti;
}

/** A DA Database */
class DADatabase : public Database {
    friend class DatabaseBuilder;
    friend class DADocument;
    private:
	struct DA_file * DA_r;
	struct DA_file * DA_t;

	FILE * valuefile;

	mutable std::map<string, RefCntPtr<const DATerm> > termmap;

	bool heavy_duty;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);

	// Look up term in database
	RefCntPtr<const DATerm> term_lookup(const string & tname) const;

	// Get a record
	struct record * get_record(om_docid did) const;

	/** Get a value from valuefile (will return empty value if valuefile
	 *  not open.
	 */
	string get_value(om_docid did, om_valueno valueid) const;

	/** Internal method for opening postlists.
	 */
	LeafPostList * open_post_list_internal(const string & tname) const;

    public:
	/** Create and open a DA database.
	 *
	 *  @exception Xapian::OpeningError thrown if database can't be opened.
	 *
	 *  @param filename_r Filename of the record file (usually called "R").
	 *  @param filename_t Filename of the term file (usually called "T").
	 *  @param filename_v Filename of the value file (or "" if none).
	 *  @param heavy_duty_ True if lengths are 3 bytes, false if they're 2.
	 */
	DADatabase(const string &filename_r, const string &filename_t,
		   const string &filename_v, bool heavy_duty_);

	~DADatabase();

	/// Get the database size.
	om_doccount  get_doccount() const;
	/// Get the average length of a document in the database.
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const string & tname) const;
	om_termcount get_collection_freq(const string & /*tname*/) const {
	    throw Xapian::UnimplementedError(
		"DADatabase::get_collection_freq() not implemented: data not stored in database.");
	}
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	Document * open_document(om_docid did, bool lazy = false) const;
	PositionList * open_position_list(om_docid did,
					  const string & tname) const;
	TermList * open_allterms() const;

	//@{
	/** DADatabase is a readonly database type, and thus this method is
	 *  not supported: if called an exception will be thrown.
	 */
	void do_begin_session() {
	    throw Xapian::UnimplementedError(
		"DADatabase::begin_session() not implemented: readonly database type");
	}

	void do_end_session() {
	    throw Xapian::UnimplementedError(
		"DADatabase::do_end_session() not implemented: readonly database type");
	}

	void do_flush() {
	    throw Xapian::UnimplementedError(
		"DADatabase::flush() not implemented: readonly database type");
	}

	void do_begin_transaction() {
	    throw Xapian::UnimplementedError(
		"DADatabase::begin_transaction() not implemented: readonly database type");
	}

	void do_commit_transaction() {
	    throw Xapian::UnimplementedError(
		"DADatabase::commit_transaction() not implemented: readonly database type");
	}

	void do_cancel_transaction() {
	    throw Xapian::UnimplementedError(
		"DADatabase::cancel_transaction() not implemented: readonly database type");
	}

	om_docid do_add_document(const OmDocument & /*document*/) {
	    throw Xapian::UnimplementedError(
		"DADatabase::add_document() not implemented: readonly database type");
	}

	void do_delete_document(om_docid /*did*/) {
	    throw Xapian::UnimplementedError(
		"DADatabase::delete_document() not implemented: readonly database type");
	}

	void do_replace_document(om_docid /*did*/, const OmDocument & /*document*/) {
	    throw Xapian::UnimplementedError(
		"DADatabase::replace_document() not implemented: readonly database type");
	}

	//@}

};

#endif /* OM_HGUARD_DA_DATABASE_H */
