/* quartz_document.h: Document from a Quartz Database
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

#ifndef OM_HGUARD_QUARTZ_DOCUMENT_H
#define OM_HGUARD_QUARTZ_DOCUMENT_H

#include "document.h"
#include "refcnt.h"
#include "quartz_table.h"

class QuartzDatabase;

/// A document from a Quartz format database
class QuartzDocument : public Document {
    friend class QuartzDatabase;
    friend class QuartzWritableDatabase;
    private:
	RefCntPtr<const Database> database;
	om_docid did;

	QuartzTable *attribute_table;
	QuartzTable *record_table;

	QuartzDocument(RefCntPtr<const Database> database_,
		       QuartzTable *attribute_table_,
		       QuartzTable *record_table_,
		       om_docid did_);

	// Stop copying
	QuartzDocument(const QuartzDocument &);
	QuartzDocument & operator = (const QuartzDocument &);
    public:
	~QuartzDocument();

	OmKey do_get_key(om_keyno keyid) const;
	std::map<om_keyno, OmKey> do_get_all_keys() const;
	OmData do_get_data() const;
};


#endif /* OM_HGUARD_QUARTZ_DOCUMENT_H */
