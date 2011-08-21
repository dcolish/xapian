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
#include <fstream>
#include <sstream>

#include "spelling_transliteration.h"
#include "spelling_transliteration_alphabets.h"

using namespace std;
using namespace Xapian;

const SpellingTransliteration::SpellingTransliterationStatic SpellingTransliteration::static_instance;

SpellingTransliterationImpl::SpellingTransliterationImpl(const string& language_name_,
                                                 const string& language_code_) :
                                                 language_name(language_name_),
                                                 language_code(language_code_)
{
}

bool
SpellingTransliterationImpl::is_default(unsigned ch) const
{
    return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
	   (ch == '.') || (ch == ',') || (ch == '!') || (ch == '?');
}

void
SpellingTransliterationImpl::add_mapping(const string& source, const string& translit)
{
    translit_map[source].push_back(translit);
}

void
SpellingTransliterationImpl::add_reverse_mapping(const string& source, const string& translit)
{
    reverse_translit_map[source].push_back(translit);
}

void
SpellingTransliterationImpl::make_reverse_mapping()
{
    map<string, vector<string> >::const_iterator it;
    for (it = translit_map.begin(); it != translit_map.end(); ++it) {
	for (unsigned i = 0; i < it->second.size(); ++i) {
	    if (!it->second[i].empty())
		reverse_translit_map[it->second[i]].push_back(it->first);
	}
    }
}

void
SpellingTransliterationImpl::get_transliterations(const string& word,
                                                  const map<string, vector<string> >& char_map,
                                                  bool keep_default, bool limit_variants,
                                                  vector<string>& transliterations) const
{
    vector<string> result;
    result.push_back(string());

    string part;
    for (Utf8Iterator it(word); it != Utf8Iterator();) {
	map<string, vector<string> >::const_iterator char_it = char_map.end();

	part.clear();
	Utf8Iterator part_it = it;
	while (part_it != Utf8Iterator()) {
	    Unicode::append_utf8(part, Unicode::tolower(*part_it++));
	    map<string, vector<string> >::const_iterator temp_it = char_map.find(part);
	    if (temp_it == char_map.end()) break;

	    char_it = temp_it;
	    ++it;
	}

	if (char_it != char_map.end()) {
	    unsigned result_size = result.size();
	    unsigned var_size = char_it->second.size();
	    if (limit_variants) var_size = min(var_size, 1u);

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
	    if (!keep_default || !is_default(Unicode::tolower(*it))) return;

	    for (unsigned i = 0; i < result.size(); ++i)
		result[i].push_back(*it);
	    ++it;
	}
    }

    transliterations.reserve(transliterations.size() + result.size());
    for (unsigned i = 0; i < result.size(); ++i)
	if (result[i] != word) transliterations.push_back(result[i]);
}

string
SpellingTransliterationImpl::get_transliteration(const string& word) const
{
    vector<string> result;
    get_transliterations(word, translit_map, true, true, result);

    return !result.empty() ? result.front() : string();
}

vector<string>
SpellingTransliterationImpl::get_transliterations(const string& word) const
{
    vector<string> result;
    get_transliterations(word, translit_map, true, false, result);
    get_transliterations(word, reverse_translit_map, false, false, result);
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

string unicode_from_string(const string& data);

string unicode_from_string(const string& data)
{
    string result;
    stringstream datastream(data);

    unsigned ch;
    string unicode_ch;
    while (getline(datastream, unicode_ch, 'u')) {
	if (unicode_ch.empty()) continue;
	stringstream stream;
	stream << hex << unicode_ch;
	stream >> ch;
	Unicode::append_utf8(result, ch);
    }

    return result;
}

SpellingTransliterationImpl*
SpellingTransliteration::SpellingTransliterationStatic::load_transliteration(const string& language_name,
                                                                             const string& language_code) const
{
    ifstream lm(("../languages/transliteration/" + language_name + ".tr").c_str(), ifstream::in);
    if (!lm.good()) return NULL;

    SpellingTransliterationImpl* result = new SpellingTransliterationImpl(language_name,
                                                                          language_code);

    string line;
    string key;
    string value;
    while (getline(lm, line)) {
	key.clear();
	value.clear();

	Utf8Iterator it(line);
	Utf8Iterator end_it;
	if (it == end_it) continue;

	bool reverse = false;
	if ((reverse = (*it == '~'))) ++it;

	while (it != end_it && !Unicode::is_whitespace(*it))
	    Unicode::append_utf8(key, *it++);

	while (it != end_it && *it != '(') ++it;
	if (key.empty() || it == end_it) continue;

	++it;
	while (it != end_it && *it != ')') {
	    unsigned ch = *it++;
	    if (ch == '_') ch = ' ';
	    Unicode::append_utf8(value, ch);
	}

	if (!reverse)
	    result->add_mapping(unicode_from_string(key), value);
	else result->add_reverse_mapping(key, unicode_from_string(value));
    }
    lm.close();

    return result;
}

SpellingTransliteration::SpellingTransliterationStatic::SpellingTransliterationStatic()
{
    default_internal = new SpellingTransliterationImpl("english", "en");

    ifstream lm("../languages/transliteration/languages", ifstream::in);
    if (lm.good()) {
	string language;
	string language_name;
	string language_code;

	while (getline(lm, language)) {
	    stringstream stream(language);
	    stream >> language_name >> language_code;
	    SpellingTransliterationImpl* internal = load_transliteration(language_name,
	                                                                language_code);
	    if (internal != NULL) internals.push_back(internal);
	}
    }
    lm.close();
}

SpellingTransliteration::SpellingTransliteration(const string& name) : internal(NULL)
{
    for (unsigned i = 0; i < static_instance.internals.size() && internal.get() == NULL; ++i) {

	if (static_instance.internals[i]->get_lang_name() == name ||
	    static_instance.internals[i]->get_lang_code() == name)
	    internal = static_instance.internals[i];
    }

    if (internal.get() == NULL)
	internal = static_instance.default_internal;
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
