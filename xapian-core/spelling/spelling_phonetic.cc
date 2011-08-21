/** @file phonetic.cc
 * @brief Phonetic class.
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

#include "spelling_phonetic.h"
#include "spelling_phonetic_metaphone.h"
#include "spelling_transliteration.h"

using namespace std;
using namespace Xapian;

SpellingPhonetic::SpellingPhonetic(const string& language) :
	internal(new MetaphoneSpellingPhonetic),
	translit(language)
{
}

SpellingPhonetic::SpellingPhonetic(SpellingPhoneticImpl* internal_,
                                   SpellingTransliteration translit_) :
	internal(internal_), translit(translit_)
{
}

SpellingPhonetic::~SpellingPhonetic()
{
}

string SpellingPhonetic::get_phonetic(const string& input) const
{
    if (internal.get() == NULL) return string();

    string word = translit.get_transliteration(input);
    if (word.empty()) word = input;

    vector<string> result_vector;
    if (!internal->get_phonetic(word, result_vector))
	return string();

    return result_vector.front();
}
