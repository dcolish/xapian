/* quartz_modifications.h: Management of modifications to a quartz database
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

#ifndef OM_HGUARD_QUARTZ_MODIFICATIONS_H
#define OM_HGUARD_QUARTZ_MODIFICATIONS_H

#include "config.h"
#include "quartz_table_manager.h"
#include "quartz_diffs.h"
#include "quartz_log.h"
#include "quartz_table.h"
#include "om/omindexdoc.h"
#include "om/autoptr.h"

#include <stdio.h>

/** Class managing all the modifications made to a Quartz database.
 */
class QuartzModifications {
    private:
	/// Copying not allowed
	QuartzModifications(const QuartzModifications &);

	/// Assignment not allowed
	void operator=(const QuartzModifications &);

	/** Pointer to the database manager.
	 */
	QuartzDiskTableManager * table_manager;


	/** Diffs made to the PostList database.
	 */
	AutoPtr<QuartzPostListDiffs> postlist_diffs;

	/** Diffs made to the PositionList database.
	 */
	AutoPtr<QuartzPositionListDiffs> positionlist_diffs;

	/** Diffs made to the TermList database.
	 */
	AutoPtr<QuartzTermListDiffs> termlist_diffs;

	/** Diffs made to the Lexicon database.
	 */
	AutoPtr<QuartzLexiconDiffs> lexicon_diffs;

	/** Diffs made to the Attribute database.
	 */
	AutoPtr<QuartzAttributeDiffs> attribute_diffs;

	/** Diffs made to the Record database.
	 */
	AutoPtr<QuartzRecordDiffs> record_diffs;

	/** Open the diffs objects.
	 */
	void open_diffs();

	/** Close the diffs objects.
	 */
	void close_diffs();

	/** Get a document ID for a new document.
	 */
	om_docid get_newdocid();
    public:

	/** Construct the modifications object.
	 *
	 *  @param table_manager_  The object holding the tables
	 *                         constituting the database.
	 *                      
	 */
	QuartzModifications(QuartzBufferedTableManager * table_manager_);

	/** Destroy the modifications.  Any unapplied modifications will
	 *  be lost.
	 */
	~QuartzModifications();

	/** Atomically apply the modifications.  Throws an exception if an
	 *  error occurs.  If an error occurs (eg, any of the modifications
	 *  fail), the database will be left unaltered, and the
	 *  modifications will be discarded.  If a catastrophic error occurs
	 *  (such as a power failure, or disk error), the database may be
	 *  left in a state from which a recovery step needs to be performed
	 *  in order to use it again; after such a step the partly applied
	 *  modifications will be discarded.
	 */
	void apply();

	/** Store the changes needed to add a document in the modifications.
	 */
	om_docid add_document(const OmDocumentContents & document);

	/** Store the changes needed to delete a document in the modifications.
	 */
	void delete_document(om_docid did);

	/** Store the changes needed to replace a document in the modifications.
	 */
	void replace_document(om_docid did,
			      const OmDocumentContents & document);

	/** Get a document, checking the modifications in case it has been
	 *  modified.  FIXME: store the relevant blocks in the modifications?
	 *  Maybe in a separate place so that they can be discarded if
	 *  memory is tight...
	 */
	OmDocumentContents get_document(om_docid did);
};

#endif /* OM_HGUARD_QUARTZ_MODIFICATIONS_H */
