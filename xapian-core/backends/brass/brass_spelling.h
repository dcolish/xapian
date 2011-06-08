/** @file brass_spelling.h
 * @brief Spelling correction data for a brass database.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_BRASS_SPELLING_H
#define XAPIAN_INCLUDED_BRASS_SPELLING_H

#include <xapian/types.h>

#include "brass_lazytable.h"
#include "termlist.h"

#include <map>
#include <set>
#include <vector>

#include <string>

class BrassSpellingTable : public BrassLazyTable
{
		void set_entry_wordfreq(const string& word, Xapian::termcount freq);
		Xapian::termcount get_entry_wordfreq(const string& word) const;

		std::map<std::string, Xapian::termcount> wordfreq_changes;

	protected:
		virtual void merge_fragment_changes() = 0;

		virtual void toggle_word(const string& word) = 0;

		virtual void populate_word(const string& word, unsigned max_distance, std::vector<TermList*>& result) = 0;

	public:
		/** Create a new BrassSpellingTable object.
		 *
		 *  This method does not create or open the table on disk - you
		 *  must call the create() or open() methods respectively!
		 *
		 *  @param dbdir		The directory the brass database is stored in.
		 *  @param readonly		true if we're opening read-only, else false.
		 */
		BrassSpellingTable(const std::string & dbdir, bool readonly) :
			BrassLazyTable("spelling", dbdir + "/spelling.", readonly, Z_DEFAULT_STRATEGY)
		{
		}

		// Merge in batched-up changes.
		void merge_changes();

		void add_word(const std::string & word, Xapian::termcount freqinc);
		void remove_word(const std::string & word, Xapian::termcount freqdec);

		TermList * open_termlist(const std::string & word);

		TermList * open_termlist(const std::string & word, unsigned max_distance);

		Xapian::doccount get_word_frequency(const std::string & word) const;

		/** Override methods of BrassTable.
		 *
		 *  NB: these aren't virtual, but we always call them on the subclass in
		 *  cases where it matters.
		 *  @{
		 */

		bool is_modified() const
		{
			return !wordfreq_changes.empty() || BrassTable::is_modified();
		}

		void flush_db()
		{
			merge_changes();
			BrassTable::flush_db();
		}

		void cancel()
		{
			// Discard batched-up changes.
			wordfreq_changes.clear();

			BrassTable::cancel();
		}
		// @}
};

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_H
