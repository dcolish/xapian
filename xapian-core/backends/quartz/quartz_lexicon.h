/* quartz_lexicon.h: Lexicon in a quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#ifndef USE_LEXICON
#error quartz_lexicon.h should not get included with USE_LEXICON
#endif

#ifndef OM_HGUARD_QUARTZ_LEXICON_H
#define OM_HGUARD_QUARTZ_LEXICON_H

#include <string>

#include "quartz_table.h"
#include "om/omtypes.h"

using namespace std;

/** The lexicon in a quartz database.
 *  The lexicon stores an entry for each term in the database.  This entry
 *  contains:
 *   - a termid, for use in accessing a posting list or a position
 *     list for that term.
 *   - the term frequency of the term (ie, how many documents contain
 *     the term.)
 */
class QuartzLexicon {
    private:
	/** Parse an entry from the lexicon.
	 *
	 *  @param data     The data stored in the tag in the lexicon table.
	 *  @param termfreq A pointer to a value which is filled with the
	 *                  term frequency, if the term is found.  If the
	 *                  pointer is 0, the termfreq read is discarded
	 */
	static void parse_entry(const string & data, om_doccount * termfreq);

	/** Make an entry to go into the lexicon.
	 */
	static void make_entry(string & data,
			       om_doccount termfreq);

    public:
	/** Add an occurrence of a term within a document to the lexicon.
	 *
	 *  If the term already exists, this merely increases the term
	 *  frequency for that term - if the term doesn't exist yet a new
	 *  entry is created for it
	 *
	 *  @param table   The table which the lexicon is stored in.
	 *  @param tname   The term to add.
	 */
	static void increment_termfreq(QuartzBufferedTable * table,
				       const string & tname);

	/** Remove an entry from the lexicon.  If the entry
	 *  doesn't already exist, no action is taken.
	 */
	static void decrement_termfreq(QuartzBufferedTable * table,
				       const string & tname);


	/** Get an entry from the lexicon.
	 *
	 *  @param table    The table which the lexicon is stored in.
	 *  @param tname    The term being looked up in the lexicon.
	 *  @param termfreq A pointer to a value which is filled with the
	 *                  term frequency, if the term is found.  If the
	 *                  pointer is 0, the termfreq read is discarded
	 *
	 *  @return       true if term was found in lexicon, false
	 *                otherwise.
	 */
	static bool get_entry(const QuartzTable * table,
			      const string & tname,
			      om_doccount * termfreq);
};

#endif /* OM_HGUARD_QUARTZ_LEXICON_H */
