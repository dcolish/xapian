/** @file spelling_base.h
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

#ifndef XAPIAN_INCLUDED_SPELLING_BASE_H
#define XAPIAN_INCLUDED_SPELLING_BASE_H

#include <vector>
#include <string>
#include "database.h"

/**
 * Base class for a word sequence spelling correction.
 */
class SpellingBase {

protected:
    //Method to obtain a frequency of the given word.
    unsigned request_internal(const std::string& word);

    //Method to obtain a frequency of the given word pair.
    unsigned request_internal(const std::string& first_word, const std::string& second_word);

    //Keeps references to internal databases.
    const std::vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> > & internal;

public:
    SpellingBase(const std::vector<Xapian::Internal::RefCntPtr<Xapian::Database::Internal> >& internal_);
    virtual ~SpellingBase();

    //Find spelling correction for a sequence of words.
    virtual unsigned get_spelling(const std::string& word, std::string& result) = 0;

    //Find spelling correction for a sequence of words.
    virtual unsigned get_spelling(const std::vector<std::string>& words, std::vector<
	    std::string>& result) = 0;
};

#endif // XAPIAN_INCLUDED_SPELLING_BASE_H
