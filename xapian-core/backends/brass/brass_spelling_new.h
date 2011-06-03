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

#ifndef XAPIAN_INCLUDED_BRASS_SPELLING_NEW_H
#define XAPIAN_INCLUDED_BRASS_SPELLING_NEW_H

#include <xapian/types.h>

#include "brass_spelling.h"

#include <string>
#include <vector>

class BrassSpellingTableNew : public BrassSpellingTable
{
		void populate_ngram_word(const std::vector<unsigned>& word, unsigned max_distance, std::string& str_buf,
				std::string& data, std::vector<TermList*>& result);

		void populate_action(const std::string& str_buf, string& data, std::vector<TermList*>& result);

	protected:
		void toggle_word(const std::string& word);

		void populate_word(const std::string& word, unsigned max_distance, std::vector<TermList*>& result);

	public:
		/** Create a new BrassSpellingTableNew object.
		 *
		 *  This method does not create or open the table on disk - you
		 *  must call the create() or open() methods respectively!
		 *
		 *  @param dbdir		The directory the brass database is stored in.
		 *  @param readonly		true if we're opening read-only, else false.
		 */
		BrassSpellingTableNew(const std::string & dbdir, bool readonly) :
			BrassSpellingTable(dbdir, readonly)
		{
		}
};

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_NEW_H
