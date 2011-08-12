/** @file brass_spelling.cc
 * @brief Spelling correction data for a brass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2011 Nikita Smetanin
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
#include <xapian/unordered_set.h>

#include "expandweight.h"
#include "brass_spelling.h"
#include "omassert.h"
#include "ortermlist.h"
#include "pack.h"

#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <set>
#include <string>
#include <limits>

using namespace Brass;
using namespace std;

const char* BrassSpellingTable::GROUPMAX_SIGNATURE = "GROUPMAX";
const char* BrassSpellingTable::GROUPSTACK_SIGNATURE = "GROUPSTACK";
const unsigned BrassSpellingTable::PREFIX_DISABLED = numeric_limits<unsigned char>::max();

void
BrassSpellingTable::merge_changes()
{
    merge_fragment_changes();

    map<string, Xapian::termcount>::const_iterator j;
    for (j = wordfreq_changes.begin(); j != wordfreq_changes.end(); ++j) {
	map<string, string>::const_iterator wv_i = wordvalue_map.find(j->first);

	if (wv_i != wordvalue_map.end())
	    set_entry_wordfreq(WORD_SIGNATURE, j->first, j->second, wv_i->second);
	else set_entry_wordfreq(WORD_SIGNATURE, j->first, j->second);
    }
    wordfreq_changes.clear();

    for (j = wordsfreq_changes.begin(); j != wordsfreq_changes.end(); ++j) {
	set_entry_wordfreq(WORDS_SIGNATURE, j->first, j->second);
    }
    wordsfreq_changes.clear();

    map<string, unsigned>::const_iterator i;
    for (i = prefix_changes.begin(); i != prefix_changes.end(); ++i) {
	string key = SPELLING_SIGNATURE + i->first;

	if (i->second != PREFIX_DISABLED) {
	    string tag;
	    pack_uint_last(tag, i->second);
	    add(key, tag);
	} else del(key);
    }
    prefix_changes.clear();

    if (prefix_index_max != 0) {
	string data;
	pack_uint_last(data, prefix_index_max);
	add(GROUPMAX_SIGNATURE, data);

	data.clear();
	for (unsigned k = 0; k < prefix_index_stack.size(); ++k)
	    pack_uint(data, prefix_index_stack[k]);
	add(GROUPSTACK_SIGNATURE, data);
    }

    prefix_index_stack.clear();
    prefix_index_max = 0;
}

void
BrassSpellingTable::add_word(const string& word,
                             Xapian::termcount freqinc, const string& prefix)
{
    if (word.size() <= 1) return;

    unsigned prefix_group = get_spelling_group(prefix);
    if (prefix_group == PREFIX_DISABLED) return;

    string prefixed_word;
    append_prefix_group(prefixed_word, prefix_group);
    prefixed_word.append(word);

    map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(prefixed_word);
    if (i != wordfreq_changes.end()) {
	// Word "word" already exists and has been modified.
	if (i->second) {
	    i->second += freqinc;
	    return;
	}
	// If "word" is currently modified such that it no longer exists, so
	// we need to execute the code below to re-add trigrams for it.
	i->second = freqinc;
    } else {
	Xapian::termcount freq = get_entry_wordfreq(WORD_SIGNATURE, prefixed_word);
	if (freq != 0) {
	    wordfreq_changes[prefixed_word] = freq + freqinc;
	    return;
	}
	wordfreq_changes[prefixed_word] = freqinc;
    }

    // New word - need to create trigrams for it.
    toggle_word(word, prefix);
}

void
BrassSpellingTable::remove_word(const string& word,
                                Xapian::termcount freqdec, const string& prefix)
{
    if (word.size() <= 1) return;

    unsigned prefix_group = get_spelling_group(prefix);
    if (prefix_group == PREFIX_DISABLED) return;

    string prefixed_word;
    append_prefix_group(prefixed_word, prefix_group);
    prefixed_word.append(word);

    map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(prefixed_word);
    if (i != wordfreq_changes.end()) {
	if (i->second == 0) {
	    // Word has already been deleted.
	    return;
	}
	// Word "word" exists and has been modified.
	if (freqdec < i->second) {
	    i->second -= freqdec;
	    return;
	}

	// Mark word as deleted.
	i->second = 0;
    } else {
	Xapian::termcount freq = get_entry_wordfreq(WORD_SIGNATURE, prefixed_word);
	if (freq == 0) return;

	if (freqdec < freq) {
	    wordfreq_changes[prefixed_word] = freq - freqdec;
	    return;
	}
	// Mark word as deleted.
	wordfreq_changes[prefixed_word] = 0;
    }

    // Remove trigrams for word.
    toggle_word(word, prefix);
}

void
BrassSpellingTable::append_prefix_group(std::string& data, unsigned value)
{
    for (unsigned i = 0; i < sizeof(unsigned char); ++i) {
	data.push_back(value & 0xFF);
	value >>= 8;
    }
}

string
BrassSpellingTable::pack_words(const string& prefix,
                               const string& first_word,
                               const string& second_word) const
{
    unsigned prefix_group = get_spelling_group(prefix);
    if (prefix_group == PREFIX_DISABLED) return string();

    hash<const string&> hasher;

    string value;
    unsigned first_hash = hasher(first_word);
    unsigned second_hash = hasher(second_word);

    if (first_word > second_word) swap(first_hash, second_hash);

    pack_uint_last(value, get_spelling_group(prefix));
    pack_uint_last(value, first_hash);
    pack_uint_last(value, second_hash);

    return value;
}

void
BrassSpellingTable::add_words(const string& first_word,
                              const string& second_word,
                              Xapian::termcount freqinc, const string& prefix)
{
    if (second_word.empty()) return add_word(first_word, freqinc);
    if (first_word.empty()) return add_word(second_word, freqinc);

    string prefixed_word = pack_words(prefix, first_word, second_word);
    if (prefixed_word.empty()) return;

    map<string, Xapian::termcount>::iterator i = wordsfreq_changes.find(prefixed_word);
    if (i == wordsfreq_changes.end()) {
	Xapian::termcount freq = get_entry_wordfreq(WORDS_SIGNATURE, prefixed_word);
	wordsfreq_changes[prefixed_word] = freq + freqinc;
    } else i->second += freqinc;
}

void
BrassSpellingTable::remove_words(const string& first_word,
                                 const string& second_word,
                                 Xapian::termcount freqdec, const string& prefix)
{
    string prefixed_word = pack_words(prefix, first_word, second_word);
    if (prefixed_word.empty()) return;

    map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(prefixed_word);
    if (i == wordfreq_changes.end()) {
	Xapian::termcount freq = get_entry_wordfreq(WORDS_SIGNATURE, prefixed_word);
	wordfreq_changes[prefixed_word] = freq - min(freqdec, freq);
    } else i->second -= min(freqdec, i->second);
}

TermList*
BrassSpellingTable::open_termlist(const string & word, const string& prefix)
{
    return open_termlist(word, word.size(), prefix);
}

struct TermListGreaterApproxSize {
    bool operator()(const TermList *a, const TermList *b)
    {
	return a->get_approx_size() > b->get_approx_size();
    }
};

TermList *
BrassSpellingTable::open_termlist(const string & word, unsigned max_distance, const string& prefix)
{
    // This should have been handled by Database::get_spelling_suggestion().
    AssertRel(word.size(),>,1);

    // Merge any pending changes to disk, but don't call commit() so they
    // won't be switched live.
    if (!wordfreq_changes.empty()) merge_changes();

    // Build a priority queue of TermList objects which returns those of
    // greatest approximate size first.
    priority_queue<TermList*, vector<TermList*> , TermListGreaterApproxSize> pq;
    try {
	vector<TermList*> result;
	result.reserve(word.size());

	populate_word(word, prefix, max_distance, result);

	for (size_t i = 0; i < result.size(); ++i)
	    pq.push(result[i]);

	if (pq.empty()) return NULL;

	// Build up an OrTermList tree by combine leaves and/or branches in
	// pairs.  The tree is balanced by the approximated sizes of the leaf
	// BrassSpellingTermList objects - the way the tree is built are very
	// similar to how an optimal Huffman code is often constructed.
	//
	// Balancing the tree like this should tend to minimise the amount of
	// work done.
	while (pq.size() > 1) {
	    // Build the tree such that left is always >= right so that
	    // OrTermList can rely on this when trying to minimise work.
	    TermList * termlist = pq.top();
	    pq.pop();

	    termlist = new OrTermList(pq.top(), termlist);
	    pq.pop();
	    pq.push(termlist);
	}

	return pq.top();
    } catch (...) {
	// Make sure we delete all the TermList objects to avoid leaking
	// memory.
	while (!pq.empty()) {
	    delete pq.top();
	    pq.pop();
	}
	throw;
    }
}

Xapian::doccount
BrassSpellingTable::get_word_frequency(const string& word,
                                       const string& prefix) const
{
    unsigned prefix_group = get_spelling_group(prefix);
    if (prefix_group == PREFIX_DISABLED) return 0;

    string prefixed_word;
    append_prefix_group(prefixed_word, prefix_group);
    prefixed_word.append(word);

    map<string, Xapian::termcount>::const_iterator i;
    i = wordfreq_changes.find(prefixed_word);
    if (i != wordfreq_changes.end()) {
	// Modified frequency for word:
	return i->second;
    }
    return get_entry_wordfreq(WORD_SIGNATURE, prefixed_word);
}

Xapian::doccount
BrassSpellingTable::get_words_frequency(const string& first_word,
                                        const string& second_word,
                                        const string& prefix) const
{
    if (second_word.empty()) return get_word_frequency(first_word);
    if (first_word.empty()) return get_word_frequency(second_word);

    string prefixed_word = pack_words(prefix, first_word, second_word);
    if (prefixed_word.empty()) return 0;

    map<string, Xapian::termcount>::const_iterator i = wordsfreq_changes.find(prefixed_word);
    if (i != wordsfreq_changes.end()) {
	// Modified frequency for word:
	return i->second;
    }
    return get_entry_wordfreq(WORDS_SIGNATURE, prefixed_word);
}

void
BrassSpellingTable::enable_spelling(const string& prefix, const string& group_prefix)
{
    unsigned spelling_group = get_spelling_group(group_prefix);
    unsigned current_spelling_group = get_spelling_group(prefix);

    if (spelling_group == current_spelling_group && spelling_group != PREFIX_DISABLED) return;

    if (spelling_group == PREFIX_DISABLED) {
	if (prefix_index_max == 0)
	    prefix_index_max = get_prefix_data(prefix_index_stack);

	if (prefix_index_stack.empty()) {
	    if (prefix_index_max == PREFIX_DISABLED)
		throw Xapian::DatabaseError("Can't add new entry - prefix index is too large");
	    spelling_group = prefix_index_max++;
	} else {
	    spelling_group = prefix_index_stack.back();
	    prefix_index_stack.pop_back();
	}
    }
    prefix_changes.insert(make_pair(prefix, spelling_group));
}

void
BrassSpellingTable::disable_spelling(const string& prefix)
{
    unsigned spelling_group = get_spelling_group(prefix);
    if (spelling_group == PREFIX_DISABLED) return;

    prefix_changes.insert(make_pair(prefix, PREFIX_DISABLED));

    if (prefix_index_max == 0)
	prefix_index_max = get_prefix_data(prefix_index_stack);

    if (spelling_group == prefix_index_max - 1)
	prefix_index_max--;
    else prefix_index_stack.push_back(spelling_group);
}

bool
BrassSpellingTable::is_spelling_enabled(const string& prefix) const
{
    return get_spelling_group(prefix) != PREFIX_DISABLED;
}

unsigned
BrassSpellingTable::get_spelling_group(const string& prefix) const
{
    if (prefix.empty()) return 0;

    map<string, unsigned>::const_iterator i = prefix_changes.find(prefix);
    if (i != prefix_changes.end()) return i->second;

    string data;
    if (get_exact_entry(SPELLING_SIGNATURE + prefix, data)) {
	unsigned group;
	const char* start = data.data();
	const char* end = start + data.size();
	if (!unpack_uint_last(&start, end, &group))
	    throw Xapian::DatabaseCorruptError("Bad spelling prefix group");
	return group;
    }
    return PREFIX_DISABLED;
}

void
BrassSpellingTable::set_entry_wordfreq(char prefix, const string& word,
                                       Xapian::termcount freq, const string& extra_value)
{
    string key = prefix + word;
    if (freq != 0) {
	string tag;
	pack_uint(tag, freq);
	tag.append(extra_value);
	add(key, tag);
    } else del(key);
}

Xapian::termcount
BrassSpellingTable::get_entry_wordfreq(char prefix, const string& word) const
{
    string key = prefix + word;
    string data;
    if (get_exact_entry(key, data)) {
	// Word "word" already exists.
	Xapian::termcount freq;
	const char *p = data.data();
	if (!unpack_uint(&p, p + data.size(), &freq) || freq == 0) {
	    throw Xapian::DatabaseCorruptError("Bad spelling word freq");
	}
	return freq;
    }
    return 0;
}

unsigned
BrassSpellingTable::get_prefix_data(vector<unsigned>& index_stack) {
    string data;

    index_stack.clear();
    if (get_exact_entry(GROUPSTACK_SIGNATURE, data)) {
	const char* start = data.data();
	const char* end = start + data.size();

	unsigned index;
	while (start != end) {
	    if (!unpack_uint(&start, end, &index))
		throw Xapian::DatabaseCorruptError("Bad spelling prefix index stack.");
	    index_stack.push_back(index);
	}
    }

    unsigned index_max = 1;
    if (get_exact_entry(GROUPMAX_SIGNATURE, data)) {
	const char* start = data.data();
	const char* end = start + data.size();
	if (!unpack_uint_last(&start, end, &index_max) || index_max == 0)
	    throw Xapian::DatabaseCorruptError("Bad spelling group index max value.");
    }
    return index_max;
}

void
BrassSpellingTable::set_word_value(const string& word, const string& value)
{
    wordvalue_map.insert(make_pair(word, value));
}

string
BrassSpellingTable::get_word_value(const string& word) const
{
    map<string, string>::const_iterator it = wordvalue_map.find(word);
    if (it != wordvalue_map.end()) return it->second;

    string data;
    string result;
    if (get_exact_entry(WORD_SIGNATURE + word, data)) {
	const char* start = data.data();
	const char* end = start + data.size();
	Xapian::termcount freq;

	if (!unpack_uint(&start, end, &freq) || freq == 0)
	    throw Xapian::DatabaseCorruptError("Bad spelling word freq");
	result.assign(start, size_t(end - start));
    }
    return result;
}
