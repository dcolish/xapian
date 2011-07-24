/** @file spelling_splitter.cc
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

#include <config.h>
#include <limits>
#include <vector>
#include <algorithm>
#include <map>
#include <xapian/unicode.h>
#include <iostream>
#include "database.h"
#include "spelling_splitter_new.h"
#include "spelling_corrector.h"

using namespace std;
using namespace Xapian;

const unsigned SpellingSplitterNew::INF = numeric_limits<unsigned>::max();

double
SpellingSplitterNew::request_word_pair(const word_splitter_data&,
                                       word_splitter_temp& temp,
                                       unsigned start,
                                       unsigned index,
                                       unsigned p_start,
                                       unsigned p_index) const
{
    string word;
    if (start != INF)
	word = temp.word_vector[start][index].second;

    string p_word;
    if (p_start != INF)
	p_word = temp.word_vector[p_start][p_index].second;

    return request_internal(word, p_word);
}

bool
SpellingSplitterNew::request_word_exists(const word_splitter_data& data,
                                         unsigned word_start,
                                         unsigned word_end,
                                         string& word) const
{
    unsigned word_utf_start = data.word_utf_map[word_start];
    unsigned word_utf_end = data.word_utf_map[word_end];
    word.assign(data.allword, word_utf_start, word_utf_end - word_utf_start);
    double result = request_internal(word);
    return result > 1e-12;
}

SpellingSplitterNew::word_splitter_key
SpellingSplitterNew::recursive_select_words(const word_splitter_data& data,
                                            word_splitter_temp& temp,
                                            unsigned start,
                                            unsigned p_start,
                                            unsigned p_index) const
{
    unsigned skip = start;
    while (start < data.word_total_length && temp.word_vector[start].empty())
	++start;
    skip = start - skip;

    if (skip > 0)
	p_start = p_index = INF;

    word_splitter_key key;
    key.start = start;
    key.p_start = p_start;
    key.p_index = p_index;

    if (temp.memo.find(key) != temp.memo.end()) return key;

    pair<unsigned, unsigned> values;

    if (start < data.word_total_length) {
	multimap<double, word_splitter_value, greater<double> > value_map;

	for (unsigned i = 0; i < temp.word_vector[start].size(); ++i) {
	    unsigned end = temp.word_vector[start][i].first;

	    word_splitter_key next_key = recursive_select_words(data, temp, end, start, i);
	    pair<unsigned, unsigned> next_values = temp.memo.at(next_key);

	    double pair_freq = request_word_pair(data, temp, start, i, p_start, p_index);

	    for (unsigned v = next_values.first; v < next_values.second; ++v) {
		word_splitter_value result_value;
		result_value.freq = temp.value_vector[v].freq + pair_freq;
		result_value.has_next = true;
		result_value.next_key = next_key;
		result_value.next_value_index = v;
		result_value.index = i;
		result_value.start = start;
		value_map.insert(make_pair(result_value.freq, result_value));
	    }
	}

	values.first = temp.value_vector.size();

	unsigned limit = 0;
	multimap<double, word_splitter_value, greater<double> >::const_iterator it;
	for (it = value_map.begin(); it != value_map.end() && limit < data.result_count; ++it, ++limit)
	    temp.value_vector.push_back(it->second);

	values.second = temp.value_vector.size();
    }
    else {
	word_splitter_value result_value;
	result_value.freq = 0.0;
	result_value.has_next = false;

	values.first = temp.value_vector.size();
	temp.value_vector.push_back(result_value);
	values.second = temp.value_vector.size();
    }
    return temp.memo.insert(make_pair(key, values)).first->first;
}

void
SpellingSplitterNew::find_existing_words(const word_splitter_data& data,
                                         word_splitter_temp& temp) const
{
    temp.word_vector.assign(data.word_total_length + 1, vector<pair<unsigned, string> >());

    vector<bool> begins(data.word_total_length + 1);
    vector<unsigned> splits(data.word_total_length + 1, 1);
    vector<unsigned> end_vector(data.word_total_length, 0);

    unsigned word_offset = 0;
    for (unsigned i = 0; i < data.word_count; ++i) {
	begins[word_offset] = true;
	splits[word_offset] = 0;
	unsigned word_length = data.word_lengths[i];
	fill_n(end_vector.begin() + word_offset, word_length, word_offset + word_length);
	word_offset += word_length;
    }

    for (unsigned index = 0; index < data.word_count; ++index) {
	unsigned length = data.word_lengths[index];
	unsigned merge_length = length;

	for (unsigned i = 1; i <= min(MAX_MERGE_COUNT, data.word_count - index - 1); ++i)
	    merge_length += data.word_lengths[index + i];

	unsigned offset = data.word_starts[index];

	for (unsigned start = 0; start < length; ++start) {
	    unsigned real_start = offset + start;
	    if (!begins[real_start]) continue;

	    unsigned split = splits[real_start];

	    unsigned begin_end = start + 1;
	    if (splits[real_start] >= MAX_SPLIT_COUNT)
		begin_end = max(length - 1, begin_end);

	    for (unsigned end = begin_end; end < merge_length; ++end) {
		unsigned real_end = offset + end + 1;
		if (request_word_exists(data, real_start, real_end, temp.word)) {
		    unsigned next_split = (end < length) ? (1 + split) : 1;

		    begins[real_start] = true;
		    if (!begins[real_end]) {
			begins[real_end] = true;
			splits[real_end] = next_split;
		    } else splits[real_end] = min(next_split, splits[real_end]);

		    temp.word_vector[real_start].push_back(make_pair(real_end, temp.word));
		}
	    }
	}
    }

    SpellingCorrector corrector(internal, prefix, max_edit_distance);

    vector<string> unknown_words;
    unsigned skip_end = 0;
    for (unsigned i = 0; i < data.word_total_length; ++i) {
	if (!begins[i]) continue;

	if (i == 0 || skip_end < i) {
	    skip_end = i;
	    while (skip_end < data.word_total_length && temp.word_vector[skip_end].empty())
		++skip_end;
	}
	unsigned end = min(skip_end, end_vector[i]);
	if (end == i) continue;

	unsigned word_utf_start = data.word_utf_map[i];
	unsigned word_utf_end = data.word_utf_map[end];
	temp.word.assign(data.allword, word_utf_start, word_utf_end - word_utf_start);

	unknown_words.clear();
	if (max_edit_distance > 0)
	    corrector.get_top_spelling_corrections(temp.word, TOP_SPELLING_CORRECTIONS, false, true, unknown_words);
	unknown_words.push_back(temp.word);

	for (unsigned u = 0; u < unknown_words.size(); ++u)
	    temp.word_vector[i].push_back(make_pair(end, unknown_words[u]));
    }
}

double
SpellingSplitterNew::generate_result(const word_splitter_temp& temp,
                                     word_splitter_key key,
                                     vector<string>& result) const
{
    word_splitter_value value = temp.value_vector[temp.memo.at(key).first];
    double result_freq = value.freq;

    result.clear();
    while (true) {
	if (!value.has_next) break;

	result.push_back(temp.word_vector[value.start][value.index].second);
	value = temp.value_vector[value.next_value_index];
    }
    return result_freq;
}

void
SpellingSplitterNew::generate_multiple_result(const word_splitter_temp& temp,
                                              word_splitter_key key,
                                              map<double, vector<string> >& result) const
{
    pair<unsigned, unsigned> start_values = temp.memo.at(key);

    for (unsigned i = start_values.first; i < start_values.second; ++i) {
	word_splitter_value value = temp.value_vector[i];

	vector<string>& result_vector = result[value.freq];
	while (true) {
	    if (!value.has_next) break;

	    result_vector.push_back(temp.word_vector[value.start][value.index].second);
	    value = temp.value_vector[value.next_value_index];
	}
    }
}

SpellingSplitterNew::word_splitter_key
SpellingSplitterNew::find_spelling(const std::vector<std::string>& words,
                                   word_splitter_data& data, word_splitter_temp& temp) const
{
    unsigned word_start = 0;
    unsigned byte_start = 0;
    data.word_total_length = 0;
    for (unsigned i = 0; i < words.size(); ++i) {
	unsigned word_length = 0;

	Utf8Iterator word_begin(words[i]);
	Utf8Iterator word_end;

	for (Utf8Iterator word_it = word_begin; word_it != word_end; ++word_it) {
	    unsigned byte_i = word_it.raw() - word_begin.raw();
	    data.word_utf_map.push_back(byte_start + byte_i);
	    ++word_length;
	}

	data.word_starts.push_back(word_start);
	word_start += word_length;

	data.word_lengths.push_back(word_length);
	data.word_total_length += word_length;
	data.allword.append(words[i]);

	byte_start += words[i].size();
    }
    data.word_utf_map.push_back(byte_start);
    data.word_count = words.size();

    find_existing_words(data, temp);
    return recursive_select_words(data, temp, 0, INF, INF);
}

double
SpellingSplitterNew::get_spelling(const string& word,
                                  string& result) const
{
    result = word;
    return request_internal(word);
}

double
SpellingSplitterNew::get_spelling(const vector<string>& words,
                                  vector<string>& result) const
{
    word_splitter_data data;
    word_splitter_temp temp;
    data.result_count = 1;
    word_splitter_key key = find_spelling(words, data, temp);

    return generate_result(temp, key, result);
}

void
SpellingSplitterNew::get_multiple_spelling(const vector<string>& words,
                                           unsigned top,
                                           map<double, vector<string> >& result) const
{
    word_splitter_data data;
    word_splitter_temp temp;
    data.result_count = max(top, 1u);
    word_splitter_key key = find_spelling(words, data, temp);

    return generate_multiple_result(temp, key, result);
}
