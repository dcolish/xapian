/** @file spelling_base.cc
 * @brief Spelling base.
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
#include <algorithm>

#include "database.h"
#include "spelling_base.h"

using namespace std;
using namespace Xapian;

SpellingBase::SpellingBase(const std::vector<Xapian::Internal::RefCntPtr<Database::Internal> >& internal_, const std::string& prefix_) :
    internal(internal_), prefix(prefix_)
{
}

SpellingBase::~SpellingBase()
{
}

unsigned
SpellingBase::request_internal(const string& word)
{
    unsigned freq = 0;

    for (size_t i = 0; i < internal.size(); ++i)
	freq += internal[i]->get_spelling_frequency(word, prefix);

    return freq;
}

unsigned
SpellingBase::request_internal(const string& first_word, const string& second_word)
{
    unsigned freq = 0;

    for (size_t i = 0; i < internal.size(); ++i)
	freq += internal[i]->get_spellings_frequency(first_word, second_word, prefix);

    if (freq > 0) return freq * 2;

    freq = request_internal(first_word) + request_internal(second_word);

    if (freq > 0) return max(freq / 32, 1u);
    return 0;
}
