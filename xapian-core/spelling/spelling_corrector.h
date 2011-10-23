/** @file spelling_corrector.h
 * @brief Spelling word group corrector.
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

#ifndef XAPIAN_INCLUDED_SPELLING_CORRECTOR_H
#define XAPIAN_INCLUDED_SPELLING_CORRECTOR_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "database.h"

#include "editdistance.h"
#include "extended_edit_distance.h"

#include "spelling_keyboard.h"
#include "spelling_transliteration.h"
#include "spelling_base.h"

class SpellingCorrector : public SpellingBase {

    static const unsigned LIMIT_CORRECTIONS = 5;
    static const unsigned MAX_GAP = 1;
    static const unsigned INF;

    //Key for states memorisation
    struct word_spelling_key {
	unsigned first_word_index;
	unsigned first_spelling_index;
	unsigned second_word_index;
	unsigned second_spelling_index;

	bool operator<(const word_spelling_key& other) const
	{
	    return first_word_index < other.first_word_index ||
		(first_word_index == other.first_word_index && 
		 (first_spelling_index < other.first_spelling_index ||
		  (first_spelling_index == other.first_spelling_index &&
		   (second_word_index < other.second_word_index ||
		    (second_word_index == other.second_word_index &&
		     (second_spelling_index < other.second_spelling_index))))));
	}
    };

    struct word_corrector_key {
	unsigned word_index;
	unsigned key_start;
	unsigned key_end;
	const std::vector<unsigned>& key_vector;

	word_corrector_key(const std::vector<unsigned>& key_vector_) : key_vector(key_vector_)
	{
	}

	bool operator<(const word_corrector_key& other) const
	{
	    if (word_index != other.word_index)
		return word_index < other.word_index;

	    unsigned key_count = key_end - key_start;
	    unsigned o_key_count = other.key_end - other.key_start;

	    for (unsigned i = 0; i < min(key_count, o_key_count); ++i) {
		unsigned key = key_vector[i + key_start];
		unsigned o_key = other.key_vector[i + other.key_start];

		if (key != o_key) return key < o_key;
	    }

	    return key_count < o_key_count;
	}
    };

    struct word_corrector_value {
	double freq;
	unsigned next_value_index;
	unsigned word_index;
	unsigned spelling_index;
    };

    struct word_corrector_data {
	std::vector<std::vector<std::string> > word_corrections;
	unsigned result_count;
    };

    struct word_corrector_temp {
	std::vector<unsigned> word_spelling;
	std::map<word_spelling_key, double> freq_map;
	std::map<word_corrector_key, pair<unsigned, unsigned> > memo;
	std::vector<word_corrector_value> value_vector;
	std::vector<unsigned> key_vector;
    };

    unsigned max_edit_distance;
    Xapian::SpellingKeyboard keyboard;
    Xapian::SpellingTransliteration translit;

    double get_spelling_freq(const word_corrector_data& data,
                             const word_corrector_temp& temp,
			     unsigned index) const;

    double get_spelling_freq(const word_corrector_data& data,
                             word_corrector_temp& temp,
			     unsigned first_index,
			     unsigned second_index) const;

    double get_sort_distance(const word_corrector_temp& temp,
                               word_corrector_value first,
                               word_corrector_value second) const;

    word_corrector_key recursive_spelling_corrections(const word_corrector_data& data,
                                                      word_corrector_temp& temp,
                                                      unsigned word_index) const;

    word_corrector_key find_spelling(const std::vector<std::string>& words,
                                     word_corrector_data& data,
                                     word_corrector_temp& temp) const;

public:
    using SpellingBase::get_multiple_spelling;

    SpellingCorrector(const std::vector<Xapian::Internal::intrusive_ptr<Xapian::Database::Internal> >& internal_,
                      const std::string& prefix_, unsigned max_edit_distance_,
                      const Xapian::SpellingKeyboard& keyboard_ = Xapian::SpellingKeyboard(),
                      const Xapian::SpellingTransliteration& translit_ = Xapian::SpellingTransliteration()) :
                	  SpellingBase(internal_, prefix_), max_edit_distance(max_edit_distance_),
                	  keyboard(keyboard_), translit(translit_)
    {
    }

    void get_top_spelling_corrections(const std::string& word, unsigned top, bool use_freq, bool skip_exact,
				      std::vector<std::string>& result) const;

    double get_spelling(const std::string& words, std::string& result) const;

    double get_spelling(const std::vector<std::string>& words, std::vector<std::string>& result) const;

    void get_multiple_spelling(const std::string& word, unsigned result_count,
                               std::multimap<double, std::string, std::greater<double> >& result) const;

    void get_multiple_spelling(const std::vector<std::string>& words, unsigned result_count,
                               std::multimap<double, std::vector<std::string>, std::greater<double> >& result) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_CORRECTOR_H
