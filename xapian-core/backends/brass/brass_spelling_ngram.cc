/** @file brass_spelling_ngram.cc
 * @brief N-Gram spelling correction data for a brass database.
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
#include "brass_spelling_ngram.h"
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

void BrassSpellingTableNGram::toggle_fragment(fragment frag,
					      const string & word)
{
    map<fragment, set<string> >::iterator i = termlist_deltas.find(frag);
    if (i == termlist_deltas.end()) {
	i = termlist_deltas.insert(make_pair(frag, set<string> ())).first;
    }

    // The commonest case is that we're adding lots of words, so try insert
    // first and if that reports that the word already exists, remove it.
    pair<set<string>::iterator, bool> res = i->second.insert(word);
    if (!res.second) {
	// word is already in the set, so remove it.
	i->second.erase(res.first);
    }
}

void BrassSpellingTableNGram::merge_fragment_changes()
{
    map<fragment, set<string> >::const_iterator i;
    for (i = termlist_deltas.begin(); i != termlist_deltas.end(); ++i) {
	string key = i->first;
	const set<string> & changes = i->second;

	set<string>::const_iterator d = changes.begin();
	if (d == changes.end())
	    continue;

	string updated;
	string current;
	PrefixCompressedStringWriter out(updated);
	if (get_exact_entry(key, current)) {
	    PrefixCompressedStringItor in(current);
	    updated.reserve(current.size()); // FIXME plus some?
	    while (!in.at_end() && d != changes.end()) {
		const string & word = *in;
		Assert(d != changes.end());
		int cmp = word.compare(*d);
		if (cmp < 0) {
		    out.append(word);
		    ++in;
		} else if (cmp > 0) {
		    out.append(*d);
		    ++d;
		} else {
		    // If an existing entry is in the changes list, that means
		    // we should remove it.
		    ++in;
		    ++d;
		}
	    }
	    if (!in.at_end()) {
		// FIXME : easy to optimise this to a fix-up and substring copy.
		while (!in.at_end()) {
		    out.append(*in++);
		}
	    }
	}

	while (d != changes.end()) {
	    out.append(*d++);
	}

	if (!updated.empty()) {
	    add(key, updated);
	} else {
	    del(key);
	}
    }
    termlist_deltas.clear();
}

void BrassSpellingTableNGram::toggle_word(const string& word, const string&)
{
    fragment buf;

    // Head:
    buf[0] = 'H';
    buf[1] = word[0];
    buf[2] = word[1];
    buf[3] = '\0';
    toggle_fragment(buf, word);

    // Tail:
    buf[0] = 'T';
    buf[1] = word[word.size() - 2];
    buf[2] = word[word.size() - 1];
    buf[3] = '\0';
    toggle_fragment(buf, word);

    if (word.size() <= 4) {
	// We also generate 'bookends' for two, three, and four character
	// terms so we can handle transposition of the middle two characters
	// of a four character word, substitution or deletion of the middle
	// character of a three character word, or insertion in the middle of a
	// two character word.
	// 'Bookends':
	buf[0] = 'B';
	buf[1] = word[0];
	buf[3] = '\0';
	toggle_fragment(buf, word);
    }
    if (word.size() > 2) {
	// Middles:
	buf[0] = 'M';
	for (size_t start = 0; start <= word.size() - 3; ++start) {
	    for (int i = 0; i < 3; ++i)
		buf[i + 1] = word[start + i];
	    toggle_fragment(buf, word);
	}
    }
}

void BrassSpellingTableNGram::populate_word(const string& word, const string&, unsigned,
					    vector<TermList*>& result)
{
    string data;
    fragment buf;

    // Head:
    buf[0] = 'H';
    buf[1] = word[0];
    buf[2] = word[1];
    if (get_exact_entry(string(buf), data))
	result.push_back(new BrassSpellingTermListNGram(data));

    // Tail:
    buf[0] = 'T';
    buf[1] = word[word.size() - 2];
    buf[2] = word[word.size() - 1];
    if (get_exact_entry(string(buf), data))
	result.push_back(new BrassSpellingTermListNGram(data));

    if (word.size() <= 4) {
	// We also generate 'bookends' for two, three, and four character
	// terms so we can handle transposition of the middle two
	// characters of a four character word, substitution or deletion of
	// the middle character of a three character word, or insertion in
	// the middle of a two character word.
	buf[0] = 'B';
	buf[1] = word[0];
	buf[3] = '\0';
	if (get_exact_entry(string(buf), data))
	    result.push_back(new BrassSpellingTermListNGram(data));
    }

    if (word.size() > 2) {
	// Middles:
	buf[0] = 'M';
	for (size_t start = 0; start <= word.size() - 3; ++start) {
	    //			memcpy(buf.data + 1, word.data() + start, 3);
	    for (int i = 0; i < 3; ++i)
		buf[i + 1] = word[start + i];
	    if (get_exact_entry(string(buf), data))
		result.push_back(new BrassSpellingTermListNGram(data));
	}

	if (word.size() == 3) {
	    // For three letter words, we generate the two "single
	    // transposition" forms too, so that we can produce good
	    // spelling suggestions.
	    // ABC -> BAC
	    buf[1] = word[1];
	    buf[2] = word[0];
	    if (get_exact_entry(string(buf), data))
		result.push_back(new BrassSpellingTermListNGram(data));
	    // ABC -> ACB
	    buf[1] = word[0];
	    buf[2] = word[2];
	    buf[3] = word[1];
	    if (get_exact_entry(string(buf), data))
		result.push_back(new BrassSpellingTermListNGram(data));
	}
    } else {
	Assert(word.size() == 2);
	// For two letter words, we generate H and T terms for the
	// transposed form so that we can produce good spelling
	// suggestions.
	// AB -> BA
	buf[0] = 'H';
	buf[1] = word[1];
	buf[2] = word[0];
	if (get_exact_entry(string(buf), data))
	    result.push_back(new BrassSpellingTermListNGram(data));
	buf[0] = 'T';
	if (get_exact_entry(string(buf), data))
	    result.push_back(new BrassSpellingTermListNGram(data));
    }
}

///////////////////////////////////////////////////////////////////////////

Xapian::termcount BrassSpellingTermListNGram::get_approx_size() const
{
    // This is only used to decide how to build a OR-tree of TermList objects
    // so we just need to return "sizes" which are ordered roughly correctly.
    return data.size();
}

std::string BrassSpellingTermListNGram::get_termname() const
{
    return current_term;
}

Xapian::termcount BrassSpellingTermListNGram::get_wdf() const
{
    return 1;
}

Xapian::doccount BrassSpellingTermListNGram::get_termfreq() const
{
    return 1;
}

Xapian::termcount BrassSpellingTermListNGram::get_collection_freq() const
{
    return 1;
}

TermList *
BrassSpellingTermListNGram::next()
{
    if (p == data.size()) {
	p = 0;
	data.resize(0);
	return NULL;
    }
    if (!current_term.empty()) {
	if (p == data.size())
	    throw Xapian::DatabaseCorruptError("Bad spelling termlist");
	current_term.resize(byte(data[p++]) ^ MAGIC_XOR_VALUE);
    }
    size_t add;
    if (p == data.size() || (add = byte(data[p]) ^ MAGIC_XOR_VALUE)
	    >= data.size() - p)
	throw Xapian::DatabaseCorruptError("Bad spelling termlist");
    current_term.append(data.data() + p + 1, add);
    p += add + 1;
    return NULL;
}

TermList *
BrassSpellingTermListNGram::skip_to(const string & term)
{
    while (!data.empty() && current_term < term) {
	(void)BrassSpellingTermListNGram::next();
    }
    return NULL;
}

bool BrassSpellingTermListNGram::at_end() const
{
    return data.empty();
}

Xapian::termcount BrassSpellingTermListNGram::positionlist_count() const
{
    throw Xapian::UnimplementedError("BrassSpellingTermList::positionlist_count() not implemented");
}

Xapian::PositionIterator BrassSpellingTermListNGram::positionlist_begin() const
{
    throw Xapian::UnimplementedError("BrassSpellingTermList::positionlist_begin() not implemented");
}
