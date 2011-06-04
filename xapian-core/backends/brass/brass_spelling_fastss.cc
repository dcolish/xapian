/** @file brass_spelling.cc
 * @brief Spelling correction data for a brass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#include <xapian/error.h>
#include <xapian/types.h>

#include "expandweight.h"
#include "brass_spelling_fastss.h"
#include "omassert.h"
#include "ortermlist.h"
#include "pack.h"
#include <xapian/unicode.h>

#include "../prefix_compressed_strings.h"

#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <set>
#include <string>

#include <iostream>

using namespace Brass;
using namespace Xapian;
using namespace Unicode;
using namespace std;

bool BrassSpellingTableFastSS::TermIndexCompare::operator()(unsigned first_term, unsigned second_term)
{
	unsigned first_wordindex;
	unsigned first_error_mask;

	unpack_term_index(first_term, first_wordindex, first_error_mask);

	unsigned second_wordindex;
	unsigned second_error_mask;

	unpack_term_index(second_term, second_wordindex, second_error_mask);

	return compare_string(wordlist_map[first_wordindex], wordlist_map[second_wordindex], first_error_mask,
			second_error_mask) < 0;
}

unsigned BrassSpellingTableFastSS::pack_term_index(unsigned wordindex, unsigned error_mask)
{
	const unsigned shift = (sizeof(unsigned) * 8 * 3) / 4;
	return (wordindex & ((1 << shift) - 1)) | (error_mask << shift);
}

void BrassSpellingTableFastSS::unpack_term_index(unsigned termindex, unsigned& wordindex, unsigned& error_mask)
{
	const unsigned shift = (sizeof(unsigned) * 8 * 3) / 4;
	wordindex = termindex & ((1 << shift) - 1);
	error_mask = termindex >> shift;
}

void BrassSpellingTableFastSS::get_term_prefix(const string& word, string& prefix, unsigned error_mask,
		unsigned prefix_length)
{
	for (unsigned i = 0; i < word.size() && prefix.size() < prefix_length; ++i, error_mask >>= 1)
	{
		if (~error_mask & 1) prefix.push_back(word[i]);
	}
}

int BrassSpellingTableFastSS::compare_string(const string& first_word, const string& second_word,
		unsigned first_error_mask, unsigned second_error_mask)
{
	return compare_string(first_word, second_word, first_error_mask, second_error_mask, max(first_word.size(),
			second_word.size()));
}

int BrassSpellingTableFastSS::compare_string(const string& first_word, const string& second_word,
		unsigned first_error_mask, unsigned second_error_mask, unsigned limit)
{
	unsigned first_i = 0;
	unsigned second_i = 0;
	const unsigned first_end = min(first_word.size(), limit);
	const unsigned second_end = min(second_word.size(), limit);

	for (;; ++first_i, ++second_i, first_error_mask >>= 1, second_error_mask >>= 1)
	{
		while ((first_error_mask & 1) && first_i < first_end)
		{
			first_error_mask >>= 1;
			++first_i;
		}

		while ((second_error_mask & 1) && second_i < second_end)
		{
			second_error_mask >>= 1;
			++second_i;
		}

		if (first_i == first_end && second_i == second_end) return 0;
		if (first_i == first_end && second_i < second_end) return -1;
		if (first_i < first_end && second_i == second_end) return 1;

		if (first_word[first_i] < second_word[second_i]) return -1;
		if (first_word[first_i] > second_word[second_i]) return 1;
	}
}

void BrassSpellingTableFastSS::merge_fragment_changes()
{
	for (unsigned i = 0; i < wordlist_map.size(); ++i)
	{
		add("WI" + i, wordlist_map[i]);
	}

	string data;

	map<string, set<unsigned, TermIndexCompare> >::const_iterator it;
	for (it = termlist_deltas.begin(); it != termlist_deltas.end(); ++it)
	{
		data.clear();
		set<unsigned, TermIndexCompare>::const_iterator set_it;

		for (set_it = it->second.begin(); set_it != it->second.end(); ++set_it)
		{
			append_data_int(data, *set_it);
		}
		add("I" + it->first, data);
	}
	wordlist_map.clear();
	termlist_deltas.clear();
}

void BrassSpellingTableFastSS::toggle_word(const string& word)
{
	wordlist_map.push_back(word);
	unsigned index = wordlist_map.size() - 1;

	string prefix;
	toggle_recursive_term(word, prefix, index, 0, 0, 2, 8);
}

void BrassSpellingTableFastSS::toggle_term(const string& word, string& prefix, unsigned index, unsigned error_mask)
{
	prefix.clear();
	get_term_prefix(word, prefix, error_mask, 3);

	map<string, set<unsigned, TermIndexCompare> >::iterator it = termlist_deltas.find(prefix);

	if (it == termlist_deltas.end())
	{
		set<unsigned, TermIndexCompare> empty_set(term_compare);
		it = termlist_deltas.insert(make_pair(prefix, empty_set)).first;
	}
	it->second.insert(pack_term_index(index, error_mask));
}

void BrassSpellingTableFastSS::toggle_recursive_term(const string& word, string& prefix, unsigned index,
		unsigned error_mask, unsigned start, unsigned k, unsigned limit)
{
	if (k == 0) return;

	toggle_term(word, prefix, index, error_mask);

	for (unsigned i = start; i < min(word.size(), limit); ++i)
	{
		unsigned current_error_mask = error_mask | (1 << i);
		toggle_recursive_term(word, prefix, index, current_error_mask, i + 1, k - 1, limit);
	}
}

unsigned BrassSpellingTableFastSS::get_data_int(const string& data, unsigned index)
{
	unsigned result = 0;
	for (unsigned i = 0; i < sizeof(unsigned); ++i)
	{
		result |= data[index * sizeof(unsigned) + i] << (i * 8);
	}
	return result;
}

void BrassSpellingTableFastSS::append_data_int(string& data, unsigned value)
{
	for (unsigned i = 0; i < sizeof(unsigned); ++i)
	{
		data.push_back(value & 0xFF);
		value >>= 8;
	}
}

void BrassSpellingTableFastSS::populate_term(const string& word, string& prefix, unsigned error_mask,
		vector<TermList*>& result)
{
	prefix.clear();
	get_term_prefix(word, prefix, error_mask, 3);

	string data;
	if (get_exact_entry("I" + prefix, data))
	{
		//		int lower = term_binary_search(data, word, error_mask, 0, data.size() / sizeof(unsigned), true);
		//		int upper = term_binary_search(data, word, error_mask, lower, data.size() / sizeof(unsigned), false);

		vector<string> result_list;
		//		result_list.reserve(upper - lower);

		for (unsigned i = 0; i < data.size() / sizeof(unsigned); ++i)
		{
			unsigned value = get_data_int(data, i);

			unsigned current_index;
			unsigned current_error_mask;
			unpack_term_index(value, current_index, current_error_mask);

			string buf;
			get_exact_entry("WI" + current_index, buf);
			result_list.push_back(buf);
		}
		result.push_back(new BrassSpellingFastSSTermList(result_list));
	}
}

unsigned BrassSpellingTableFastSS::term_binary_search(const string& data, const string& word, unsigned error_mask,
		unsigned start, unsigned end, bool lower)
{
	unsigned count = end - start;

	string current_word;
	while (count > 0)
	{
		unsigned current = start;
		unsigned step = count / 2;
		current += step;

		unsigned value = get_data_int(data, current);

		unsigned current_index;
		unsigned current_error_mask;
		unpack_term_index(value, current_index, current_error_mask);

		get_exact_entry("WI" + current_index, current_word);

		int result = compare_string(current_word, word, current_error_mask, error_mask, 8);
		if ((lower && result < 0) || (!lower && result <= 0))
		{
			start = ++current;
			count -= step + 1;
		}
		else count = step;
	}
	return start;
}

void BrassSpellingTableFastSS::populate_recursive_term(const string& word, string& prefix, unsigned error_mask,
		unsigned start, unsigned k, unsigned limit, vector<TermList*>& result)
{
	if (k == 0) return;

	populate_term(word, prefix, error_mask, result);

	for (unsigned i = start; i < min(word.size(), limit); ++i)
	{
		unsigned current_error_mask = error_mask | (1 << i);
		populate_recursive_term(word, prefix, current_error_mask, i + 1, k - 1, limit, result);
	}
}

void BrassSpellingTableFastSS::populate_word(const string& word, unsigned max_distance, vector<TermList*>& result)
{
	string prefix;
	populate_recursive_term(word, prefix, 0, 0, min(max_distance, (unsigned) 2), 8, result);

	//	vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Xapian::termcount BrassSpellingFastSSTermList::get_approx_size() const
{
	return words.size();
}

std::string BrassSpellingFastSSTermList::get_termname() const
{
	return words[index - 1];
}

Xapian::termcount BrassSpellingFastSSTermList::get_wdf() const
{
	return 1;
}

Xapian::doccount BrassSpellingFastSSTermList::get_termfreq() const
{
	return 1;
}

Xapian::termcount BrassSpellingFastSSTermList::get_collection_freq() const
{
	return 1;
}

TermList *
BrassSpellingFastSSTermList::next()
{
	if (index < words.size()) ++index;
	return NULL;
}

TermList *
BrassSpellingFastSSTermList::skip_to(const string & term)
{
	while (index < words.size() && words[index] != term)
		++index;
	return NULL;
}

bool BrassSpellingFastSSTermList::at_end() const
{
	return index == words.size();
}

Xapian::termcount BrassSpellingFastSSTermList::positionlist_count() const
{
	throw Xapian::UnimplementedError("BrassSpellingTermList::positionlist_count() not implemented");
}

Xapian::PositionIterator BrassSpellingFastSSTermList::positionlist_begin() const
{
	throw Xapian::UnimplementedError("BrassSpellingTermList::positionlist_begin() not implemented");
}
