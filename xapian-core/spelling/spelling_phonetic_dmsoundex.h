/** @file spelling_phonetic_dmsoundex.h
 * @brief Spelling phonetic Daitch-Mokotoff Soundex algorithm.
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

#ifndef XAPIAN_INCLUDED_SPELLING_PHONETIC_DMSOUNDEX_H
#define XAPIAN_INCLUDED_SPELLING_PHONETIC_DMSOUNDEX_H

#include <vector>
#include <string>
#include <xapian/unordered_map.h>
#include "spelling_phonetic.h"

class DMSoundexSpellingPhonetic : public SpellingPhonetic {

    struct Entry {
	bool vowel;

	const char* first;
	const char* before;
	const char* other;
	const char* alternate;
    };

    std::unordered_map<std::string, Entry> entry_map;
    unsigned max_entry_length;

    void add_entry(const char* str, bool vowel, const char* first, const char* before, const char* other,
		   const char* alternate);

    unsigned find_entry(const std::string& word, unsigned offset, Entry& entry, std::string& buffer) const;

    const char* get_entry_value(const std::vector<Entry>& entries, unsigned index, const Entry& entry) const;

public:
    DMSoundexSpellingPhonetic();

    bool get_phonetic(const std::string& input, std::vector<std::string>& result) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_PHONETIC_DMSOUNDEX_H
