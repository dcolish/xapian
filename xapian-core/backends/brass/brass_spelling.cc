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
#include <xapian/unordered_set.h>

#include "expandweight.h"
#include "brass_spelling.h"
#include "omassert.h"
#include "ortermlist.h"
#include "pack.h"

#include "../prefix_compressed_strings.h"

#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <set>
#include <string>

using namespace Brass;
using namespace std;

void BrassSpellingTable::merge_changes()
{
	merge_fragment_changes();

	map<string, Xapian::termcount>::const_iterator j;
	for (j = wordfreq_changes.begin(); j != wordfreq_changes.end(); ++j)
	{
		set_entry_wordfreq(WORD_SIGNATURE, j->first, j->second);
	}
	wordfreq_changes.clear();

	for (j = wordsfreq_changes.begin(); j != wordsfreq_changes.end(); ++j)
	{
		set_entry_wordfreq(WORDS_SIGNATURE, j->first, j->second);
	}
	wordsfreq_changes.clear();
}

void BrassSpellingTable::add_word(const string & word, Xapian::termcount freqinc)
{
	if (word.size() <= 1)
		return;

	map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(word);
	if (i != wordfreq_changes.end())
	{
		// Word "word" already exists and has been modified.
		if (i->second)
		{
			i->second += freqinc;
			return;
		}
		// If "word" is currently modified such that it no longer exists, so
		// we need to execute the code below to re-add trigrams for it.
		i->second = freqinc;
	}
	else
	{
		Xapian::termcount freq = get_entry_wordfreq(WORD_SIGNATURE, word);
		if (freq != 0)
		{
			wordfreq_changes[word] = freq + freqinc;
			return;
		}
		wordfreq_changes[word] = freqinc;
	}

	// New word - need to create trigrams for it.

	toggle_word(word);
}

void BrassSpellingTable::remove_word(const string & word, Xapian::termcount freqdec)
{
	map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(word);
	if (i != wordfreq_changes.end())
	{
		if (i->second == 0)
		{
			// Word has already been deleted.
			return;
		}
		// Word "word" exists and has been modified.
		if (freqdec < i->second)
		{
			i->second -= freqdec;
			return;
		}

		// Mark word as deleted.
		i->second = 0;
	}
	else
	{
		Xapian::termcount freq = get_entry_wordfreq(WORD_SIGNATURE, word);
		if (freq == 0)
			return;

		if (freqdec < freq)
		{
			wordfreq_changes[word] = freq - freqdec;
			return;
		}
		// Mark word as deleted.
		wordfreq_changes[word] = 0;
	}

	// Remove fragment entries for word.

	toggle_word(word);
}

string BrassSpellingTable::pack_words(const string& first_word, const string& second_word)
{
	hash<const string&> hasher;

	string value;
	unsigned first_hash = hasher(first_word);
	unsigned second_hash = hasher(second_word);

	if (first_hash > second_hash)
		swap(first_hash, second_hash);

	pack_uint_last(value, first_hash);
	pack_uint_last(value, second_hash);

	return value;
}

void BrassSpellingTable::add_words(const string& first_word, const string& second_word, Xapian::termcount freqinc)
{
	if (second_word.empty())
		return add_word(first_word, freqinc);

	if (first_word.empty())
		return add_word(second_word, freqinc);

	string word = pack_words(first_word, second_word);

	map<string, Xapian::termcount>::iterator i = wordsfreq_changes.find(word);
	if (i == wordsfreq_changes.end())
	{
		Xapian::termcount freq = get_entry_wordfreq(WORDS_SIGNATURE, word);
		wordsfreq_changes[word] = freq + freqinc;
	}
	else i->second += freqinc;
}

void BrassSpellingTable::remove_words(const string& first_word, const string& second_word, Xapian::termcount freqdec)
{
	string word = pack_words(first_word, second_word);

	map<string, Xapian::termcount>::iterator i = wordfreq_changes.find(word);
	if (i == wordfreq_changes.end())
	{
		Xapian::termcount freq = get_entry_wordfreq(WORDS_SIGNATURE, word);
		wordfreq_changes[word] = freq - min(freqdec, freq);
	}
	else i->second -= min(freqdec, i->second);
}

