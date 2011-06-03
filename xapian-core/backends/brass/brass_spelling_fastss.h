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

#ifndef XAPIAN_INCLUDED_BRASS_SPELLING_FASTSS_H
#define XAPIAN_INCLUDED_BRASS_SPELLING_FASTSS_H

#include <xapian/types.h>

#include "brass_lazytable.h"
#include "brass_spelling.h"
#include "termlist.h"

#include <map>
#include <set>
#include <vector>

#include <string>

class BrassSpellingTableFastSS : public BrassSpellingTable
{
		static const unsigned K = 2;

		class TermIndexCompare
		{
				std::vector<string>& wordlist_map;

			public:
				TermIndexCompare(const TermIndexCompare& other) :
					wordlist_map(other.wordlist_map)
				{
				}

				TermIndexCompare(std::vector<string>& wordlist_map_) :
					wordlist_map(wordlist_map_)
				{
				}

				bool operator()(unsigned first_term, unsigned second_term);
		};

		static unsigned get_data_int(const string& data, unsigned index);

		static void append_data_int(string& data, unsigned value);

		int binarySearch(const string& data, const string& word, unsigned error_mask, int start, int end, bool lower);

		void toggleTerm(const string& word, string& prefix, unsigned index, unsigned error_mask);

		void toggleRecursiveTerm(const string& word, string& prefix, unsigned index, unsigned error_mask,
				unsigned start, unsigned k, unsigned limit);

		void populateTerm(const string& word, string& prefix, unsigned error_mask);

		void populateRecursiveTerm(const string& word, string& prefix, unsigned error_mask, unsigned start, unsigned k,
				unsigned limit);

		void get_term_prefix(string& prefix, const string& word, unsigned error_mask, unsigned prefix_length);

		static unsigned pack_term_index(unsigned wordindex, unsigned error_mask);
		static void unpack_term_index(unsigned termindex, unsigned& wordindex, unsigned& error_mask);

		static int compare_string(const string& first_word, const string& second_word, unsigned first_error_mask,
				unsigned second_error_mask);

		static int compare_string(const string& first_word, const string& second_word, unsigned first_error_mask,
				unsigned second_error_mask, unsigned limit);

		std::vector<string> wordlist_map;
		std::map<std::string, std::set<unsigned, TermIndexCompare> > termlist_deltas;
		TermIndexCompare term_compare;

	protected:
		void merge_fragment_changes();

		void toggle_word(const string& word);

		void populate_word(const string& word, unsigned max_distance, std::vector<TermList*>& result);

	public:

		/** Create a new BrassSpellingTable object.
		 *
		 *  This method does not create or open the table on disk - you
		 *  must call the create() or open() methods respectively!
		 *
		 *  @param dbdir		The directory the brass database is stored in.
		 *  @param readonly		true if we're opening read-only, else false.
		 */
		BrassSpellingTableFastSS(const std::string & dbdir, bool readonly) :
			BrassSpellingTable(dbdir, readonly), wordlist_map(), termlist_deltas(), term_compare(wordlist_map)
		{
		}

		string get_word(unsigned index);

		/** Override methods of BrassTable.
		 *
		 *  NB: these aren't virtual, but we always call them on the subclass in
		 *  cases where it matters.
		 *  @{
		 */
		void cancel()
		{
			termlist_deltas.clear();
			BrassSpellingTable::cancel();
		}
		// @}
};

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_FASTSS_H
