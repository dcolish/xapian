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

namespace Xapian {

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
	add_char_mapping(0x0430, "a"); // А
	add_char_mapping(0x0431, "b"); // Б
	add_char_mapping(0x0432, "v"); // В
	add_char_mapping(0x0432, "w"); // В (alt)
	add_char_mapping(0x0433, "g"); // Г
	add_char_mapping(0x0434, "d"); // Д
	add_char_mapping(0x0435, "e"); // Е
	add_char_mapping(0x0451, "e"); // Ё
	add_char_mapping(0x0451, "yo"); // Ё (alt)
	add_char_mapping(0x0451, "jo"); // Ё (alt2)
	add_char_mapping(0x0436, "zh"); // Ж
	add_char_mapping(0x0437, "z"); // З
	add_char_mapping(0x0438, "i"); // И
	add_char_mapping(0x0439, "i"); // Й
	add_char_mapping(0x0439, "y"); // Й (alt)
	add_char_mapping(0x0439, "j"); // Й (alt2)
	add_char_mapping(0x043A, "k"); // К
	add_char_mapping(0x043B, "l"); // Л
	add_char_mapping(0x043C, "m"); // М
	add_char_mapping(0x043D, "n"); // Н
	add_char_mapping(0x043E, "o"); // О
	add_char_mapping(0x043F, "p"); // П
	add_char_mapping(0x0440, "r"); // Р
	add_char_mapping(0x0441, "s"); // С
	add_char_mapping(0x0442, "t"); // Т
	add_char_mapping(0x0443, "u"); // У
	add_char_mapping(0x0444, "f"); // Ф
	add_char_mapping(0x0444, "ph"); // Ф (alt)
	add_char_mapping(0x0445, "kh"); // Х
	add_char_mapping(0x0446, "ts"); // Ц
	add_char_mapping(0x0447, "ch"); // Ч
	add_char_mapping(0x0448, "sh"); // Ш
	add_char_mapping(0x0449, "sch"); // Щ
	add_char_mapping(0x0449, "shch"); // Щ (alt)
	add_char_mapping(0x044A, ""); // Ъ
	add_char_mapping(0x044B, "y"); // Ы
	add_char_mapping(0x044B, "i"); // Ы (alt)
	add_char_mapping(0x044C, ""); // Ь
	add_char_mapping(0x044D, "e"); // Э
	add_char_mapping(0x044E, "iu"); // Ю
	add_char_mapping(0x044E, "yu"); // Ю (alt)
	add_char_mapping(0x044E, "ju"); // Ю (alt2)
	add_char_mapping(0x044F, "ia"); // Я
	add_char_mapping(0x044F, "ya"); // Я (alt)
	add_char_mapping(0x044F, "ja"); // Я (alt2)
    }
};

}
#endif // XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_ALPHABETS_H
