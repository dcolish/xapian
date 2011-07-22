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

#ifndef XAPIAN_INCLUDED_SPELLING_SPLITTER_NEW_H
#define XAPIAN_INCLUDED_SPELLING_SPLITTER_NEW_H

#include <vector>
#include <limits>
#include "database.h"
#include "spelling_base.h"

class SpellingSplitterNew : public SpellingBase {

    static const unsigned MAX_SPLIT_COUNT = 2;
    static const unsigned MAX_MERGE_COUNT = 2;
    static const unsigned INF;

    //Word sequence data for the splitting
    struct word_splitter_data {

	unsigned word_count;
	unsigned word_total_length;
	std::vector<unsigned> word_starts;
	std::vector<unsigned> word_lengths;
	std::vector<unsigned> word_utf_map;

	std::string allword;
    };

    //Key structure for states memorisation
    struct word_extract_key {
	unsigned start;
	unsigned p_start;
	unsigned p_end;

	bool operator<(const word_extract_key& other) const
	{
	    return start < other.start || (start == other.start && (p_start < other.p_start || (p_start == other.p_start
		    && (p_end < other.p_end))));
	}
    };

    //Value structure for states memorisation
    struct word_extract_value {
	double freq;
	bool has_next;
	word_extract_key next_key;
	unsigned index;
    };

    //Temp structures for result computation
    struct word_extract_temp {

	std::vector< vector<unsigned> > word_vector;
    	std::map<word_extract_key, word_extract_value> memo;

	std::string word;
	std::string p_word;
    };

    double request_word_pair(const word_splitter_data& data, word_extract_temp& temp, unsigned start, unsigned end, unsigned p_start, unsigned p_end) const;

    bool request_word_exists(const word_splitter_data& data, unsigned word_start, unsigned word_end, string& word) const;

    word_extract_key recursive_extract_words(const word_splitter_data& data, word_extract_temp& temp, unsigned start, unsigned p_start, unsigned p_end) const;

    double generate_extract_words_result(const word_splitter_data& data, const word_extract_temp& temp, word_extract_key key, std::vector<std::string>& result) const;

    void extract_words(const word_splitter_data& data, word_extract_temp& temp) const;

public:
    SpellingSplitterNew(const std::vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> >& internal_,
                        const std::string& prefix_) : SpellingBase(internal_, prefix_)
    {
    }

    double get_spelling(const std::string& word, std::string& result) const;

    double get_spelling(const std::vector<std::string>& words, std::vector<std::string>& result) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_SPLITTER_NEW_H
