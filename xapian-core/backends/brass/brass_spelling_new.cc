/** @file brass_spelling_new.cc
 * @brief N-gram based optimised spelling correction algorithm for a brass database.
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

#include <xapian/error.h>
#include <xapian/types.h>
#include <xapian/unicode.h>

#include "brass_spelling_new.h"

#include <algorithm>
#include <vector>
#include <string>
#include <map>

using namespace Brass;
using namespace Xapian;
using namespace Unicode;
using namespace std;

void BrassSpellingTableNew::toggle_word(const string& word, const string& prefix)
{
    vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());

    const int end = int(word_utf.size()) - NGRAM_SIZE + 1;

    const unsigned prefix_group = get_spelling_group(prefix);

    set<string> str_buf_set;

    string str_buf;
    str_buf.reserve(word_utf.size() * sizeof(unsigned));

    //start - position of n-gram in string. -1 - head, end - tail.
    for (int start = -1; start <= end; ++start) {
	str_buf.clear();
	str_buf.push_back(NGRAM_SIGNATURE);
	//Store position of n-gram in string
	str_buf.push_back(char(start + NGRAM_SIZE));
	//Append prefix group
	append_prefix_group(str_buf, prefix_group);

	//If head, put placeholder as the first char
	if (start >= 0)
	    append_utf8(str_buf, word_utf[start]);
	else
	    str_buf.push_back(PLACEHOLDER);

	for (int i = 1; i < NGRAM_SIZE - 1; ++i)
	    append_utf8(str_buf, word_utf[start + i]);

	//If tail, put placeholder as the last char
	if (start < end)
	    append_utf8(str_buf, word_utf[start + NGRAM_SIZE - 1]);
	else
	    str_buf.push_back(PLACEHOLDER);

	if (str_buf_set.insert(str_buf).second)
	    toggle_fragment(str_buf, word);
    }

    if (word_utf.size() <= NGRAM_SIZE + 1) {
	// We also generate 'bookends' for two, three, and four character
	// terms so we can handle transposition of the middle two
	// characters of a four character word, substitution or deletion of
	// the middle character of a three character word, or insertion in
	// the middle of a two character word. We store first and last characters.

	str_buf.clear();
	str_buf.push_back(NGRAM_SIGNATURE);
	str_buf.push_back(1);
	append_prefix_group(str_buf, prefix_group);
	str_buf.append(NGRAM_SIZE - 2, PLACEHOLDER);

	append_utf8(str_buf, word_utf[0]);
	append_utf8(str_buf, word_utf[word_utf.size() - 1]);

	if (str_buf_set.insert(str_buf).second)
	    toggle_fragment(str_buf, word);
    }
}

void BrassSpellingTableNew::populate_word(const string& word,
                                          const string& prefix,
					  unsigned max_distance,
					  vector<TermList*>& result)
{
    vector<unsigned> word_utf((Utf8Iterator(word)), Utf8Iterator());

    const unsigned prefix_group = get_spelling_group(prefix);

    string str_buf;
    str_buf.reserve(word_utf.size() * sizeof(unsigned));

    string data;

    populate_ngram_word(word_utf, prefix_group, max_distance, str_buf, data, result);

    //'Bookends'
    if (word_utf.size() <= NGRAM_SIZE + 1) {
	str_buf.clear();
	str_buf.push_back(NGRAM_SIGNATURE);
	str_buf.push_back(1);
	append_prefix_group(str_buf, prefix_group);
	str_buf.append(NGRAM_SIZE - 2, PLACEHOLDER);

	append_utf8(str_buf, word_utf[0]);
	append_utf8(str_buf, word_utf[word_utf.size() - 1]);

	populate_action(str_buf, data, result);
    }

    //Transpositions for short words.
    if (int(word_utf.size()) <= NGRAM_SIZE) {
	for (int i = 0; i < int(word_utf.size()) - 1; ++i) {
	    swap(word_utf[i], word_utf[i + 1]);
	    populate_ngram_word(word_utf, prefix_group, max_distance, str_buf, data, result);
	    swap(word_utf[i], word_utf[i + 1]);
	}
    }
}

void BrassSpellingTableNew::populate_ngram_word(const vector<unsigned>& word_utf,
                                                unsigned prefix_group,
						unsigned max_distance,
						string& str_buf, string& data,
						vector<TermList*>& result)
{
    const int end = int(word_utf.size()) - NGRAM_SIZE + 1;

    for (int start = -1; start <= end; ++start) {
	str_buf.clear();
	str_buf.push_back(NGRAM_SIGNATURE);
	str_buf.push_back(1);
	append_prefix_group(str_buf, prefix_group);

	if (start >= 0)
	    append_utf8(str_buf, word_utf[start]);
	else
	    str_buf.push_back(PLACEHOLDER);

	for (int i = 1; i < NGRAM_SIZE - 1; ++i)
	    append_utf8(str_buf, word_utf[start + i]);

	if (start < end)
	    append_utf8(str_buf, word_utf[start + NGRAM_SIZE - 1]);
	else
	    str_buf.push_back(PLACEHOLDER);

	for (int i = max(start - int(max_distance), -1); i <= start
		+ int(max_distance); ++i) {
	    str_buf[1] = char(NGRAM_SIZE + i);
	    populate_action(str_buf, data, result);
	}
    }
}

void BrassSpellingTableNew::populate_action(const string& str_buf,
					    string& data,
					    vector<TermList*>& result)
{
    if (get_exact_entry(str_buf, data))
	result.push_back(new BrassSpellingTermListNGram(data));
}
