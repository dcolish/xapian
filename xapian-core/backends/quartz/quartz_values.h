/* quartz_values.h: Values in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#ifndef OM_HGUARD_QUARTZ_VALUES_H
#define OM_HGUARD_QUARTZ_VALUES_H

#include <map>
#include <string>

#include "quartz_table.h"
#include <xapian/types.h>

using namespace std;

class QuartzValueManager {
    private:
	QuartzValueManager();
	~QuartzValueManager();

	/** Read an entry from position.  Throw appropriate exceptions if
	 *  data runs out.
	 */
	static void unpack_entry(const char ** pos,
				 const char * end,
				 Xapian::valueno * this_value_no,
				 string & this_value);

	/** Generate key representing docid/valueno pair.
	 */
	static void make_key(string & key, Xapian::docid did, Xapian::valueno valueno);
    public:

	/** Store a value.  If a value of the same document ID and
	 *  value number already exists, it is overwritten by this.
	 */
	static void add_value(QuartzTable & table,
				  const string & value,
				  Xapian::docid did,
				  Xapian::valueno valueno);

	/** Get a value.
	 *
	 *  @return The value if found, a null value otherwise.
	 */
	static void get_value(const QuartzTable & table,
				  string & value,
				  Xapian::docid did,
				  Xapian::valueno valueno);

	/** Get all values.
	 *
	 *  @param values  A map to be filled with all the values
	 *                     for the specified document.
	 *
	 */
	static void get_all_values(const QuartzTable & table,
				       map<Xapian::valueno, string> & values,
				       Xapian::docid did);

	/** Remove all values.
	 *
	 *  @param did	The document id for which to remove the values.
	 *
	 */
	static void delete_all_values(QuartzTable &table,
					  Xapian::docid did);
};

#endif /* OM_HGUARD_QUARTZ_VALUES_H */
