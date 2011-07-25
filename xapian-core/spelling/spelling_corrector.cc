/** @file spelling_corrector.cc
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

#include <config.h>
#include <limits>
#include <vector>
#include <map>
#include <algorithm>
#include <xapian/unicode.h>

#include "database.h"
#include "spelling_corrector.h"
#include "autoptr.h"
#include "ortermlist.h"
#include "editdistance.h"
#include "extended_edit_distance.h"

using namespace std;
using namespace Xapian;

const unsigned SpellingCorrector::INF = numeric_limits<unsigned>::max();

void
SpellingCorrector::get_top_spelling_corrections(const string& word,
                                                unsigned top, bool use_freq,
                                                bool skip_exact,
						vector<string>& result) const
{
    AutoPtr<TermList> merger;
    for (size_t i = 0; i < internal.size(); ++i) {
	TermList* term_list = internal[i]->open_spelling_termlist(word, prefix, max_edit_distance);
	if (term_list != 0) {
	    if (merger.get() != 0) {
		merger.reset(new OrTermList(merger.release(), term_list));
	    } else merger.reset(term_list);
	}
    }

    if (merger.get() == 0) return;

    vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());
    vector<unsigned> term_utf;
    multimap<double, string> top_spelling;

    ExtendedEditDistance edit_distance(keyboard_layout);

    while (true)
    {
	TermList* return_term_list = merger->next();
	if (return_term_list != 0)
	    merger.reset(return_term_list);
	if (merger->at_end())
	    break;

	string term = merger->get_termname();
	term_utf.assign((Utf8Iterator(term)), Utf8Iterator());

	if (unsigned(abs(long(term_utf.size()) - long(word_utf.size()))) > max_edit_distance)
	    continue;

	unsigned distance = edit_distance_unsigned(&term_utf[0], int(term_utf.size()), &word_utf[0],
	                                           int(word_utf.size()), max_edit_distance);
	if (distance == 0 && skip_exact) continue;

	if (distance <= max_edit_distance) {
	    double distance_precise = edit_distance.edit_distance(&term_utf[0], term_utf.size(), &word_utf[0],
	                                                          word_utf.size(), distance);

	    if (use_freq)
		distance_precise /= request_internal(term);

	    top_spelling.insert(make_pair(distance_precise, term));

	    if (top_spelling.size() > top)
		top_spelling.erase(--top_spelling.end());
	}
    }

    multimap<double, string>::const_iterator it;
    for (it = top_spelling.begin(); it != top_spelling.end(); ++it)
	result.push_back(it->second);
}

double SpellingCorrector::get_spelling_freq(const word_corrector_data& data,
                                            const word_corrector_temp& temp,
                                            unsigned index) const
{
    return request_internal(data.word_corrections[index][temp.word_spelling[index]]);
}

double
SpellingCorrector::get_spelling_freq(const word_corrector_data& data,
                                     word_corrector_temp& temp,
        			     unsigned first_index,
        			     unsigned second_index) const
{
    word_spelling_key key;
    key.first_word_index = first_index;
    key.first_spelling_index = temp.word_spelling[first_index];
    key.second_word_index = second_index;
    key.second_spelling_index = temp.word_spelling[second_index];

    map<word_spelling_key, double>::const_iterator it = temp.freq_map.find(key);
    if (it == temp.freq_map.end()) {
	double freq = request_internal(data.word_corrections[first_index][temp.word_spelling[first_index]],
	                               data.word_corrections[second_index][temp.word_spelling[second_index]]);

	it = temp.freq_map.insert(make_pair(key, freq)).first;
    }
    return it->second;
}

SpellingCorrector::word_corrector_key
SpellingCorrector::recursive_spelling_corrections(const word_corrector_data& data,
                                                  word_corrector_temp& temp,
                                                  unsigned word_index) const
{
    word_corrector_key key(temp.key_vector);
    key.word_index = word_index;
    key.key_start = temp.key_vector.size();

    for (unsigned gap = 0; gap < min(word_index, MAX_GAP + 1); ++gap)
	temp.key_vector.push_back(temp.word_spelling[word_index - gap - 1]);

    key.key_end = temp.key_vector.size();

    if (temp.memo.find(key) != temp.memo.end()) return key;

    pair<unsigned, unsigned> values;

    if (word_index < data.word_corrections.size()) {
	multimap<double, word_corrector_value, greater<double> > value_map;

	for (unsigned i = 0; i < data.word_corrections[word_index].size(); ++i) {
	    temp.word_spelling[word_index] = i;

	    double word_freq = 0.0;
	    for (unsigned gap = 0; gap < min(word_index, MAX_GAP + 1); ++gap)
		word_freq += get_spelling_freq(data, temp, word_index - gap - 1, word_index);

	    if (data.word_corrections.size() == 1)
		word_freq += get_spelling_freq(data, temp, word_index);

	    word_corrector_key next_key = recursive_spelling_corrections(data, temp, word_index + 1);
	    pair<unsigned, unsigned> next_values = temp.memo.at(next_key);

	    for (unsigned v = next_values.first; v < next_values.second; ++v) {
		word_corrector_value result_value;
		result_value.freq = temp.value_vector[v].freq + word_freq;
		result_value.next_value_index = v;
		result_value.word_index = word_index;
		result_value.spelling_index = i;

		value_map.insert(make_pair(result_value.freq, result_value));
	    }
	}

	values.first = temp.value_vector.size();

	unsigned limit = 0;
	multimap<double, word_corrector_value, greater<double> >::const_iterator it;
	for (it = value_map.begin(); it != value_map.end() && limit < data.result_count; ++it, ++limit)
	    temp.value_vector.push_back(it->second);

	values.second = temp.value_vector.size();

    } else {
	word_corrector_value result_value;
	result_value.freq = 0.0;
	result_value.next_value_index = INF;

	values.first = temp.value_vector.size();
	temp.value_vector.push_back(result_value);
	values.second = temp.value_vector.size();
    }
    return temp.memo.insert(make_pair(key, values)).first->first;
}

SpellingCorrector::word_corrector_key
SpellingCorrector::find_spelling(const vector<string>& words,
                                 word_corrector_data& data,
                                 word_corrector_temp& temp) const
{
    data.word_corrections.assign(words.size(), vector<string>());
    for (unsigned i = 0; i < words.size(); ++i) {
	data.word_corrections[i].reserve(LIMIT_CORRECTIONS + 1);
	data.word_corrections[i].push_back(words[i]);
	get_top_spelling_corrections(words[i], LIMIT_CORRECTIONS, false, true, data.word_corrections[i]);
    }

    temp.word_spelling.assign(words.size(), 0);
    return recursive_spelling_corrections(data, temp, 0);
}

double
SpellingCorrector::get_spelling(const string& word, string& result) const
{
    result.clear();
    vector<string> result_vector;
    get_top_spelling_corrections(word, 1, true, true, result_vector);

    if (result_vector.empty()) return 0.0;
    return request_internal(result_vector.front());
}

double
SpellingCorrector::get_spelling(const vector<string>& words,
                                vector<string>& result) const
{
    word_corrector_data data;
    data.result_count = 1;

    word_corrector_temp temp;
    word_corrector_key key = find_spelling(words, data, temp);

    word_corrector_value value = temp.value_vector[temp.memo.at(key).first];
    double result_freq = value.freq;

    bool exact = true;
    while (value.next_value_index != INF) {
	exact = exact && value.spelling_index == 0;
	result.push_back(data.word_corrections[value.word_index][value.spelling_index]);
	value = temp.value_vector[value.next_value_index];
    }

    if (!exact)
	return result_freq;

    result.clear();
    return 0.0;
}

void
SpellingCorrector::get_multiple_spelling(const vector<string>& words, unsigned result_count,
					 multimap<double, vector<string> >& result) const
{
    word_corrector_data data;
    data.result_count = max(result_count, 1u);

    word_corrector_temp temp;
    word_corrector_key key = find_spelling(words, data, temp);

    pair<unsigned, unsigned> start_values = temp.memo.at(key);

    for (unsigned i = start_values.first; i < start_values.second; ++i) {
	word_corrector_value value = temp.value_vector[i];

	multimap<double, vector<string> >::iterator it;
	it = result.insert(make_pair(value.freq, vector<string>()));

	vector<string>& result_vector = it->second;

	bool exact = true;
	while (value.next_value_index != INF) {
	    exact = exact && value.spelling_index == 0;
	    result_vector.push_back(data.word_corrections[value.word_index][value.spelling_index]);
	    value = temp.value_vector[value.next_value_index];
	}
	if (exact) result.erase(it);
    }
}
