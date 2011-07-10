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

using namespace std;
using namespace Xapian;

SpellingTransliteration::SpellingTransliteration(const std::string& language_name_,
                                                 const std::string& language_code_) :
                                                 language_name(language_name_),
                                                 language_code(language_code_)
{
}

bool
SpellingTransliteration::is_default(unsigned ch) const
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

void
SpellingTransliteration::add_char_mapping(unsigned lang_char, const char* sequence)
{
    char_map.insert(make_pair(Unicode::tolower(lang_char), sequence));
}

bool
SpellingTransliteration::get_transliteration(const std::string& word, std::string& result) const
{
    result.clear();
    for (Utf8Iterator it(word); it != Utf8Iterator(); ++it) {
	unordered_map<unsigned, const char*>::const_iterator char_it = char_map.find(Unicode::tolower(*it));
	if (char_it == char_map.end()) {
	    if (!is_default(*it)) return false;

	    result.push_back(*it);
	} else result.append(char_it->second);
    }
    return true;
}

const std::string&
SpellingTransliteration::get_language_name() const
{
    return language_name;
}

const std::string&
SpellingTransliteration::get_language_code() const
{
    return language_code;
}
