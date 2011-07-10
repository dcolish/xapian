/** @file phonetic.h
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

#ifndef XAPIAN_INCLUDED_SPELLING_BASE_H
#define XAPIAN_INCLUDED_SPELLING_BASE_H

#include <vector>
#include <string>
#include "database.h"
#include "spelling_phonetic.h"
#include "spelling_transliteration.h"

class Phonetic {

    const SpellingPhonetic* spelling_phonetic;
    const SpellingTransliteration* spelling_transliteration;

public:
    Phonetic(const std::string& language);
    Phonetic(const SpellingTransliteration* spelling_transliteration_ = NULL);
    ~Phonetic();

    std::string get_phonetic(const std::string& input) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_BASE_H
