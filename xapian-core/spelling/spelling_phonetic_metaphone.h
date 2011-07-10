/** @file spelling_phonetic_metaphone.h
 * @brief Spelling phonetic Metaphone algorithm.
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

#ifndef XAPIAN_INCLUDED_SPELLING_PHONETIC_METAPHONE_H
#define XAPIAN_INCLUDED_SPELLING_PHONETIC_METAPHONE_H

#include <vector>
#include <string>
#include "spelling_phonetic.h"

class MetaphoneSpellingPhonetic : public SpellingPhonetic {

    enum Flag {
	VOWEL = 1,
	SAME = 2,
	VAR_SOUND = 4,
	FRONT_VOWEL = 8,
	NOGHF = 16
    };

    static const char alpha[];

    static char at(const std::string& word, int index);
    static bool is(char ch, Flag flag);

public:
    bool get_phonetic(const std::string& input, std::vector<std::string>& result) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_PHONETIC_METAPHONE_H