TermList*
BrassSpellingTable::open_termlist(const string & word)
{
	return open_termlist(word, word.size());
}

struct TermListGreaterApproxSize
{
		bool operator()(const TermList *a, const TermList *b)
		{
			return a->get_approx_size() > b->get_approx_size();
		}
};

TermList *
BrassSpellingTable::open_termlist(const string & word, unsigned max_distance)
{
	// This should have been handled by Database::get_spelling_suggestion().
	AssertRel(word.size(),>,1);

	// Merge any pending changes to disk, but don't call commit() so they
	// won't be switched live.
	if (!wordfreq_changes.empty())
		merge_changes();

	// Build a priority queue of TermList objects which returns those of
	// greatest approximate size first.
	priority_queue<TermList*, vector<TermList*> , TermListGreaterApproxSize> pq;
	try
	{
		vector<TermList*> result;
		result.reserve(word.size());

		populate_word(word, max_distance, result);

		for (size_t i = 0; i < result.size(); ++i)
			pq.push(result[i]);

		if (pq.empty())
			return NULL;

		// Build up an OrTermList tree by combine leaves and/or branches in
		// pairs.  The tree is balanced by the approximated sizes of the leaf
		// BrassSpellingTermList objects - the way the tree is built are very
		// similar to how an optimal Huffman code is often constructed.
		//
		// Balancing the tree like this should tend to minimise the amount of
		// work done.
		while (pq.size() > 1)
		{
			// Build the tree such that left is always >= right so that
			// OrTermList can rely on this when trying to minimise work.
			TermList * termlist = pq.top();
			pq.pop();

			termlist = new OrTermList(pq.top(), termlist);
			pq.pop();
			pq.push(termlist);
		}

		return pq.top();
	}
	catch (...)
	{
		// Make sure we delete all the TermList objects to avoid leaking
		// memory.
		while (!pq.empty())
		{
			delete pq.top();
			pq.pop();
		}
		throw;
	}
}

Xapian::doccount BrassSpellingTable::get_word_frequency(const string & word) const
{
	map<string, Xapian::termcount>::const_iterator i;
	i = wordfreq_changes.find(word);
	if (i != wordfreq_changes.end())
	{
		// Modified frequency for word:
		return i->second;
	}
	return get_entry_wordfreq(WORD_SIGNATURE, word);
}

Xapian::doccount BrassSpellingTable::get_words_frequency(const std::string& first_word, const std::string& second_word) const
{
	if (second_word.empty())
		return get_word_frequency(first_word);
	if (first_word.empty())
		return get_word_frequency(second_word);

	string word = pack_words(first_word, second_word);

	map<string, Xapian::termcount>::const_iterator i = wordsfreq_changes.find(word);
	if (i != wordsfreq_changes.end())
	{
		// Modified frequency for word:
		return i->second;
	}
	return get_entry_wordfreq(WORDS_SIGNATURE, word);
}

void BrassSpellingTable::set_entry_wordfreq(char prefix, const string& word, Xapian::termcount freq)
{
	string key = prefix + word;
	if (freq != 0)
	{
		string tag;
		pack_uint_last(tag, freq);
		add(key, tag);
	}
	else del(key);
}

Xapian::termcount BrassSpellingTable::get_entry_wordfreq(char prefix, const string& word) const
{
	string key = prefix + word;
	string data;
	if (get_exact_entry(key, data))
	{
		// Word "word" already exists.
		Xapian::termcount freq;
		const char *p = data.data();
		if (!unpack_uint_last(&p, p + data.size(), &freq) || freq == 0)
		{
			throw Xapian::DatabaseCorruptError("Bad spelling word freq");
		}
		return freq;
	}
	return 0;
}
