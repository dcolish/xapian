/** @file spelling_phonetic.h
 * @brief Spelling phonetic base.
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

#ifndef XAPIAN_INCLUDED_SPELLING_PHONETIC_H
#define XAPIAN_INCLUDED_SPELLING_PHONETIC_H

#include <vector>
#include <string>

/**
 * Base class for a word phonetic key generation.
 */
class SpellingPhonetic {

public:
    virtual ~SpellingPhonetic() { }

    //Generate phonetic codes for a given word.
    virtual bool get_phonetic(const std::string& input, std::vector<std::string>& result) const = 0;
};

#endif // XAPIAN_INCLUDED_SPELLING_PHONETIC_H
