/** @file spelling_splitter.h
 * @brief Spelling word group splitter and merger.
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

#ifndef XAPIAN_INCLUDED_SPELLING_SPLITTER_H
#define XAPIAN_INCLUDED_SPELLING_SPLITTER_H

#include <vector>
#include <limits>
#include "database.h"
#include "spelling_base.h"

class SpellingSplitter : public SpellingBase {

    static const unsigned N = 2;
    static const unsigned MAX_SPLIT_COUNT = 1;
    static const unsigned MAX_MERGE_COUNT = 1;

    //Word sequence data for the splitting
    struct word_splitter_data {

	unsigned word_count;
	std::vector<unsigned> word_starts;
	std::vector<unsigned> word_lengths;
	std::vector<unsigned> word_utf_map;

	std::string allword;
    };

    //Key structure for states memorisation
    struct word_splitter_key {

	unsigned index;
	unsigned first;
	unsigned second;

	bool operator<(const word_splitter_key& other) const
	{
	    return index < other.index || (index == other.index && (first < other.first || (first == other.first
		    && (second < other.second))));
	}
    };

    //Value structure for states memorisation
    struct word_splitter_value {

	unsigned word_freq;
	bool has_next;
	word_splitter_key next_key; //next key in resulting sequence
	std::pair<unsigned, unsigned> word_stack_range;
    };

    //Temp structures for result computation
    struct word_splitter_temp {

	typedef std::pair<std::pair<unsigned, unsigned>, std::pair<unsigned, unsigned> > word_freq_key;

	std::vector<unsigned> word_stack;
	std::vector<std::pair<unsigned, unsigned> > word_range;

	std::map<word_freq_key, unsigned> word_freq_map;
	std::map<word_splitter_key, word_splitter_value> word_recursive_map;

	std::vector<unsigned> word_ranges;
	std::vector<unsigned> word_temp_ranges;

	std::string first_string;
	std::string second_string;
    };

    unsigned request_pair(const word_splitter_data& data, word_splitter_temp& temp);

    void generate_word_splitter_result(const word_splitter_data& data, const word_splitter_temp& temp,
				       word_splitter_value value, std::vector<std::string>& result);

    word_splitter_key get_splitter_key(const word_splitter_temp& temp, unsigned word_index);

    word_splitter_value recursive_word_splits(const word_splitter_data& data, word_splitter_temp& temp,
					      unsigned word_index, unsigned word_offset, unsigned k,
					      unsigned split_start, unsigned merge_count);

    word_splitter_value recursive_word_splitter(const word_splitter_data& data, word_splitter_temp& temp,
						unsigned word_index, unsigned word_offset, unsigned merge_count);

public:
    SpellingSplitter(const std::vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> >& internal_) : SpellingBase(internal_)
    {
    }

    unsigned get_spelling(const std::string& word, std::string& result);

    unsigned get_spelling(const std::vector<std::string>& words, std::vector<std::string>& result);
};

#endif // XAPIAN_INCLUDED_SPELLING_SPLITTER_H
