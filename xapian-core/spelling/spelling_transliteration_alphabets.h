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
	add_mapping("\u0430", "a"); // А
	add_mapping("\u0431", "b"); // Б
	add_mapping("\u0432", "v"); // В
	add_mapping("\u0432", "w"); // В (alt)
	add_mapping("\u0433", "g"); // Г
	add_mapping("\u0434", "d"); // Д
	add_mapping("\u0435", "e"); // Е
	add_mapping("\u0451", "e"); // Ё
	add_mapping("\u0451", "yo"); // Ё (alt)
	add_mapping("\u0451", "jo"); // Ё (alt2)
	add_mapping("\u0436", "zh"); // Ж
	add_mapping("\u0437", "z"); // З
	add_mapping("\u0438", "i"); // И
	add_mapping("\u0439", "i"); // Й
	add_mapping("\u0439", "y"); // Й (alt)
	add_mapping("\u0439", "j"); // Й (alt2)
	add_mapping("\u043A", "k"); // К
	add_mapping("\u043B", "l"); // Л
	add_mapping("\u043C", "m"); // М
	add_mapping("\u043D", "n"); // Н
	add_mapping("\u043E", "o"); // О
	add_mapping("\u043F", "p"); // П
	add_mapping("\u0440", "r"); // Р
	add_mapping("\u0441", "s"); // С
	add_mapping("\u0442", "t"); // Т
	add_mapping("\u0443", "u"); // У
	add_mapping("\u0444", "f"); // Ф
	add_mapping("\u0444", "ph"); // Ф (alt)
	add_mapping("\u0445", "kh"); // Х
	add_mapping("\u0446", "ts"); // Ц
	add_mapping("\u0446", "c"); // Ц (alt)
	add_mapping("\u0447", "ch"); // Ч
	add_mapping("\u0448", "sh"); // Ш
	add_mapping("\u0449", "sch"); // Щ
	add_mapping("\u0449", "shch"); // Щ (alt)
	add_mapping("\u044A", ""); // Ъ
	add_mapping("\u044A", "'"); // Ъ (alt)
	add_mapping("\u044B", "y"); // Ы
	add_mapping("\u044B", "i"); // Ы (alt)
	add_mapping("\u044C", ""); // Ь
	add_mapping("\u044C", "'"); // Ь (alt)
	add_mapping("\u044D", "e"); // Э
	add_mapping("\u044E", "iu"); // Ю
	add_mapping("\u044E", "yu"); // Ю (alt)
	add_mapping("\u044E", "ju"); // Ю (alt2)
	add_mapping("\u044F", "ia"); // Я
	add_mapping("\u044F", "ya"); // Я (alt)
	add_mapping("\u044F", "ja"); // Я (alt2)

	make_reverse_mapping();

	add_reverse_mapping("q", "\u043A"); // К (alt)
	add_reverse_mapping("c", "\u043A"); // К (alt2)
	add_reverse_mapping("ck", "\u043A"); // К (alt2)
	add_reverse_mapping("e", "\u0438"); // И
	add_reverse_mapping("x", "\u043A\u0441"); // КС
    }
};

}
#endif // XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_ALPHABETS_H
