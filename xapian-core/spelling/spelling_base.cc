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
#include <cmath>

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

double
SpellingBase::normalize_freq(double freq) const
{
    return log(1.0 + freq) / log(2.0);
}

unsigned
SpellingBase::request_internal_freq(const std::string& word) const
{
    unsigned freq = 0;

    for (size_t i = 0; i < internal.size(); ++i)
	freq += internal[i]->get_spelling_frequency(word, prefix);

    return freq;
}

unsigned
SpellingBase::request_internal_freq(const std::string& first_word, const std::string& second_word) const
{
    unsigned freq = 0;

    for (size_t i = 0; i < internal.size(); ++i)
	freq += internal[i]->get_spellings_frequency(first_word, second_word, prefix);

    return freq;
}

double
SpellingBase::request_internal(const string& word) const
{
    return normalize_freq(request_internal_freq(word));
}

double
SpellingBase::request_internal(const string& first_word, const string& second_word) const
{
    if (first_word.empty()) return request_internal(second_word);
    if (second_word.empty()) return request_internal(first_word);

    unsigned pair_freq = request_internal_freq(first_word, second_word);
    unsigned single_freq = request_internal_freq(first_word) + request_internal_freq(second_word);

    return (1 + pair_freq) * normalize_freq(single_freq);
}
