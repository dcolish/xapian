/* quartz_database.cc: quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
 * Copyright 2002 Ananova Ltd
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

#include "quartz_table_manager.h"

#include "quartz_database.h"
#include "utils.h"
#include "omdebug.h"
#include "autoptr.h"
#include "om/omerror.h"
#include "refcnt.h"

#include "quartz_postlist.h"
#include "quartz_termlist.h"
#include "quartz_positionlist.h"
#include "quartz_lexicon.h"
#include "quartz_record.h"
#include "quartz_attributes.h"
#include "quartz_document.h"
#include "quartz_alltermslist.h"

#include <string>

//
// Compulsory settings.
// quartz_dir    - Directory that the database is stored in.  Must be a full
//                 path.
//
// Optional settings.
// quartz_logfile - File in which to store log information regarding
//                 modifications and accesses made to the database.  If not
//                 specified, such log information will not be stored.
//                 If this is a relative path, it is taken to be relative
//                 to the quartz_dir directory.
//
// quartz_block_size - Integer.  This is the size of the blocks to use in
//                 the tables, in bytes.  Acceptable values are powers of
//                 two in the range 2048 to 65536.  The default is 8192.
//                 This setting is only used when creating databases.  If
//                 the database already exists, it is completely ignored.
//
QuartzDatabase::QuartzDatabase(const OmSettings & settings)
{
    // Open database manager
    tables.reset(new QuartzDiskTableManager(get_db_dir(settings),
					    get_log_filename(settings),
					    true,
					    0u,
					    false,
					    false));
}

QuartzDatabase::QuartzDatabase(AutoPtr<QuartzTableManager> tables_)
	: tables(tables_)
{
}

QuartzDatabase::~QuartzDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzDatabase destructor.");
    }
}

std::string
QuartzDatabase::get_db_dir(const OmSettings & settings)
{
    return settings.get("quartz_dir");
}

std::string
QuartzDatabase::get_log_filename(const OmSettings & settings)
{
    return settings.get("quartz_logfile", "");
}

unsigned int
QuartzDatabase::get_block_size(const OmSettings & settings)
{
    return settings.get_int("quartz_block_size", QUARTZ_BTREE_DEF_BLOCK_SIZE);
}

bool
QuartzDatabase::get_create(const OmSettings & settings)
{
    return settings.get_bool("database_create", false);
}

bool
QuartzDatabase::get_allow_overwrite(const OmSettings & settings)
{
    return settings.get_bool("database_allow_overwrite", false);
}

void
QuartzDatabase::do_begin_session()
{
    throw OmInvalidOperationError(
	"Cannot begin a modification session: database opened readonly.");
}

void
QuartzDatabase::do_end_session()
{ Assert(false); }

void
QuartzDatabase::do_flush()
{ Assert(false); }

void
QuartzDatabase::do_begin_transaction()
{ Assert(false); }

void
QuartzDatabase::do_commit_transaction()
{ Assert(false); }

void
QuartzDatabase::do_cancel_transaction()
{ Assert(false); }

om_docid
QuartzDatabase::do_add_document(const OmDocument & document)
{
    Assert(false);
    return 0;
}

void
QuartzDatabase::do_delete_document(om_docid did)
{ Assert(false); }

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocument & document)
{ Assert(false); }

om_doccount 
QuartzDatabase::get_doccount() const
{
    OmLockSentry sentry(quartz_mutex);

    return get_doccount_internal();
}

om_doccount
QuartzDatabase::get_doccount_internal() const
{
    return QuartzRecordManager::get_doccount(*(tables->get_record_table()));
}

om_doclength
QuartzDatabase::get_avlength() const
{
    OmLockSentry sentry(quartz_mutex);

    return get_avlength_internal();
}

om_doclength
QuartzDatabase::get_avlength_internal() const
{
    // FIXME: probably want to cache this value (but not miss updates)
    om_doccount docs = get_doccount_internal();
    if (docs == 0) return 0;
    om_totlength totlen = QuartzRecordManager::get_total_length(*(tables->get_record_table()));

    return (double) totlen / docs;
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
    Assert(did != 0);
    OmLockSentry sentry(quartz_mutex);

    QuartzTermList termlist(0,
			    tables->get_termlist_table(),
			    tables->get_lexicon_table(),
			    did,
			    0);
    return termlist.get_doclength();
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);

    om_doccount termfreq = 0; // If not found, this value will be unchanged.
    QuartzLexicon::get_entry(tables->get_lexicon_table(),
			     tname,
			     &termfreq);
    return termfreq;
}

om_termcount
QuartzDatabase::get_collection_freq(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);

    om_termcount collfreq = 0; // If not found, this value will be unchanged.
    QuartzPostList pl(0,
		      tables->get_postlist_table(),
		      tables->get_positionlist_table(),
		      tname);
    collfreq = pl.get_collection_freq();
    return collfreq;
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
    Assert(tname.size() != 0);
    OmLockSentry sentry(quartz_mutex);
    return QuartzLexicon::get_entry(tables->get_lexicon_table(),
				    tname, 0);
}


LeafPostList *
QuartzDatabase::do_open_post_list(const om_termname& tname) const
{
    OmLockSentry sentry(quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return open_post_list_internal(tname, ptrtothis);
}

LeafPostList *
QuartzDatabase::open_post_list_internal(const om_termname& tname,
				RefCntPtr<const Database> ptrtothis) const
{
    Assert(tname.size() != 0);
    return(new QuartzPostList(ptrtothis,
			      tables->get_postlist_table(),
			      tables->get_positionlist_table(),
			      tname));
}

LeafTermList *
QuartzDatabase::open_term_list_internal(om_docid did,
				RefCntPtr<const Database> ptrtothis) const
{
    Assert(did != 0);
    return(new QuartzTermList(ptrtothis,
			      tables->get_termlist_table(),
			      tables->get_lexicon_table(),
			      did,
			      get_doccount_internal()));
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
    OmLockSentry sentry(quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return open_term_list_internal(did, ptrtothis);
}

Document *
QuartzDatabase::open_document(om_docid did, bool lazy) const
{
    Assert(did != 0);
    OmLockSentry sentry(quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzDatabase> ptrtothis(tmp, this);

    return new QuartzDocument(ptrtothis,
			      tables->get_attribute_table(),
			      tables->get_record_table(),
			      did, lazy);
}

AutoPtr<PositionList> 
QuartzDatabase::open_position_list(om_docid did,
				   const om_termname & tname) const
{
    Assert(did != 0);
    OmLockSentry sentry(quartz_mutex);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(tables->get_positionlist_table(), did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	RefCntPtr<const QuartzDatabase> ptrtothis(RefCntBase::RefCntPtrToThis(),
						  this);
	// If the doc doesn't exist, this will throw OmDocNotFound:
	AutoPtr<LeafTermList> ltl(open_term_list_internal(did, ptrtothis));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw OmRangeError("Can't open position list: requested term is not present in document.");
    }

    return AutoPtr<PositionList>(poslist.release());
}

void
QuartzDatabase::do_reopen()
{
    tables->reopen();
}

TermList *
QuartzDatabase::open_allterms() const
{
    AutoPtr<QuartzCursor> pl_cursor(tables->get_postlist_table()->cursor_get());
    return new QuartzAllTermsList(RefCntPtr<const QuartzDatabase>(RefCntPtrToThis(), this),
				  pl_cursor);
}


QuartzWritableDatabase::QuartzWritableDatabase(const OmSettings & settings)
	: buffered_tables(new QuartzBufferedTableManager(
				QuartzDatabase::get_db_dir(settings),
				QuartzDatabase::get_log_filename(settings),
				QuartzDatabase::get_block_size(settings),
				QuartzDatabase::get_create(settings),
				QuartzDatabase::get_allow_overwrite(settings))),
	  changecount(0),
	  database_ro(AutoPtr<QuartzTableManager>(buffered_tables))
{
}

QuartzWritableDatabase::~QuartzWritableDatabase()
{
    // FIXME - release write lock if held
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzWritableDatabase destructor.");
    }
}

void
QuartzWritableDatabase::do_begin_session()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    changecount = 0;
    // FIXME - get a write lock on the database
}

void
QuartzWritableDatabase::do_end_session()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    buffered_tables->apply();

    // FIXME - release write lock on the database (even if an apply() throws)
}

void
QuartzWritableDatabase::do_flush()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    changecount = 0;
    buffered_tables->apply();
}

void
QuartzWritableDatabase::do_begin_transaction()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_commit_transaction()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_cancel_transaction()
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    throw OmUnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}

om_docid
QuartzWritableDatabase::do_add_document(const OmDocument & document)
{
    DEBUGCALL(DB, om_docid,
	      "QuartzWritableDatabase::do_add_document", document);

    OmLockSentry sentry(database_ro.quartz_mutex);

    Assert(buffered_tables != 0);

    // Calculate the new document length
    quartz_doclen_t new_doclen = 0;
    {
	OmTermIterator term = document.termlist_begin();
	OmTermIterator term_end = document.termlist_end();    
	for ( ; term != term_end; ++term) {
	    new_doclen += term.get_wdf();
	}
    }

    om_docid did;

    try {
	// Set the record, and get the document ID to use.
	did = QuartzRecordManager::add_record(
		*(buffered_tables->get_record_table()),
		document.get_data());
	Assert(did != 0);

	// Set the attributes.
	{
	    OmKeyListIterator key = document.keylist_begin();
	    OmKeyListIterator key_end = document.keylist_end();
	    for ( ; key != key_end; ++key) {
		QuartzAttributesManager::add_attribute(
		    *(buffered_tables->get_attribute_table()),
		    *key, did, key.get_keyno());
	    }
	}

	// Set the termlist.
	QuartzTermList::set_entries(buffered_tables->get_termlist_table(), did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);

	// Set the new document length
	// (Old doclen is always zero, since this is a new document)
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		0,
		new_doclen);

	{
	    OmTermIterator term = document.termlist_begin();
	    OmTermIterator term_end = document.termlist_end();    
	    for ( ; term != term_end; ++term) {
		QuartzLexicon::increment_termfreq(
		    buffered_tables->get_lexicon_table(),
		    *term);
		QuartzPostList::add_entry(buffered_tables->get_postlist_table(),
					  *term, did, term.get_wdf(),
					  new_doclen);
		if (term.positionlist_begin() != term.positionlist_end())
		{
		  QuartzPositionList::set_positionlist(
		      buffered_tables->get_positionlist_table(), did,
		      *term, term.positionlist_begin(), term.positionlist_end());
		}
	    }
	}

    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();

	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount >= 1000) {
	changecount = 0;
	buffered_tables->apply();
    }

    RETURN(did);
}

void
QuartzWritableDatabase::do_delete_document(om_docid did)
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);
    Assert(buffered_tables != 0);

    try {
	QuartzDatabase::RefCntPtrToThis tmp;
	RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

	QuartzTermList termlist(ptrtothis,
				database_ro.tables->get_termlist_table(),
				database_ro.tables->get_lexicon_table(),
				did,
				database_ro.get_doccount_internal());

	termlist.next();
	while (!termlist.at_end()) {
	    om_termname tname = termlist.get_termname();
	    QuartzPostList::delete_entry(buffered_tables->get_postlist_table(),
		tname, did);
	    QuartzPositionList::delete_positionlist(
		buffered_tables->get_positionlist_table(),
		did, tname);
	    QuartzLexicon::decrement_termfreq(
		buffered_tables->get_lexicon_table(),
		tname);
	    termlist.next();
	}

	// Set the document length.
	// (New doclen is always zero, since we're deleting the document.)
	quartz_doclen_t old_doclen = termlist.get_doclength();
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		old_doclen,
		0);

	// Remove the attributes
	QuartzAttributesManager::delete_all_attributes(*(buffered_tables->get_attribute_table()),
						       did);

	// Remove the termlist.
	QuartzTermList::delete_termlist(buffered_tables->get_termlist_table(),
					did);

	// Remove the record.
	QuartzRecordManager::delete_record(*(buffered_tables->get_record_table()),
					   did);
    } catch (...) {
	// If an error occurs while deleting a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();

	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount > 1000) {
	changecount = 0;
	buffered_tables->apply();
    }
}

void
QuartzWritableDatabase::do_replace_document(om_docid did,
				    const OmDocument & document)
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);

    Assert(buffered_tables != 0);
    DEBUGCALL(DB, void,
	      "QuartzWritableDatabase::do_replace_document", document);

    // Calculate the new document length
    quartz_doclen_t new_doclen = 0;
    {
	OmTermIterator term = document.termlist_begin();
	OmTermIterator term_end = document.termlist_end();    
	for ( ; term != term_end; ++term) {
	    new_doclen += term.get_wdf();
	}
    }

    try {
	// Replace the record
	QuartzRecordManager::replace_record(
		*(buffered_tables->get_record_table()),
		document.get_data(),
		did);

	// Replace the attributes.
	QuartzAttributesManager::delete_all_attributes(
		*(buffered_tables->get_attribute_table()),
		did);
	{
	    OmKeyListIterator key = document.keylist_begin();
	    OmKeyListIterator key_end = document.keylist_end();
	    for ( ; key != key_end; ++key) {
		QuartzAttributesManager::add_attribute(
		    *(buffered_tables->get_attribute_table()),
		    *key, did, key.get_keyno());
	    }
	}

	// Set the termlist.
        // We should detect what terms have been deleted, and which ones have been added. Then we'll add/delete only
        //   those terms.
	quartz_doclen_t old_doclen;
	{
            std::vector<om_termname> delTerms;
            std::vector<om_termname> addTerms;
            std::vector<om_termname> posTerms;

            // First, before we modify the Postlist, we should detect the old document length, since that
            //   seems to be of some importance later on.
	    QuartzDatabase::RefCntPtrToThis tmp;
	    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);
	    QuartzTermList termlist(ptrtothis,
	  			    database_ro.tables->get_termlist_table(),
				    database_ro.tables->get_lexicon_table(),
				    did,
				    database_ro.get_doccount_internal());
	    old_doclen = termlist.get_doclength();
            OmTermIterator tNewIter = document.termlist_begin();
            termlist.next();
            while (!termlist.at_end() && tNewIter != document.termlist_end())
            {
              om_termname tname = termlist.get_termname();
              if (tname < (*tNewIter))
              {
                // Deleted term exists in the old termlist, but not in the new one.
                delTerms.push_back(tname);
                termlist.next();
              }
              else
              {
                if (tname > (*tNewIter))
                {
                  // Added term does not exist in the old termlist, but it does in the new one.
                  addTerms.push_back((*tNewIter));
                  ++tNewIter;
                }
                else
                {
                  // Terms are equal, but perhaps its positionlist has been modified. Record it, and skip to the next.
                  posTerms.push_back(tname);
                  ++tNewIter;
                  termlist.next();
                }
              }
            }
            // One of the lists (or both!) has been processed. Check if any of the iterators are not at the end.
            while (!termlist.at_end())
            {
              // Any term left in the old list must be removed.
              om_termname tname = termlist.get_termname();
              delTerms.push_back(tname);
              termlist.next();
            }
            while (tNewIter != document.termlist_end())
            {
              // Any term left in the new list must be added.
              addTerms.push_back((*tNewIter));
              ++tNewIter;
            }
            // We now know which terms to add and which to remove. Let's get to work!
            // Delete the terms on our "hitlist"...
            std::vector<om_termname>::iterator vIter = delTerms.begin();
            while (vIter != delTerms.end())
            {
	        om_termname tname = (*vIter);
	        QuartzPostList::delete_entry(buffered_tables->get_postlist_table(),
		    tname, did);
	        QuartzPositionList::delete_positionlist(
		    buffered_tables->get_positionlist_table(),
		    did, tname);
	        QuartzLexicon::decrement_termfreq(
		    buffered_tables->get_lexicon_table(),
		    tname);
                ++vIter;
	    }
            // Now add the terms that are new...
            vIter = addTerms.begin();
            while (vIter != addTerms.end())
            {
                OmTermIterator tIter = document.termlist_begin();
                tIter.skip_to((*vIter));
		QuartzLexicon::increment_termfreq(
		    buffered_tables->get_lexicon_table(),
		    *tIter);
		QuartzPostList::add_entry(buffered_tables->get_postlist_table(),
					  *tIter, did, tIter.get_wdf(),
					  new_doclen);
		if (tIter.positionlist_begin() != tIter.positionlist_end())
		{
		  QuartzPositionList::set_positionlist(
		      buffered_tables->get_positionlist_table(), did,
		      *tIter, tIter.positionlist_begin(), tIter.positionlist_end());
		}
                ++vIter;
	    }
            // Finally, update the positionlist of terms that are not new or removed.
            vIter = posTerms.begin();
            while (vIter != posTerms.end())
            {
                OmTermIterator tIter = document.termlist_begin();
                tIter.skip_to((*vIter));
                if (tIter.positionlist_begin() == tIter.positionlist_end())
                {
                  // In the new document, this term does not have any positions associated with it
                  QuartzPositionList qpl;
                  qpl.read_data(buffered_tables->get_positionlist_table(), did, *tIter);
                  if (qpl.get_size() != 0)
                  {
                    // But there are positions associated with this term in the index. Delete the positionlist.
                    QuartzPositionList::delete_positionlist(buffered_tables->get_positionlist_table(), did, *tIter);
                  }
                }
                else
                {
                  // In the new document, this term has positions associated with it. Check whether we need to re-create
                  //   the positionlist.
                  QuartzPositionList qpl;
                  qpl.read_data(buffered_tables->get_positionlist_table(), did, *tIter);
                  qpl.next();
                  OmPositionListIterator pIter = tIter.positionlist_begin();
                  while (!qpl.at_end() && pIter != tIter.positionlist_end())
                  {
                    if (qpl.get_current_pos() != (*pIter))
                    {
                      // Position lists do not match, so create a new one.
  		      QuartzPositionList::set_positionlist(
		        buffered_tables->get_positionlist_table(), did,
		        *tIter, tIter.positionlist_begin(), tIter.positionlist_end());
                      break;
                    }
                    qpl.next();
                    ++pIter;
                  }
                  if (!qpl.at_end() || pIter != tIter.positionlist_end())
                  {
                    // One of the position lists has not reached yet the end -- which means they are different. Create a
                    //   new posisitionlist based on the one in the new OmDocument.
  		    QuartzPositionList::set_positionlist(
		      buffered_tables->get_positionlist_table(), did,
		      *tIter, tIter.positionlist_begin(), tIter.positionlist_end());
                  }
                }
                ++vIter;
	    }
            // All done!
	}
        // Set pointers from the document to the terms.
	QuartzTermList::set_entries(buffered_tables->get_termlist_table(), did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);
	// Set the new document length
	QuartzRecordManager::modify_total_length(
		*(buffered_tables->get_record_table()),
		old_doclen,
		new_doclen);

    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();

	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    if (++changecount > 1000) {
	changecount = 0;
	buffered_tables->apply();
    }
}

om_doccount 
QuartzWritableDatabase::get_doccount() const
{
    OmLockSentry sentry(database_ro.quartz_mutex);
    return database_ro.get_doccount_internal();
}

om_doclength
QuartzWritableDatabase::get_avlength() const
{
    return database_ro.get_avlength();
}

om_doclength
QuartzWritableDatabase::get_doclength(om_docid did) const
{
    return database_ro.get_doclength(did);
}

om_doccount
QuartzWritableDatabase::get_termfreq(const om_termname & tname) const
{
    return database_ro.get_termfreq(tname);
}

om_termcount
QuartzWritableDatabase::get_collection_freq(const om_termname & tname) const
{
    return database_ro.get_collection_freq(tname);
}

bool
QuartzWritableDatabase::term_exists(const om_termname & tname) const
{
    return database_ro.term_exists(tname);
}


LeafPostList *
QuartzWritableDatabase::do_open_post_list(const om_termname& tname) const
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    return database_ro.open_post_list_internal(tname, ptrtothis);
}

LeafTermList *
QuartzWritableDatabase::open_term_list(om_docid did) const
{
    OmLockSentry sentry(database_ro.quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    return database_ro.open_term_list_internal(did, ptrtothis);
}

Document *
QuartzWritableDatabase::open_document(om_docid did, bool lazy) const
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);

    RefCntBase::RefCntPtrToThis tmp;
    RefCntPtr<const QuartzWritableDatabase> ptrtothis(tmp, this);

    return new QuartzDocument(ptrtothis,
			      buffered_tables->get_attribute_table(),
			      buffered_tables->get_record_table(),
			      did, lazy);
}

AutoPtr<PositionList> 
QuartzWritableDatabase::open_position_list(om_docid did,
				   const om_termname & tname) const
{
    Assert(did != 0);
    OmLockSentry sentry(database_ro.quartz_mutex);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(buffered_tables->get_positionlist_table(), did, tname);

    return AutoPtr<PositionList>(poslist.release());
}

void
QuartzWritableDatabase::do_reopen()
{
    /* Do nothing - we're the only writer, and so must be up to date. */
}

TermList *
QuartzWritableDatabase::open_allterms() const
{
    AutoPtr<QuartzCursor> pl_cursor(buffered_tables->get_postlist_table()->cursor_get());
    return new QuartzAllTermsList(RefCntPtr<const QuartzWritableDatabase>(RefCntPtrToThis(), this),
				  pl_cursor);
}
