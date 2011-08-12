/** @file spelling_transliteration.cc
 * @brief Spelling transliteration.
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
#include <vector>
#include <xapian/unicode.h>

#include "spelling_transliteration.h"
#include "spelling_transliteration_alphabets.h"

using namespace std;
using namespace Xapian;

SpellingTransliterationImpl::SpellingTransliterationImpl(const string& language_name_,
                                                 const string& language_code_) :
                                                 language_name(language_name_),
                                                 language_code(language_code_)
{
}

bool
SpellingTransliterationImpl::is_default(unsigned ch) const
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

void
SpellingTransliterationImpl::add_char_mapping(unsigned lang_char, const char* sequence)
{
    char_map[Unicode::tolower(lang_char)].push_back(sequence);
}

string
SpellingTransliterationImpl::get_transliteration(const string& word) const
{
    string result;
    for (Utf8Iterator it(word); it != Utf8Iterator(); ++it) {
	map<unsigned, vector<const char*> >::const_iterator char_it = char_map.find(Unicode::tolower(*it));
	if (char_it == char_map.end()) {
	    if (!is_default(*it)) return string();

	    result.push_back(*it);
	} else result.append(char_it->second.front());
    }
    return result;
}

vector<string>
SpellingTransliterationImpl::get_transliterations(const string& word) const
{
    vector<string> result;
    result.push_back(string());
    for (Utf8Iterator it(word); it != Utf8Iterator(); ++it) {
	map<unsigned, vector<const char*> >::const_iterator char_it = char_map.find(Unicode::tolower(*it));

	if (char_it != char_map.end()) {
	    unsigned result_size = result.size();
	    unsigned var_size = char_it->second.size();
	    result.reserve(result_size * var_size);

	    for (unsigned k = 1; k < var_size; ++k)
		for (unsigned i = 0; i < result_size && result.size() < MAX_TRANSLITERATIONS; ++i)
		    result.push_back(result[i]);

	    for (unsigned k = 0; k < var_size; ++k) {
		unsigned offset = result_size * k;
		for (unsigned i = 0; i < result_size && offset + i < result.size(); ++i)
		    result[offset + i].append(char_it->second[k]);
	    }
	} else {
	    if (!is_default(*it)) return vector<string>();

	    for (unsigned i = 0; i < result.size(); ++i)
		result[i].push_back(*it);
	}
    }
    return result;
}

const string&
SpellingTransliterationImpl::get_lang_name() const
{
    return language_name;
}

const string&
SpellingTransliterationImpl::get_lang_code() const
{
    return language_code;
}

SpellingTransliteration::SpellingTransliteration(const string& name) : internal(0)
{
    vector< Internal::intrusive_ptr<SpellingTransliterationImpl> > internals;
    internals.push_back(new RussianSpellingTransliteration);

    for (unsigned i = 0; i < internals.size() && internal.get() == NULL; ++i) {

	if (internals[i]->get_lang_name() == name || internals[i]->get_lang_code() == name)
	    internal = internals[i];
    }

    if (internal.get() == NULL)
	internal = new EnglishSpellingTransliteration;
}

SpellingTransliteration::SpellingTransliteration(SpellingTransliterationImpl* impl) : internal(impl)
{
}

string
SpellingTransliteration::get_transliteration(const string& word) const
{
    if (internal.get() == NULL || word.empty()) return string();

    return internal->get_transliteration(word);
}

vector<string>
SpellingTransliteration::get_transliterations(const string& word) const
{
    if (internal.get() == NULL || word.empty()) return vector<string>();

    return internal->get_transliterations(word);
}
