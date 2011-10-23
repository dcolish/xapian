/** @file brass_spelling_fastss.cc
 * @brief FastSS spelling correction algorithm for a brass database.
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

#include "brass_spelling_fastss.h"

#include "xapian/error.h"
#include "xapian/types.h"
#include "xapian/unicode.h"

#include "pack.h"

#include <algorithm>
#include <vector>
#include <string>
#include <limits>

using namespace Brass;
using namespace Xapian;
using namespace Xapian::Unicode;
using namespace std;

const char* BrassSpellingTableFastSS::WORD_INDEX_SIGNATURE = "X";
const char* BrassSpellingTableFastSS::INDEXMAX_SIGNATURE = "INDEXMAX";
const char* BrassSpellingTableFastSS::INDEXSTACK_SIGNATURE = "INDEXSTACK";

bool
BrassSpellingTableFastSS::TermIndexCompare::operator()(termindex first_term, termindex second_term)
{
    unpack_term_index(first_term, first_word_index, first_error_mask);
    unpack_term_index(second_term, second_word_index, second_error_mask);

    const vector<unsigned>& first_word = wordlist_map[first_word_index];
    const vector<unsigned>& second_word = wordlist_map[second_word_index];

    return compare_string(first_word.begin(), first_word.end(), second_word.begin(), second_word.end(),
			  first_error_mask, second_error_mask, numeric_limits<unsigned>::max()) < 0;
}

void
BrassSpellingTableFastSS::get_word_key(unsigned index, string& key)
{
    key.clear();
    key.append(WORD_INDEX_SIGNATURE);
    pack_uint_last(key, index);
}

unsigned
BrassSpellingTableFastSS::check_index(unsigned index)
{
    const unsigned shift = sizeof(termindex) * 8 - LIMIT;
    if (index > (1 << shift) - 1)
	throw Xapian::DatabaseError("Can't add new entry - index is too large");
    return index;
}

BrassSpellingTableFastSS::termindex
BrassSpellingTableFastSS::pack_term_index(unsigned wordindex,
                                          unsigned error_mask)
{
    const unsigned shift = sizeof(termindex) * 8 - LIMIT;
    return (termindex(wordindex) & ((1 << shift) - 1)) | (error_mask << shift);
}

void
BrassSpellingTableFastSS::unpack_term_index(termindex termindex,
                                            unsigned& wordindex,
                                            unsigned& error_mask)
{
    const unsigned shift = sizeof(termindex) * 8 - LIMIT;
    wordindex = unsigned(termindex & ((1 << shift) - 1));
    error_mask = unsigned(termindex >> shift);
}

BrassSpellingTableFastSS::termindex
BrassSpellingTableFastSS::get_data_termindex(const string& data,
                                       unsigned index)
{
    unsigned result = 0;
    for (unsigned i = 0; i < sizeof(termindex); ++i) {
	result |= unsigned(byte(data[index * sizeof(termindex) + i])) << (i * 8);
    }
    return result;
}

void
BrassSpellingTableFastSS::append_data_termindex(string& data, termindex value)
{
    for (unsigned i = 0; i < sizeof(termindex); ++i) {
	data.push_back(value & 0xFF);
	value >>= 8;
    }
}

void
BrassSpellingTableFastSS::get_term_prefix(const vector<unsigned>& word, string& prefix,
                                          unsigned error_mask, unsigned prefix_length) const
{
    prefix_length += prefix.size();
    for (unsigned i = 0; i < word.size() && prefix.size() < prefix_length; ++i, error_mask >>= 1) {
	if (~error_mask & 1) append_utf8(prefix, word[i]);
    }
}

void
BrassSpellingTableFastSS::merge_fragment_changes()
{
    unsigned index_max = 0;
    vector<unsigned> index_stack;

    //Load index value from which we should start assign new indexes (if index stack is empty)
    string databuffer;
    if (get_exact_entry(INDEXMAX_SIGNATURE, databuffer)) {
	const char* start = databuffer.data();
	const char* end = start + databuffer.size();
	unpack_uint_last(&start, end, &index_max);
    }

    //Load stack of free indexes which should be assigned to new words.
    if (get_exact_entry(INDEXSTACK_SIGNATURE, databuffer)) {
	const char* start = databuffer.data();
	const char* end = start + databuffer.size();

	unsigned index;
	while (start != end) {
	    if (!unpack_uint(&start, end, &index))
		throw Xapian::DatabaseCorruptError("Bad spelling fastss index stack.");
	    index_stack.push_back(index);
	}
    }

    vector<unsigned> wordlist_index_map(wordlist_deltas.size(), 0);

    //Merge word list
    string key;
    string word;
    string key_word;
    for (unsigned i = 0; i < wordlist_deltas.size(); ++i) {
	const vector<unsigned>& word_utf = wordlist_deltas[i];
	unsigned prefix_group = wordlist_deltas_prefixes[i];

	word.clear();
	for (unsigned j = 0; j < word_utf.size(); ++j)
	    append_utf8(word, word_utf[j]);

	key_word.clear();
	append_prefix_group(key_word, prefix_group);
	key_word.append(word);

	string word_value = get_word_value(key_word);

	//If new word already exists, we should remove it
	if (!word_value.empty()) {
	    unsigned index;
	    const char* start = word_value.data();
	    const char* end = start + word_value.size();
	    unpack_uint_last(&start, end, &index);

	    wordlist_index_map[i] = index;
	    index_stack.push_back(index);

	    get_word_key(index, key);
	    set_word_value(key_word, string());
	    del(key);
	} else {
	    //Else assign new index and add word to the database.
	    unsigned index;
	    if (!index_stack.empty()) {
		index = index_stack.back();
		index_stack.pop_back();
	    } else index = check_index(index_max++);
	    wordlist_index_map[i] = index;

	    get_word_key(index, key);
	    add(key, word);
	    word_value.clear();
	    pack_uint_last(word_value, index);
	    set_word_value(key_word, word_value);
	}
    }

    //Store index_max value
    databuffer.clear();
    pack_uint_last(databuffer, index_max);
    add(INDEXMAX_SIGNATURE, databuffer);

    //Store index_stack value
    databuffer.clear();
    for (unsigned i = 0; i < index_stack.size(); ++i)
	pack_uint(databuffer, index_stack[i]);
    add(INDEXSTACK_SIGNATURE, databuffer);

    TermIndexCompare term_index_compare(wordlist_deltas);

    string new_databuffer;
    new_databuffer.reserve(databuffer.size() * 2);

    string key_buffer;
    string existing_word_buffer;
    string merging_word_buffer;

    //Merge terms (for each prefix)
    map<string, vector<unsigned> >::iterator it;
    for (it = termlist_deltas.begin(); it != termlist_deltas.end(); ++it) {
	vector<termindex>& merging_data = it->second;
	sort(merging_data.begin(), merging_data.end(), term_index_compare);

	if (!get_exact_entry(it->first, databuffer)) databuffer.clear();

	unsigned existing_data_length = databuffer.size() / sizeof(unsigned);
	unsigned merging_data_length = merging_data.size();

	if (merging_data_length == 0) continue;

	unsigned existing_i = 0;
	unsigned merging_i = 0;
	bool update_existing = true;
	bool update_merging = true;

	termindex existing_value = 0;
	unsigned existing_word_index = 0;
	unsigned existing_error_mask = 0;

	termindex merging_value = 0;
	unsigned merging_word_index = 0;
	unsigned merging_error_mask = 0;

	new_databuffer.clear();

	//Merge terms as sorted lists.
	while (existing_i < existing_data_length || merging_i < merging_data_length) {
	    bool existing_has_more = existing_i < existing_data_length;
	    bool merging_has_more = merging_i < merging_data_length;

	    if (existing_has_more && update_existing) {
		existing_value = get_data_termindex(databuffer, existing_i);
		unpack_term_index(existing_value, existing_word_index, existing_error_mask);
		get_word(existing_word_index, key_buffer, existing_word_buffer);
		update_existing = false;
	    }

	    if (merging_has_more && update_merging) {
		merging_value = merging_data[merging_i];
		unpack_term_index(merging_value, merging_word_index, merging_error_mask);
		merging_word_index = wordlist_index_map[merging_word_index];
		merging_value = pack_term_index(merging_word_index, merging_error_mask);
		get_word(merging_word_index, key_buffer, merging_word_buffer);
		update_merging = false;
	    }

	    if (existing_has_more && merging_has_more) {
		if (existing_value == merging_value) {
		    ++existing_i;
		    ++merging_i;
		    update_existing = true;
		    update_merging = true;
		} else {
		    int compare_result = compare_string((Utf8Iterator(existing_word_buffer)), Utf8Iterator(),
							(Utf8Iterator(merging_word_buffer)), Utf8Iterator(),
							existing_error_mask, merging_error_mask, numeric_limits<
								unsigned>::max());

		    if (compare_result < 0) {
			append_data_termindex(new_databuffer, existing_value);
			++existing_i;
			update_existing = true;
		    } else {
			append_data_termindex(new_databuffer, merging_value);
			++merging_i;
			update_merging = true;
		    }
		}
	    } else if (existing_has_more) //Copy remaining existing data
	    {
		append_data_termindex(new_databuffer, existing_value);
		++existing_i;
		update_existing = true;
	    } else if (merging_has_more) //Copy remaining merging data
	    {
		append_data_termindex(new_databuffer, merging_value);
		++merging_i;
		update_merging = true;
	    }
	}
	//Store new term list in database
	add(it->first, new_databuffer);
    }
    wordlist_deltas.clear();
    wordlist_deltas_prefixes.clear();
    termlist_deltas.clear();
}

void
BrassSpellingTableFastSS::toggle_word(const string& word, const string& prefix)
{
    unsigned prefix_group = get_spelling_group(prefix);
    if (prefix_group == PREFIX_DISABLED) return;

    vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());

    wordlist_deltas.push_back(word_utf);
    wordlist_deltas_prefixes.push_back(prefix_group);
    unsigned index = wordlist_deltas.size() - 1;

    string prefix_data;
    toggle_recursive_term(word_utf, prefix_data, prefix_group, index, 0, 0, 0,
                          min<size_t>(MAX_DISTANCE, word.size() / 2));
}

void
BrassSpellingTableFastSS::toggle_term(const vector<unsigned>& word, string& prefix,
                                      unsigned prefix_group, unsigned index,
                                      unsigned error_mask, bool update_prefix)
{
    if (update_prefix) {
	prefix.clear();
	prefix.push_back(PREFIX_SIGNATURE);
	append_prefix_group(prefix, prefix_group);
	get_term_prefix(word, prefix, error_mask, PREFIX_LENGTH);
    }

    map<string, vector<termindex> >::iterator it = termlist_deltas.find(prefix);

    if (it == termlist_deltas.end())
	it = termlist_deltas.insert(make_pair(prefix, vector<termindex>())).first;
    it->second.push_back(pack_term_index(index, error_mask));
}

void
BrassSpellingTableFastSS::toggle_recursive_term(const vector<unsigned>& word, string& prefix,
                                                unsigned prefix_group, unsigned index, unsigned error_mask,
                                                unsigned start, unsigned distance, unsigned max_distance)
{
    bool update_prefix = start <= PREFIX_LENGTH + distance;
    toggle_term(word, prefix, prefix_group, index, error_mask, update_prefix);

    if (distance < max_distance)
	for (unsigned i = start; i < min<size_t>(word.size(), LIMIT); ++i) {
	    unsigned current_error_mask = error_mask | (1 << i);
	    toggle_recursive_term(word, prefix, prefix_group, index,
	                          current_error_mask, i + 1, distance + 1, max_distance);
	}
}

unsigned
BrassSpellingTableFastSS::term_binary_search(const string& data, const vector<unsigned>& word,
                                             unsigned error_mask, unsigned start, unsigned end, bool lower) const
{
    unsigned count = end - start;

    unsigned current_index;
    unsigned current_error_mask;

    string key;
    string current_word;

    while (count > 0) {
	unsigned current = start;
	unsigned step = count / 2;
	current += step;

	termindex current_value = get_data_termindex(data, current);
	unpack_term_index(current_value, current_index, current_error_mask);

	get_word(current_index, key, current_word);

	int result = compare_string((Utf8Iterator(current_word)), Utf8Iterator(), word.begin(), word.end(),
				    current_error_mask, error_mask, LIMIT);

	if ((lower && result < 0) || (!lower && result <= 0)) {
	    start = ++current;
	    count -= step + 1;
	} else count = step;
    }
    return start;
}

void
BrassSpellingTableFastSS::populate_term(const vector<unsigned>& word, string& data, string& prefix,
                                        unsigned prefix_group, unsigned error_mask,
                                        bool update_prefix, unordered_set<unsigned>& result) const
{
    if (update_prefix) {
	prefix.clear();
	prefix.push_back(PREFIX_SIGNATURE);
	append_prefix_group(prefix, prefix_group);
	get_term_prefix(word, prefix, error_mask, PREFIX_LENGTH);
	if (!get_exact_entry(prefix, data)) data.clear();
    }

    if (!data.empty()) {
	unsigned length = data.size() / sizeof(unsigned);
	unsigned lower = term_binary_search(data, word, error_mask, 0, length, true);
	unsigned upper = term_binary_search(data, word, error_mask, lower, length, false);

	unsigned current_index;
	unsigned current_error_mask;

	for (unsigned i = lower; i < upper; ++i) {
	    termindex current_value = get_data_termindex(data, i);
	    unpack_term_index(current_value, current_index, current_error_mask);
	    result.insert(current_index);
	}
    }
}

void
BrassSpellingTableFastSS::populate_recursive_term(const vector<unsigned>& word, string& data, string& prefix,
                                                  unsigned prefix_group, unsigned error_mask,
                                                  unsigned start, unsigned distance,
						  unsigned max_distance, unordered_set<unsigned>& result) const
{
    bool update_prefix = start <= PREFIX_LENGTH + distance;
    populate_term(word, data, prefix, prefix_group, error_mask, update_prefix, result);

    if (distance < max_distance) {
	for (unsigned i = start; i < min<size_t>(word.size(), LIMIT); ++i) {
	unsigned current_error_mask = error_mask | (1 << i); // generate error mask - place one-bits at "error" positions
	populate_recursive_term(word, data, prefix, prefix_group, current_error_mask,
	                        i + 1, distance + 1, max_distance, result);
	}
    }
}

void
BrassSpellingTableFastSS::populate_word(const string& word, const string& prefix,
                                        unsigned max_distance, vector<TermList*>& result) const
{
    unsigned prefix_group = get_spelling_group(prefix);
    if (prefix_group == PREFIX_DISABLED) return;

    vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());

    string prefix_data;
    string data;
    unordered_set<unsigned> result_set;
    populate_recursive_term(word_utf, data, prefix_data, prefix_group, 0, 0, 0,
			    min<size_t>(min<size_t>(
						 max_distance, MAX_DISTANCE),
					  word.size() / 2), result_set);

    result.push_back(new BrassSpellingFastSSTermList(result_set.begin(), result_set.end(), *this));
}

bool
BrassSpellingTableFastSS::get_word(unsigned index, string& key, string& word) const
{
    key.clear();
    get_word_key(index, key);
    return get_exact_entry(key, word);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Xapian::termcount
BrassSpellingFastSSTermList::get_approx_size() const
{
    return words.size();
}

string
BrassSpellingFastSSTermList::get_termname() const
{
    return word;
}

Xapian::termcount
BrassSpellingFastSSTermList::get_wdf() const
{
    return 1;
}

Xapian::doccount
BrassSpellingFastSSTermList::get_termfreq() const
{
    return 1;
}

Xapian::termcount
BrassSpellingFastSSTermList::get_collection_freq() const
{
    return 1;
}

TermList *
BrassSpellingFastSSTermList::next()
{
    if (index < words.size())
	table.get_word(words[index], key_buffer, word);

    ++index;
    return NULL;
}

TermList *
BrassSpellingFastSSTermList::skip_to(const string & term)
{
    while (index < words.size() && word != term)
	BrassSpellingFastSSTermList::next();

    return NULL;
}

bool
BrassSpellingFastSSTermList::at_end() const
{
    return index > words.size();
}

Xapian::termcount
BrassSpellingFastSSTermList::positionlist_count() const
{
    throw Xapian::UnimplementedError("BrassSpellingTermList::positionlist_count() not implemented");
}

Xapian::PositionIterator
BrassSpellingFastSSTermList::positionlist_begin() const
{
    throw Xapian::UnimplementedError("BrassSpellingTermList::positionlist_begin() not implemented");
}
