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

#ifndef XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_ALPHABETS_H
#define XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_ALPHABETS_H

#include <vector>
#include <string>
#include <xapian/unicode.h>

#include "spelling_transliteration.h"

class EnglishSpellingTransliteration : public SpellingTransliterationImpl {

public:
    EnglishSpellingTransliteration() : SpellingTransliterationImpl("english", "en")
    {
    }
};

class RussianSpellingTransliteration : public SpellingTransliterationImpl {

public:
    RussianSpellingTransliteration() : SpellingTransliterationImpl("russian", "ru")
    {
	add_char_mapping(0x0430, "a");
	add_char_mapping(0x0431, "b");
	add_char_mapping(0x0432, "v");
	add_char_mapping(0x0433, "g");
	add_char_mapping(0x0434, "d");
	add_char_mapping(0x0435, "e");
	add_char_mapping(0x0436, "zh");
	add_char_mapping(0x0437, "z");
	add_char_mapping(0x0438, "i");
	add_char_mapping(0x0439, "i");
	add_char_mapping(0x043A, "k");
	add_char_mapping(0x043B, "l");
	add_char_mapping(0x043C, "m");
	add_char_mapping(0x043D, "n");
	add_char_mapping(0x043E, "o");
	add_char_mapping(0x043F, "p");
	add_char_mapping(0x0440, "r");
	add_char_mapping(0x0441, "s");
	add_char_mapping(0x0442, "t");
	add_char_mapping(0x0443, "u");
	add_char_mapping(0x0444, "f");
	add_char_mapping(0x0445, "kh");
	add_char_mapping(0x0446, "ts");
	add_char_mapping(0x0447, "ch");
	add_char_mapping(0x0448, "sh");
	add_char_mapping(0x0449, "sch");
	add_char_mapping(0x044A, "");
	add_char_mapping(0x044B, "y");
	add_char_mapping(0x044C, "");
	add_char_mapping(0x044D, "e");
	add_char_mapping(0x044E, "iu");
	add_char_mapping(0x044F, "ia");
	add_char_mapping(0x0451, "e");
    }
};

#endif // XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_ALPHABETS_H
