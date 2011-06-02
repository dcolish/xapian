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
#include <xapian/unicode.h>

#include "brass_spelling_new.h"

#include <algorithm>
#include <vector>
#include <string>
#include <map>

using namespace Brass;
using namespace std;

void BrassSpellingTableNew::toggle_word(const string& word)
{
	const int n = 3;
	const int end = int(word.size()) - n + 1;
	const char placeholder = 'H';

	vector<unsigned> word_utf(Xapian::Utf8Iterator(word), Xapian::Utf8Iterator());

	fragment buf;

	//start - position of n-gram in string. -1 - head, end - tail.
	for (int start = -1; start <= end; ++start)
	{
		//Store position of n-gram in string
		buf[0] = char(start + n);
		//If head, put placeholder as the first char
		buf[1] = (start >= 0) ? word[start] : placeholder;

		for (int i = 1; i < n - 1; ++i)
			buf[i + 1] = word[start + i];

		//If tail, put placeholder as the last char
		buf[n] = (start < end) ? word[start + n - 1] : placeholder;

		toggle_fragment(buf, word);
	}

	if (word.size() <= n + 1)
	{
		// We also generate 'bookends' for two, three, and four character
		// terms so we can handle transposition of the middle two
		// characters of a four character word, substitution or deletion of
		// the middle character of a three character word, or insertion in
		// the middle of a two character word. We store first and last characters.

		buf[0] = 1;

		for (int i = 1; i < n - 1; ++i)
			buf[i] = placeholder;

		buf[n - 1] = word[0];
		buf[n] = word[word.size() - 1];

		toggle_fragment(buf, word);
	}
}

void BrassSpellingTableNew::populate_word(const string& word, unsigned max_distance, vector<TermList*>& result)
{
	const int n = 3;
	const char placeholder = 'H';

	string data;
	fragment buf;

	populate_ngram_word(word, max_distance, buf, data, result);

	//'Bookends'
	if (word.size() <= n + 1)
	{
		buf[0] = 1;

		for (int i = 1; i < n - 1; ++i)
			buf[i] = placeholder;

		buf[n - 1] = word[0];
		buf[n] = word[word.size() - 1];

		populate_action(buf, data, result);
	}

	//Transpositions for short words.
	if (int(word.size()) <= n)
	{
		string word_copy = word;
		for (int i = 0; i < int(word_copy.size()) - 1; ++i)
		{
			swap(word_copy[i], word_copy[i + 1]);
			populate_ngram_word(word_copy, max_distance, buf, data, result);
			swap(word_copy[i], word_copy[i + 1]);
		}
	}
}

void BrassSpellingTableNew::populate_ngram_word(const string& word, unsigned max_distance, fragment& buf, string& data,
		vector<TermList*>& result)
{
	const int n = 3;
	const int end = int(word.size()) - n + 1;
	const char placeholder = 'H';

	for (int start = -1; start <= end; ++start)
	{
		buf[1] = (start >= 0) ? word[start] : placeholder;

		for (int i = 1; i < n - 1; ++i)
			buf[i + 1] = word[start + i];

		buf[n] = (start < end) ? word[start + n - 1] : placeholder;

		for (int i = max(start - int(max_distance), -1); i <= start + int(max_distance); ++i)
		{
			buf[0] = char(n + i);
			populate_action(buf, data, result);
		}
	}
}

void BrassSpellingTableNew::populate_action(const fragment& buf, string& data, vector<TermList*>& result)
{
	if (get_exact_entry(buf, data)) result.push_back(new BrassSpellingTermList(data));
}
