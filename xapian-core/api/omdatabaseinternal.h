/* omdatabaseinternal.h: Class definition for OmDatabase::Internal
 * and OmDatabaseGroup::Internal
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

#ifndef OM_HGUARD_OMDATABASEINTERNAL_H
#define OM_HGUARD_OMDATABASEINTERNAL_H

#include <vector>

#include <om/omenquire.h>
#include "omlocks.h"
#include "omrefcnt.h"
#include "database_builder.h"
#include "multi_database.h"
#include "database.h"
#include "om/omdatabase.h"

/////////////////////////////
// Internals of OmDatabase //
/////////////////////////////

/** Reference counted internals for OmDatabase.
 */
class OmDatabase::Internal {
    public:
	/** Make a new internal object, with the user supplied parameters.
	 *
	 *  This opens the database and stores it in the ref count pointer.
	 *
	 *  @param params  a vector of parameters to be used to open the
	 *                 database: meaning and number required depends
	 *                 on database type.
	 *
	 *  @param readonly flag as to whether to open database read only
	 */
	Internal(const OmSettings &params, bool readonly);

	/** Make a copy of this object, copying the ref count pointer.
	 */
	Internal(const Internal &other) : mydb(other.mydb), mutex() {}

	/** The database.  Access to this is not protected by a mutex here -
	 *  it is up to the database to deal with thread safety.
	 */
	OmRefCntPtr<IRDatabase> mydb;

	/** A lock to control concurrent access to this object.
	 *  This is not intended to control access to the IRDatabase object.
	 */
	OmLock mutex;
};

//////////////////////////////////
// Internals of OmDatabaseGroup //
//////////////////////////////////

/** The implementation for OmDatabaseGroup.
 */
class OmDatabaseGroup::Internal {
    friend class OmDatabaseGroup::InternalInterface;
    private:
	/** The databases which this consists of.
	 */
	std::vector<OmRefCntPtr<IRDatabase> > databases;

	/** The multidatabase, if this has been created.
	 */
	OmRefCntPtr<MultiDatabase> multi_database;

    public:
	Internal() {}
	Internal(const Internal &other)
		: databases(other.databases), mutex() {}

	/** Mutex to protect access to these internals.
	 */
	OmLock mutex;

	/** Add a database to the group, based on parameters.
	 */
	void add_database(const OmSettings &params);

	/** Add an already opened database to the group.
	 */
	void add_database(OmRefCntPtr<IRDatabase> newdb);

	/** Create a MultiDatabase from an OmDatabaseGroup.
	 *
	 *  The MultiDatabase will be newly created if it hasn't been
	 *  asked for previously (for example, a database has been added
	 *  to the group since it was last requested).  Otherwise, the
	 *  previously created MultiDatabase will be returned.
	 *
	 *  @return  A reference counted pointer to the MultiDatabase.
	 */
	OmRefCntPtr<MultiDatabase> get_multidatabase();
};

#endif // OM_HGUARD_OMDATABASEINTERNAL_H
