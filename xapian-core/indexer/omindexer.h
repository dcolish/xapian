/* omindexer.h: interface to building and using an indexer
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

#ifndef OM_HGUARD_OMINDEXER_H
#define OM_HGUARD_OMINDEXER_H

#include <string>

#include "om/omindexermessage.h"
#include <xapian/document.h>

class OmIndexer {
    public:
	class Internal;
	Internal *internal;

	/** Set the input
	 */
	void set_input(OmIndexerMessage msg);

	/** Set a configuration value in a node.
	 *  Will throw an exception if a non-existant node_id is specified.
	 *
	 * @param node_id	The id of the node to configure.
	 * @param key		The key to set
	 * @param value		The value associated with the key.
	 */
	void set_node_config(const std::string &node_id,
			     const std::string &key,
			     const std::string &value);

	/** Get the output document contents.
	 */
	Xapian::Document get_output();

	/** Get the output as an OmIndexerMessage
	 */
	OmIndexerMessage get_raw_output();

	/** Destructor
	 */
	~OmIndexer();

	/** Copy constructor */
	OmIndexer(const OmIndexer &other);
	/** Assignment operator */
	void operator=(const OmIndexer &other);
    private:
	friend class OmIndexerBuilder;

	/** Construct a blank indexer
	 */
	OmIndexer();
};

#endif /* OM_HGUARD_OMINDEXER_H */
