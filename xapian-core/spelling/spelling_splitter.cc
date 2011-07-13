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
#include "spelling_splitter.h"

using namespace std;
using namespace Xapian;

double
SpellingSplitter::request_pair(const word_splitter_data& data, word_splitter_temp& temp) const
{
    word_splitter_temp::word_freq_key word_key;

    if (temp.word_range.size() > 0) {
	word_key.first = temp.word_range[0];

	if (temp.word_range.size() > 1) word_key.second = temp.word_range[1];
    }

    temp.first_string.clear();
    temp.second_string.clear();

    map<word_splitter_temp::word_freq_key, double>::const_iterator it = temp.word_freq_map.find(word_key);

    if (it == temp.word_freq_map.end()) {
	if (temp.word_range.size() > 0) {
	    unsigned first_start_i = data.word_utf_map[word_key.first.first];
	    unsigned first_end_i = data.word_utf_map[word_key.first.second];

	    temp.first_string.assign(data.allword, first_start_i, first_end_i - first_start_i);

	    if (temp.word_range.size() > 1) {
		unsigned second_start_i = data.word_utf_map[word_key.second.first];
		unsigned second_end_i = data.word_utf_map[word_key.second.second];

		temp.second_string.assign(data.allword, second_start_i, second_end_i - second_start_i);
	    }
	}
	double value = request_internal(temp.first_string, temp.second_string);
	it = temp.word_freq_map.insert(make_pair(word_key, value)).first;
    }

    return it->second;
}

void
SpellingSplitter::generate_word_splitter_result(const word_splitter_data& data, const word_splitter_temp& temp,
 					        word_splitter_value value, vector<string>& result) const
{
    result.clear();
    string word;

    while (true) {
	for (unsigned i = value.word_stack_range.first + 1; i < value.word_stack_range.second; ++i) {
	    unsigned start_index = data.word_utf_map[temp.word_ranges[i - 1]];
	    unsigned end_index = data.word_utf_map[temp.word_ranges[i]];
	    word.assign(data.allword, start_index, end_index - start_index);
	    result.push_back(word);
	}

	if (!value.has_next) break;
	value = temp.word_recursive_map.at(value.next_key);
    }
}

SpellingSplitter::word_splitter_key
SpellingSplitter::get_splitter_key(const word_splitter_temp& temp,
                                   unsigned word_index) const
{
    const unsigned INF = std::numeric_limits<unsigned>::max();
    const unsigned size = temp.word_stack.size();

    word_splitter_key key;

    key.first = size > 0 ? temp.word_stack[size - 1] : INF;
    key.second = size > 1 ? temp.word_stack[size - 2] : INF;
    key.index = word_index;
    return key;
}

SpellingSplitter::word_splitter_value
SpellingSplitter::recursive_word_splits(const word_splitter_data& data,
				        word_splitter_temp& temp,
				        unsigned word_index,
				        unsigned word_offset, unsigned k,
					unsigned split_start,
					unsigned merge_count) const
{
    unsigned word_start = data.word_starts[word_index];
    unsigned word_length = data.word_lengths[word_index];
    bool last_word = word_index == data.word_count - 1;

    temp.word_stack.push_back(word_start + word_length);
    word_splitter_value max_value = recursive_word_splitter(data, temp, word_index + 1, word_offset, 0);
    temp.word_stack.pop_back();

    if (!last_word && merge_count < MAX_MERGE_COUNT) {
	word_splitter_value merge_value = recursive_word_splitter(data, temp, word_index + 1, word_offset, merge_count
		+ 1);

	if (merge_value.word_freq > max_value.word_freq) max_value = merge_value;
    }

    if (k != 0) {
	for (unsigned i = split_start; i < word_length; ++i) {
	    temp.word_stack.push_back(word_start + i);
	    word_splitter_value split_value = recursive_word_splits(data, temp, word_index, word_offset, k - 1, i + 1,
								    0);
	    temp.word_stack.pop_back();

	    if (split_value.word_freq > max_value.word_freq) max_value = split_value;
	}
    }
    return max_value;
}

