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

#include "phonetic.h"
#include "spelling_phonetic_metaphone.h"
#include "spelling_transliteration_alphabets.h"

using namespace std;

Phonetic::Phonetic(const string& language) :
	spelling_phonetic(new MetaphoneSpellingPhonetic),
	spelling_transliteration(SpellingTransliterationFactory::get_transliteration(language))
{
}

Phonetic::Phonetic(const SpellingTransliteration* spelling_transliteration_) :
	spelling_phonetic(new MetaphoneSpellingPhonetic),
	spelling_transliteration(spelling_transliteration_)
{
}

Phonetic::~Phonetic()
{
    delete spelling_phonetic;
}

string Phonetic::get_phonetic(const string& input) const
{
    string word;
    if (spelling_transliteration != NULL)
    {
	if (!spelling_transliteration->get_transliteration(input, word))
	    return string();
    }
    else word = input;

    vector<string> result_vector;
    if (!spelling_phonetic->get_phonetic(word, result_vector))
	return string();

    return result_vector.front();
}
