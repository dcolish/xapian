/* indexer.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_INDEXER_H
#define OM_HGUARD_INDEXER_H

#include <iosfwd>
#include "autoptr.h"

#include <xapian/types.h>

class Xapian::Document;

/** A source of data for indexing (eg, a file)
 */
class IndexerSource {
    private:
	/// Copying is not permitted
	IndexerSource(const IndexerSource &);
	/// Assignment is not permitted
	void operator=(const IndexerSource &);
    public:
	// Get the stream of data
	virtual AutoPtr<std::istream> get_stream() const = 0;
	IndexerSource() {}
	virtual ~IndexerSource() {}
};

/** Somewhere to put the indexed terms. (eg, a document, or a query)
 */
class IndexerDestination {
    private:
	/// Copying is not permitted
	IndexerDestination(const IndexerDestination &);
	/// Assignment is not permitted
	void operator=(const IndexerDestination &);
    public:
	IndexerDestination() {}
	virtual ~IndexerDestination() {}

	/** Add a new document to the destination.
	 *  The destination may return a document ID, but this will be ignored
	 *  by the indexer.
	 */
	virtual Xapian::docid add_document(const Xapian::Document & document) = 0;
};

/** A way to generate terms from sources
 */
class Indexer {
    private:
	/// Copying is not permitted
	Indexer(const Indexer &);
	/// Assignment is not permitted
	void operator=(const Indexer &);
    protected:
	IndexerDestination * dest;
    public:
	Indexer() : dest(NULL) {}
	virtual ~Indexer() {}

	// Set the destination
	void set_destination(IndexerDestination *newdest) {dest = newdest;}

	// Generate terms from the source, and put them in the destination
	virtual void add_source(const IndexerSource &) = 0;
};

#endif /* OM_HGUARD_INDEXER_H */
