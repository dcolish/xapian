/** @file spelling_transliteration.h
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

#ifndef XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_H
#define XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_H

#include <vector>
#include <string>
#include <xapian/unordered_map.h>

/**
 * Base class for word transliteration methods
 */
class SpellingTransliteration {

    std::unordered_map<unsigned, const char*> char_map;

    std::string language_name;
    std::string language_code;

protected:
    void add_char_mapping(unsigned lang_char, const char* sequence);

public:
    SpellingTransliteration(const std::string& language_name_, const std::string& language_code_);

    bool get_transliteration(const std::string& word, std::string& result) const;

    const std::string& get_language_name() const;

    const std::string& get_language_code() const;
};

#endif // XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_H
