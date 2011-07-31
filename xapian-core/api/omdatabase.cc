/* omdatabase.cc: External interface for running queries
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright 2006,2008 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <time.h>
#include "autoptr.h"

#include <xapian/error.h>
#include <xapian/positioniterator.h>
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/unicode.h>

#include "omassert.h"
#include "debuglog.h"
#include "../backends/multi/multi_postlist.h"
#include "../backends/multi/multi_termlist.h"
#include "alltermslist.h"
#include "multialltermslist.h"
#include "multivaluelist.h"
#include "database.h"
#include "../spelling/editdistance.h"
#include "../spelling/extended_edit_distance.h"
#include "ortermlist.h"
#include "noreturn.h"

#include <cstdlib> // For abs().

#include <cstring>
#include <vector>
#include <map>

#include <xapian/unordered_map.h>

#include "../spelling/spelling_corrector.h"
#include "../spelling/spelling_splitter.h"
#include "../spelling/spelling_splitter_new.h"

using namespace std;

XAPIAN_NORETURN(static void docid_zero_invalid());
static void docid_zero_invalid()
{
    throw Xapian::InvalidArgumentError("Document ID 0 is invalid");
}

XAPIAN_NORETURN(static void no_subdatabases());
static void no_subdatabases()
{
    throw Xapian::DocNotFoundError("No subdatabases");
}

namespace Xapian {

Database::Database()
{
    LOGCALL_CTOR(API, "Database", NO_ARGS);
}

Database::Database(Database::Internal *internal_)
{
    LOGCALL_CTOR(API, "Database", internal_);
    Xapian::Internal::RefCntPtr<Database::Internal> newi(internal_);
    internal.push_back(newi);
}

Database::Database(const Database &other)
{
    LOGCALL_CTOR(API, "Database", other);
    internal = other.internal;
}

void
Database::operator=(const Database &other)
{
    LOGCALL_VOID(API, "Database::operator=", other);
    if (this == &other) {
	LOGLINE(API, "Database assigned to itself");
	return;
    }

    internal = other.internal;
}

Database::~Database()
{
    LOGCALL_DTOR(API, "Database");
}

void
Database::reopen()
{
    LOGCALL_VOID(API, "Database::reopen", NO_ARGS);
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	(*i)->reopen();
    }
}

void
Database::close()
{
    LOGCALL_VOID(API, "Database::close", NO_ARGS);
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	(*i)->close();
    }
}

void
Database::add_database(const Database & database)
{
    LOGCALL_VOID(API, "Database::add_database", database);
    if (this == &database) {
	LOGLINE(API, "Database added to itself");
	throw Xapian::InvalidArgumentError("Can't add a Database to itself");
    }
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = database.internal.begin(); i != database.internal.end(); ++i) {
	internal.push_back(*i);
    }
}

PostingIterator
Database::postlist_begin(const string &tname) const
{
    LOGCALL(API, PostingIterator, "Database::postlist_begin", tname);

    // Don't bother checking that the term exists first.  If it does, we
    // just end up doing more work, and if it doesn't, we save very little
    // work.

    // Handle the common case of a single database specially.
    if (internal.size() == 1)
	RETURN(PostingIterator(internal[0]->open_post_list(tname)));

    if (rare(internal.size() == 0))
	RETURN(PostingIterator(NULL));

    vector<LeafPostList *> pls;
    try {
	vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
	for (i = internal.begin(); i != internal.end(); ++i) {
	    pls.push_back((*i)->open_post_list(tname));
	    pls.back()->next();
	}
	Assert(pls.begin() != pls.end());
    } catch (...) {
	vector<LeafPostList *>::iterator i;
	for (i = pls.begin(); i != pls.end(); ++i) {
	    delete *i;
	    *i = 0;
	}
	throw;
    }

    RETURN(PostingIterator(new MultiPostList(pls, *this)));
}

TermIterator
Database::termlist_begin(Xapian::docid did) const
{
    LOGCALL(API, TermIterator, "Database::termlist_begin", did);
    if (did == 0)
	docid_zero_invalid();

    unsigned int multiplier = internal.size();
    if (rare(multiplier == 0))
	no_subdatabases();
    TermList *tl;
    if (multiplier == 1) {
	// There's no need for the MultiTermList wrapper in the common case
	// where we're only dealing with a single database.
	tl = internal[0]->open_term_list(did);
    } else {
	Assert(multiplier != 0);
	Xapian::doccount n = (did - 1) % multiplier; // which actual database
	Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

	tl = new MultiTermList(internal[n]->open_term_list(m), *this, n);
    }
    RETURN(TermIterator(tl));
}

TermIterator
Database::allterms_begin() const
{
    return allterms_begin("");
}

TermIterator
Database::allterms_begin(const std::string & prefix) const
{
    LOGCALL(API, TermIterator, "Database::allterms_begin", NO_ARGS);
    TermList * tl;
    if (rare(internal.size() == 0)) {
	tl = NULL;
    } else if (internal.size() == 1) {
	tl = internal[0]->open_allterms(prefix);
    } else {
	tl = new MultiAllTermsList(internal, prefix);
    }
    RETURN(TermIterator(tl));
}

bool
Database::has_positions() const
{
    LOGCALL(API, bool, "Database::has_positions", NO_ARGS);
    // If any sub-database has positions, the combined database does.
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	if ((*i)->has_positions()) RETURN(true);
    }
    RETURN(false);
}

PositionIterator
Database::positionlist_begin(Xapian::docid did, const string &tname) const
{
    LOGCALL(API, PositionIterator, "Database::positionlist_begin", did | tname);
    if (tname.empty())
	throw InvalidArgumentError("Zero length terms are invalid");
    if (did == 0)
	docid_zero_invalid();

    unsigned int multiplier = internal.size();
    if (rare(multiplier == 0))
	no_subdatabases();
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database
    RETURN(PositionIterator(internal[n]->open_position_list(m, tname)));
}

Xapian::doccount
Database::get_doccount() const
{
    LOGCALL(API, Xapian::doccount, "Database::get_doccount", NO_ARGS);
    Xapian::doccount docs = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	docs += (*i)->get_doccount();
    }
    RETURN(docs);
}

Xapian::docid
Database::get_lastdocid() const
{
    LOGCALL(API, Xapian::docid, "Database::get_lastdocid", NO_ARGS);
    Xapian::docid did = 0;

    unsigned int multiplier = internal.size();
    for (Xapian::doccount i = 0; i < multiplier; ++i) {
	Xapian::docid did_i = internal[i]->get_lastdocid();
	if (did_i) did = std::max(did, (did_i - 1) * multiplier + i + 1);
    }
    RETURN(did);
}

Xapian::doclength
Database::get_avlength() const
{
    LOGCALL(API, Xapian::doclength, "Database::get_avlength", NO_ARGS);
    Xapian::doccount docs = 0;
    Xapian::doclength totlen = 0;

    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	Xapian::doccount db_doccount = (*i)->get_doccount();
	docs += db_doccount;
	totlen += (*i)->get_avlength() * db_doccount;
    }
    LOGLINE(UNKNOWN, "get_avlength() = " << totlen << " / " << docs <<
	    " (from " << internal.size() << " dbs)");

    if (docs == 0) RETURN(0.0);
    RETURN(totlen / docs);
}

Xapian::doccount
Database::get_termfreq(const string & tname) const
{
    LOGCALL(API, Xapian::doccount, "Database::get_termfreq", tname);
    if (tname.empty()) RETURN(get_doccount());

    Xapian::doccount tf = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	tf += (*i)->get_termfreq(tname);
    }
    RETURN(tf);
}

Xapian::termcount
Database::get_collection_freq(const string & tname) const
{
    LOGCALL(API, Xapian::termcount, "Database::get_collection_freq", tname);
    if (tname.empty()) RETURN(get_doccount());

    Xapian::termcount cf = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	cf += (*i)->get_collection_freq(tname);
    }
    RETURN(cf);
}

Xapian::doccount
Database::get_value_freq(Xapian::valueno valno) const
{
    LOGCALL(API, Xapian::doccount, "Database::get_value_freq", valno);

    Xapian::doccount vf = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	vf += (*i)->get_value_freq(valno);
    }
    RETURN(vf);
}

string
Database::get_value_lower_bound(Xapian::valueno valno) const
{
    LOGCALL(API, string, "Database::get_value_lower_bound", valno);

    if (rare(internal.empty())) RETURN(string());

    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    i = internal.begin();
    string full_lb = (*i)->get_value_lower_bound(valno);
    while (++i != internal.end()) {
	string lb = (*i)->get_value_lower_bound(valno);
	if (lb < full_lb) full_lb = lb;
    }
    RETURN(full_lb);
}

std::string
Database::get_value_upper_bound(Xapian::valueno valno) const
{
    LOGCALL(API, std::string, "Database::get_value_upper_bound", valno);

    std::string full_ub;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	std::string ub = (*i)->get_value_upper_bound(valno);
	if (ub > full_ub)
	    full_ub = ub;
    }
    RETURN(full_ub);
}

Xapian::termcount
Database::get_doclength_lower_bound() const
{
    LOGCALL(API, Xapian::termcount, "Database::get_doclength_lower_bound", NO_ARGS);

    if (rare(internal.empty())) RETURN(0);

    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    i = internal.begin();
    Xapian::termcount full_lb = (*i)->get_doclength_lower_bound();
    while (++i != internal.end()) {
	Xapian::termcount lb = (*i)->get_doclength_lower_bound();
	if (lb < full_lb) full_lb = lb;
    }
    RETURN(full_lb);
}

Xapian::termcount
Database::get_doclength_upper_bound() const
{
    LOGCALL(API, Xapian::termcount, "Database::get_doclength_upper_bound", NO_ARGS);

    Xapian::termcount full_ub = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	Xapian::termcount ub = (*i)->get_doclength_upper_bound();
	if (ub > full_ub) full_ub = ub;
    }
    RETURN(full_ub);
}

Xapian::termcount
Database::get_wdf_upper_bound(const string & term) const
{
    LOGCALL(API, Xapian::termcount, "Database::get_wdf_upper_bound", term);

    Xapian::termcount full_ub = 0;
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); i++) {
	Xapian::termcount ub = (*i)->get_wdf_upper_bound(term);
	if (ub > full_ub) full_ub = ub;
    }
    RETURN(full_ub);
}

ValueIterator
Database::valuestream_begin(Xapian::valueno slot) const
{
    LOGCALL(API, ValueIterator, "Database::valuestream_begin", slot);
    if (internal.size() == 0)
       	RETURN(ValueIterator());
    if (internal.size() != 1)
	RETURN(ValueIterator(new MultiValueList(internal, slot)));
    RETURN(ValueIterator(internal[0]->open_value_list(slot)));
}

Xapian::termcount
Database::get_doclength(Xapian::docid did) const
{
    LOGCALL(API, Xapian::termcount, "Database::get_doclength", did);
    if (did == 0)
	docid_zero_invalid();

    unsigned int multiplier = internal.size();
    if (rare(multiplier == 0))
	no_subdatabases();
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database
    RETURN(internal[n]->get_doclength(m));
}

Document
Database::get_document(Xapian::docid did) const
{
    LOGCALL(API, Document, "Database::get_document", did);
    if (did == 0)
	docid_zero_invalid();

    unsigned int multiplier = internal.size();
    if (rare(multiplier == 0))
	no_subdatabases();
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

    // Open non-lazily so we throw DocNotFoundError if the doc doesn't exist.
    RETURN(Document(internal[n]->open_document(m, false)));
}

Document::Internal *
Database::get_document_lazily(Xapian::docid did) const
{
    LOGCALL(DB, Document::Internal *, "Database::get_document_lazily", did);
    if (did == 0)
	docid_zero_invalid();

    unsigned int multiplier = internal.size();
    Assert(multiplier != 0);
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database

    RETURN(internal[n]->open_document(m, true));
}

bool
Database::term_exists(const string & tname) const
{
    LOGCALL(API, bool, "Database::term_exists", tname);
    if (tname.empty()) {
	RETURN(get_doccount() != 0);
    }
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	if ((*i)->term_exists(tname)) RETURN(true);
    }
    RETURN(false);
}

void
Database::keep_alive()
{
    LOGCALL_VOID(API, "Database::keep_alive", NO_ARGS);
    vector<Xapian::Internal::RefCntPtr<Database::Internal> >::const_iterator i;
    for (i = internal.begin(); i != internal.end(); ++i) {
	(*i)->keep_alive();
    }
}

string
Database::get_description() const
{
    /// \todo display contents of the database
    return "Database()";
}

string
Database::get_spelling_suggestion(const string &word,
				  unsigned max_edit_distance) const
{
    return get_spelling_suggestion(word, string(), max_edit_distance);
}

string
Database::get_spelling_suggestion(const string &word,
                                  const string &prefix,
				  unsigned max_edit_distance) const
{
    LOGCALL(API, string, "Database::get_spelling_suggestion", word | prefix | max_edit_distance);

    if (!is_spelling_enabled(prefix)) return string();

    SpellingCorrector spelling_corrector(internal, prefix, max_edit_distance);
    SpellingSplitterNew spelling_splitter(internal, prefix, max_edit_distance);

    string corrector_result;
    double corrector_freq = spelling_corrector.get_spelling(word, corrector_result);

    string splitter_result;
    double splitter_freq = spelling_splitter.get_spelling(word, splitter_result);

    if (splitter_freq > corrector_freq)
	return splitter_result;

    return corrector_result;
}

vector<string>
Database::get_spelling_suggestion(const vector<string>& words, unsigned max_edit_distance) const
{
    return get_spelling_suggestion(words, string(), max_edit_distance);
}

vector<string>
Database::get_spelling_suggestion(const vector<string>& words, const string& prefix, unsigned max_edit_distance) const
{
    LOGCALL(API, string, "Database::get_spelling_suggestion", prefix | max_edit_distance);

    if (words.empty() || !is_spelling_enabled(prefix)) return vector<string>();

    SpellingCorrector spelling_corrector(internal, prefix, max_edit_distance);
    SpellingSplitterNew spelling_splitter(internal, prefix, max_edit_distance);

    vector<string> result_corrector;
    double corrector_freq = spelling_corrector.get_spelling(words, result_corrector);

    vector<string> result_splitter;
    double splitter_freq = spelling_splitter.get_spelling(words, result_splitter);

    if (splitter_freq > corrector_freq)
	return result_splitter;
    return result_corrector;
}

vector<vector<string> >
Database::get_spelling_suggestions(const vector<string>& words, const string& prefix,
                                  unsigned count, unsigned max_edit_distance) const
{
    LOGCALL(API, string, "Database::get_spelling_suggestion", prefix | max_edit_distance);

    if (words.empty() || !is_spelling_enabled(prefix)) return vector<vector<string> >();

    SpellingCorrector spelling_corrector(internal, prefix, max_edit_distance);
    SpellingSplitterNew spelling_splitter(internal, prefix, max_edit_distance);

    multimap<double, vector<string>, greater<double> > result_map;
    spelling_corrector.get_multiple_spelling(words, count, result_map);
    spelling_splitter.get_multiple_spelling(words, count, result_map);

    vector<vector<string> > result;
    multimap<double, vector<string>, greater<double> >::const_iterator it;

    unsigned limit = 0;
    for (it = result_map.begin(); it != result_map.end() && limit < count; ++it, ++limit)
	result.push_back(it->second);

    return result;
}

bool
Database::is_spelling_enabled(const std::string& prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::is_spelling_spelling", prefix);
    for (unsigned i = 0; i < internal.size(); ++i)
	if (internal[i]->is_spelling_enabled(prefix)) return true;
    return false;
}

TermIterator
Database::spellings_begin(const std::string & prefix) const
{
    LOGCALL(API, TermIterator, "Database::spellings_begin", prefix);
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_spelling_wordlist(prefix);
	if (tl) {
	    if (merger.get()) {
		merger.reset(new FreqAdderOrTermList(merger.release(), tl));
	    } else {
		merger.reset(tl);
	    }
	}
    }
    RETURN(TermIterator(merger.release()));
}

TermIterator
Database::synonyms_begin(const std::string &term) const
{
    LOGCALL(API, TermIterator, "Database::synonyms_begin", term);
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_synonym_termlist(term);
	if (tl) {
	    if (merger.get()) {
		merger.reset(new OrTermList(merger.release(), tl));
	    } else {
		merger.reset(tl);
	    }
	}
    }
    RETURN(TermIterator(merger.release()));
}

TermIterator
Database::synonym_keys_begin(const std::string &prefix) const
{
    LOGCALL(API, TermIterator, "Database::synonyms_keys_begin", prefix);
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList * tl = internal[i]->open_synonym_keylist(prefix);
	if (tl) {
	    if (merger.get()) {
		merger.reset(new OrTermList(merger.release(), tl));
	    } else {
		merger.reset(tl);
	    }
	}
    }
    RETURN(TermIterator(merger.release()));
}

string
Database::get_metadata(const string & key) const
{
    LOGCALL(API, string, "Database::get_metadata", key);
    if (key.empty())
	throw InvalidArgumentError("Empty metadata keys are invalid");
    if (internal.empty()) RETURN(std::string());
    RETURN(internal[0]->get_metadata(key));
}

Xapian::TermIterator
Database::metadata_keys_begin(const std::string &prefix) const
{
    LOGCALL(API, Xapian::TermIterator, "Database::metadata_keys_begin", NO_ARGS);
    if (internal.empty()) RETURN(TermIterator(NULL));
    RETURN(TermIterator(internal[0]->open_metadata_keylist(prefix)));
}

std::string
Database::get_uuid() const
{
    LOGCALL(API, std::string, "Database::get_uuid", NO_ARGS);
    string uuid;
    for (size_t i = 0; i < internal.size(); ++i) {
	string sub_uuid = internal[i]->get_uuid();
	// If any of the sub-databases have no uuid, we can't make a uuid for
	// the combined database.
	if (sub_uuid.empty())
	    RETURN(sub_uuid);
	if (!uuid.empty()) uuid += ':';
	uuid += sub_uuid;
    }
    RETURN(uuid);
}

///////////////////////////////////////////////////////////////////////////

WritableDatabase::WritableDatabase() : Database()
{
    LOGCALL_CTOR(API, "WritableDatabase", NO_ARGS);
}

WritableDatabase::WritableDatabase(Database::Internal *internal_)
	: Database(internal_)
{
    LOGCALL_CTOR(API, "WritableDatabase", internal_);
}

WritableDatabase::WritableDatabase(const WritableDatabase &other)
	: Database(other)
{
    LOGCALL_CTOR(API, "WritableDatabase", other);
}

void
WritableDatabase::operator=(const WritableDatabase &other)
{
    LOGCALL_VOID(API, "WritableDatabase::operator=", other);
    Database::operator=(other);
}

WritableDatabase::~WritableDatabase()
{
    LOGCALL_DTOR(API, "WritableDatabase");
}

XAPIAN_NORETURN(static void only_one_subdatabase_allowed());
static void only_one_subdatabase_allowed()
{
    throw Xapian::InvalidOperationError("WritableDatabase needs exactly one subdatabase");
}

void
WritableDatabase::commit()
{
    LOGCALL_VOID(API, "WritableDatabase::commit", NO_ARGS);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->commit();
}

void
WritableDatabase::begin_transaction(bool flushed)
{
    LOGCALL_VOID(API, "WritableDatabase::begin_transaction", NO_ARGS);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->begin_transaction(flushed);
}

void
WritableDatabase::commit_transaction()
{
    LOGCALL_VOID(API, "WritableDatabase::commit_transaction", NO_ARGS);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->commit_transaction();
}

void
WritableDatabase::cancel_transaction()
{
    LOGCALL_VOID(API, "WritableDatabase::cancel_transaction", NO_ARGS);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->cancel_transaction();
}


Xapian::docid
WritableDatabase::add_document(const Document & document)
{
    LOGCALL(API, Xapian::docid, "WritableDatabase::add_document", document);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    RETURN(internal[0]->add_document(document));
}

void
WritableDatabase::delete_document(Xapian::docid did)
{
    LOGCALL_VOID(API, "WritableDatabase::delete_document", did);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    if (did == 0)
	docid_zero_invalid();
    internal[0]->delete_document(did);
}

void
WritableDatabase::delete_document(const std::string & unique_term)
{
    LOGCALL_VOID(API, "WritableDatabase::delete_document", unique_term);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    if (unique_term.empty())
	throw InvalidArgumentError("Empty termnames are invalid");
    internal[0]->delete_document(unique_term);
}

void
WritableDatabase::replace_document(Xapian::docid did, const Document & document)
{
    LOGCALL_VOID(API, "WritableDatabase::replace_document", did | document);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    if (did == 0)
	docid_zero_invalid();
    internal[0]->replace_document(did, document);
}

Xapian::docid
WritableDatabase::replace_document(const std::string & unique_term,
				   const Document & document)
{
    LOGCALL(API, Xapian::docid, "WritableDatabase::replace_document", unique_term | document);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    if (unique_term.empty())
	throw InvalidArgumentError("Empty termnames are invalid");
    RETURN(internal[0]->replace_document(unique_term, document));
}

void
WritableDatabase::add_spelling(const std::string & word,
                               Xapian::termcount freqinc, const std::string & prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::add_spelling", word | freqinc);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->add_spelling(word, prefix, freqinc);
}

void
WritableDatabase::add_spelling(const std::string & first_word, const std::string & second_word,
                               Xapian::termcount freqinc, const std::string & prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::add_spelling", first_word | second_word | freqinc);
    if (internal.size() != 1) only_one_subdatabase_allowed();

    internal[0]->add_spellings(first_word, second_word, prefix, freqinc);
}

void
WritableDatabase::add_spelling(const std::string & first_word, const std::string & second_word,
                               const std::string & third_word, Xapian::termcount freqinc, const std::string & prefix) const
{
    add_spelling(first_word, second_word, prefix, freqinc);
    add_spelling(first_word, third_word, prefix, max(freqinc / 2, Xapian::termcount(1)));
}

void
WritableDatabase::remove_spelling(const std::string & word,
                                  Xapian::termcount freqdec, const std::string & prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::remove_spelling", word | freqdec);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->remove_spelling(word, prefix, freqdec);
}

void
WritableDatabase::remove_spelling(const std::string & first_word, const std::string & second_word,
                                  Xapian::termcount freqdec, const std::string & prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::remove_spelling", first_word | second_word | freqdec);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->remove_spellings(first_word, second_word, prefix, freqdec);
}

void
WritableDatabase::remove_spelling(const std::string & first_word, const std::string & second_word,
                                  const std::string & third_word, Xapian::termcount freqdec,
                                  const std::string & prefix) const
{
    remove_spelling(first_word, second_word, prefix, freqdec);
    remove_spelling(first_word, third_word, prefix, max(freqdec / 2, Xapian::termcount(1)));
}

void
WritableDatabase::enable_spelling(const std::string& prefix, const std::string& group_prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::enable_spelling", prefix | group_prefix);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->enable_spelling(prefix, group_prefix);
}

void
WritableDatabase::disable_spelling(const std::string& prefix) const
{
    LOGCALL_VOID(API, "WritableDatabase::disable_spelling", prefix);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->disable_spelling(prefix);
}

void
WritableDatabase::add_synonym(const std::string & term,
			      const std::string & synonym) const
{
    LOGCALL_VOID(API, "WritableDatabase::add_synonym", term | synonym);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->add_synonym(term, synonym);
}

void
WritableDatabase::remove_synonym(const std::string & term,
				 const std::string & synonym) const
{
    LOGCALL_VOID(API, "WritableDatabase::remove_synonym", term | synonym);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->remove_synonym(term, synonym);
}

void
WritableDatabase::clear_synonyms(const std::string & term) const
{
    LOGCALL_VOID(API, "WritableDatabase::clear_synonyms", term);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    internal[0]->clear_synonyms(term);
}

void
WritableDatabase::set_metadata(const string & key, const string & value)
{
    LOGCALL_VOID(API, "WritableDatabase::set_metadata", key | value);
    if (internal.size() != 1) only_one_subdatabase_allowed();
    if (key.empty())
	throw InvalidArgumentError("Empty metadata keys are invalid");
    internal[0]->set_metadata(key, value);
}

string
WritableDatabase::get_description() const
{
    /// \todo display contents of the writable database
    return "WritableDatabase()";
}

}
