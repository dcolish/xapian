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
#include <map>
#include <xapian/unicode.h>

#include "database.h"
#include "spelling_splitter_new.h"

using namespace std;
using namespace Xapian;

const unsigned SpellingSplitterNew::INF = numeric_limits<unsigned>::max();

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

    word_splitter_temp temp;
    find_existing_words(data, temp);

    word_splitter_key key = recursive_select_words(data, temp, 0, INF, INF);
    return generate_result(data, temp, key, result);
}

double
SpellingSplitterNew::request_word_pair(const word_splitter_data& data,
                                       word_splitter_temp& temp,
                                       unsigned start,
                                       unsigned end,
                                       unsigned p_start,
                                       unsigned p_end) const
{
    if (start < end) {
	unsigned start_i = data.word_utf_map[start];
	unsigned end_i = data.word_utf_map[end];

	temp.word.assign(data.allword, start_i, end_i - start_i);

	if (p_start < p_end) {
	    unsigned p_start_i = data.word_utf_map[p_start];
	    unsigned p_end_i = data.word_utf_map[p_end];

	    temp.p_word.assign(data.allword, p_start_i, p_end_i - p_start_i);
	}
    }
    return request_internal(temp.word, temp.p_word);
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
                                            unsigned p_end) const
{
    unsigned skip = start;
    while (start < temp.word_vector.size() && temp.word_vector[start].empty())
	++start;
    skip = start - skip;

    if (skip > 0)
	p_start = p_end = INF;

    word_splitter_key key;
    key.start = start;
    key.p_start = p_start;
    key.p_end = p_end;

    map<word_splitter_key, word_splitter_value>::const_iterator it = temp.memo.find(key);
    if (it != temp.memo.end()) return it->first;

    word_splitter_value result_value;
    result_value.freq = 0.0;
    result_value.has_next = false;

    if (start < temp.word_vector.size()) {
	result_value.has_next = true;

	word_splitter_key max_next_key;
	double max_freq = 0.0;
	unsigned max_index = INF;

	for (unsigned i = 0; i < temp.word_vector[start].size(); ++i) {
	    unsigned end = temp.word_vector[start][i];

	    word_splitter_key next_key = recursive_select_words(data, temp, end, start, end);
	    word_splitter_value next_value = temp.memo.at(next_key);

	    double freq = next_value.freq + request_word_pair(data, temp, start, end, p_start, p_end);

	    if (freq > max_freq || i == 0) {
		max_freq = freq;
		max_next_key = next_key;
		max_index = i;
	    }
	}
	result_value.freq = max_freq;
	result_value.next_key = max_next_key;
	result_value.index = max_index;
    }
    return temp.memo.insert(make_pair(key, result_value)).first->first;
}

void
SpellingSplitterNew::find_existing_words(const word_splitter_data& data,
                                         word_splitter_temp& temp) const
{
    temp.word_vector.assign(data.word_total_length + 1, vector<unsigned>());

    vector<bool> begins(data.word_total_length + 1);
    unsigned word_offset = 0;
    for (unsigned i = 0; i < data.word_count; ++i) {
	begins[word_offset] = true;
	word_offset += data.word_lengths[i];
    }
    vector<unsigned> splits(data.word_total_length + 1, 0);

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

		    if (!begins[real_end]) {
			begins[real_end] = true;
			splits[real_end] = next_split;
		    } else splits[real_end] = min(next_split, splits[real_end]);

		    temp.word_vector[real_start].push_back(real_end);
		}
	    }
	}
    }
}

double
SpellingSplitterNew::generate_result(const word_splitter_data& data,
                                     const word_splitter_temp& temp,
                                     word_splitter_key key,
                                     vector<string>& result) const {
    result.clear();
    string word;

    word_splitter_value value = temp.memo.at(key);
    double result_freq = value.freq;

    while (true) {
	if (!value.has_next) break;

	unsigned start = key.start;
	unsigned end = temp.word_vector[start][value.index];

	unsigned start_i = data.word_utf_map[start];
	unsigned end_i = data.word_utf_map[end];
	word.assign(data.allword, start_i, end_i - start_i);
	result.push_back(word);

	key = value.next_key;
	value = temp.memo.at(key);
    }
    return result_freq;
}
