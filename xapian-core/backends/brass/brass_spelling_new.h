/** @file brass_spelling_new.h
 * @brief N-gram based optimised spelling correction algorithm for a brass database.
 */
/* Copyright (C) 2011 Nikita Smetanin
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

#include "brass_spelling_ngram.h"

#include <string>
#include <vector>

class BrassSpellingTableNew : public BrassSpellingTableNGram
{
		static const int NGRAM_SIZE = 3;
		static const char PLACEHOLDER = '$';
		static const char NGRAM_SIGNATURE = 'N';

		void populate_ngram_word(const std::vector<unsigned>& word, unsigned max_distance, std::string& str_buf,
				std::string& data, std::vector<TermList*>& result);

		void populate_action(const std::string& str_buf, std::string& data, std::vector<TermList*>& result);

	protected:
		void toggle_word(const std::string& word);

		void populate_word(const std::string& word, unsigned max_distance, std::vector<TermList*>& result);

	public:
		BrassSpellingTableNew(const std::string & dbdir, bool readonly) :
			BrassSpellingTableNGram(dbdir, readonly)
		{
		}
};

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_NEW_H