SpellingSplitter::word_splitter_value
SpellingSplitter::recursive_word_splitter(const word_splitter_data& data,
					  word_splitter_temp& temp,
					  unsigned word_index,
					  unsigned word_offset,
					  unsigned merge_count) const
{
    word_splitter_value value;
    value.word_freq = 0;

    unsigned new_word_offset = word_offset;

    if (word_index == data.word_count && temp.word_stack.size() <= N) {
	++new_word_offset;
	temp.word_range.clear();
	for (unsigned i = 1; i < temp.word_stack.size(); ++i) {
	    unsigned start_index = temp.word_stack[i - 1];
	    unsigned end_index = temp.word_stack[i];

	    temp.word_range.push_back(make_pair(start_index, end_index));
	}
	value.word_freq += request_pair(data, temp);
    }

    for (; new_word_offset + N < temp.word_stack.size(); ++new_word_offset) {
	temp.word_range.clear();
	for (unsigned i = new_word_offset + 1; i <= new_word_offset + N; ++i) {
	    unsigned start_index = temp.word_stack[i - 1];
	    unsigned end_index = temp.word_stack[i];

	    temp.word_range.push_back(make_pair(start_index, end_index));
	}
	value.word_freq += request_pair(data, temp);
    }

    value.has_next = word_index < data.word_count;
    if (new_word_offset > word_offset) {
	unsigned range_count = min(new_word_offset + N, temp.word_stack.size()) - word_offset;
	if (value.has_next) range_count -= min(range_count, N - 1);

	unsigned range_start = temp.word_temp_ranges.size();
	value.word_stack_range = make_pair(range_start, range_start + range_count);

	for (unsigned i = 0; i < range_count; ++i)
	    temp.word_temp_ranges.push_back(temp.word_stack[word_offset + i]);
    } else value.word_stack_range = make_pair(0, 0);

    if (value.has_next) {
	value.next_key = get_splitter_key(temp, word_index + 1);

	map<word_splitter_key, word_splitter_value>::const_iterator it = temp.word_recursive_map.find(value.next_key);

	if (it == temp.word_recursive_map.end()) {
	    unsigned range_size = temp.word_temp_ranges.size();
	    unsigned split_count = min(data.word_lengths[word_index] / 4, MAX_SPLIT_COUNT);

	    word_splitter_value next_value = recursive_word_splits(data, temp, word_index, new_word_offset,
								   split_count, 1, merge_count);

	    unsigned& range_start = next_value.word_stack_range.first;
	    unsigned& range_end = next_value.word_stack_range.second;

	    unsigned new_start = temp.word_ranges.size();
	    unsigned new_end = new_start + range_end - range_start;

	    for (unsigned i = range_start; i < range_end; ++i)
		temp.word_ranges.push_back(temp.word_temp_ranges[i]);

	    range_start = new_start;
	    range_end = new_end;

	    temp.word_temp_ranges.resize(range_size);

	    it = temp.word_recursive_map.insert(make_pair(value.next_key, next_value)).first;
	}
	value.word_freq += it->second.word_freq;
    }
    return value;
}

double
SpellingSplitter::get_spelling(const string& word, string& result) const
{
    result = word;
    return request_internal(word);
}

double
SpellingSplitter::get_spelling(const vector<string>& words, vector<string>& result) const
{
    word_splitter_data data;

    unsigned word_start = 0;
    unsigned byte_start = 0;
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
	data.allword.append(words[i]);

	byte_start += words[i].size();
    }
    data.word_utf_map.push_back(byte_start);

    data.word_count = words.size();

    word_splitter_temp temp;

    temp.word_stack.push_back(0);
    word_splitter_value value = recursive_word_splitter(data, temp, 0, 0, 0);

    unsigned& range_start = value.word_stack_range.first;
    unsigned& range_end = value.word_stack_range.second;

    unsigned new_start = temp.word_ranges.size();
    unsigned new_end = new_start + range_end - range_start;

    for (unsigned k = range_start; k < range_end; ++k)
	temp.word_ranges.push_back(temp.word_temp_ranges[k]);

    range_start = new_start;
    range_end = new_end;

    generate_word_splitter_result(data, temp, value, result);

    return value.word_freq;
}
