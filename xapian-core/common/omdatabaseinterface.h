/* omdatabaseinterface.h: Extra interface to OmDatabase
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

#ifndef OM_HGUARD_OMDATABASEINTERFACE_H
#define OM_HGUARD_OMDATABASEINTERFACE_H

#include <vector>

#include <om/omenquire.h>
#include "database_builder.h"
#include "multi_database.h"

/** This class is used basically to add an interface to OmDatabase
 *  which isn't exported to the API.  Internal OM functions can get at
 *  this interface by going through this friend class.
 */
class OmDatabase::InternalInterface {
    public:
	/** Create a MultiDatabase from an OmDatabase.
	 *
	 *  Even if the OmDatabase contains only one IRDatabase, this will
	 *  be returned encapsulated in a MultiDatabase.
	 *
	 *  The MultiDatabase will be newly created if it hasn't been
	 *  asked for previously (for example, a database has been added
	 *  to the group since it was last requested).  Otherwise, the
	 *  previously created MultiDatabase will be returned.
	 *
	 *  @param db		The source OmDatabase object.
	 *
	 *  @return  A reference counted pointer to the MultiDatabase.
	 */
	static OmRefCntPtr<MultiDatabase> get_multi_database(const OmDatabase &db);
};

#endif // OM_HGUARD_OMDATABASEINTERFACE_H
