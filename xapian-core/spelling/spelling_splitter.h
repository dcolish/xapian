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
#include "spelling_corrector.h"

class SpellingSplitter : public SpellingBase {

    static const unsigned TOP_SPELLING_CORRECTIONS = 3;
    static const unsigned MAX_SPLIT_COUNT = 2;
    static const unsigned MAX_MERGE_COUNT = 2;
    static const unsigned INF;

    //Key structure for states memorisation
    struct word_splitter_key {
	unsigned start;
	unsigned p_start;
	unsigned p_index;

	bool operator<(const word_splitter_key& other) const
	{
	    return start < other.start || (start == other.start && (
		   p_start < other.p_start || (p_start == other.p_start && (
		   p_index < other.p_index))));
	}
    };

    //Value structure for states memorisation
    struct word_splitter_value {
	double freq;
	unsigned next_value_index;
	unsigned start;
	unsigned index;

	bool operator<(const word_splitter_value& other) const
	{
	    return freq < other.freq - 1e-12;
	}
    };

    //Word sequence data for the splitting
    struct word_splitter_data {

	unsigned word_count;
	unsigned word_total_length;
	std::vector<unsigned> word_starts;
	std::vector<unsigned> word_lengths;
	std::vector<unsigned> word_utf_map;
	unsigned result_count;

	std::string allword;
    };

    //Temp structures for result computation
    struct word_splitter_temp {

	std::vector< std::vector< std::pair<unsigned, std::string> > > word_vector;
	std::map<word_splitter_key, std::pair<unsigned, unsigned> > memo;

	std::vector<word_splitter_value> value_vector;
    };

    unsigned max_edit_distance;
    SpellingCorrector spelling_corrector;

    double request_word_pair(const word_splitter_data& data, word_splitter_temp& temp,
                             unsigned start, unsigned index, unsigned p_start, unsigned p_index) const;

    bool request_word_exists(const word_splitter_data& data, unsigned word_start,
                             unsigned word_end, string& word) const;

    double get_sort_distance(const word_splitter_temp& temp,
                               word_splitter_value first,
                               word_splitter_value second) const;

    word_splitter_key recursive_select_words(const word_splitter_data& data,
                                             word_splitter_temp& temp,
                                             unsigned start, unsigned p_start, unsigned p_index) const;

    void find_existing_words(const word_splitter_data& data, word_splitter_temp& temp) const;

    word_splitter_key find_spelling(const std::vector<std::string>& words,
                                    word_splitter_data& data, word_splitter_temp& temp) const;

public:
    using SpellingBase::get_multiple_spelling;

    SpellingSplitter(const std::vector<Xapian::Internal::intrusive_ptr<Xapian::Database::Internal> >& internal_,
                        const std::string& prefix_, unsigned max_edit_distance_ = 0) :
                            SpellingBase(internal_, prefix_), max_edit_distance(max_edit_distance_),
                            spelling_corrector(internal_, prefix_, max_edit_distance_)
    {
    }

    double get_spelling(const std::string& word, std::string& result) const;

    double get_spelling(const std::vector<std::string>& words, std::vector<std::string>& result) const;

    void get_multiple_spelling(const std::string& word, unsigned result_count,
                               std::multimap<double, std::string, std::greater<double> >& result) const;

    void get_multiple_spelling(const std::vector<std::string>& words, unsigned result_count,
                               std::multimap<double, std::vector<std::string>, std::greater<double> >& result) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_SPLITTER_NEW_H
