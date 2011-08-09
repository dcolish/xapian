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
#include <map>
#include <string>
#include "database.h"

/**
 * Base class for a word sequence spelling correction.
 */
class SpellingBase {

protected:
    double normalize_freq(double freq) const;

    //Method to obtain a frequency of the given word.
    unsigned request_internal_freq(const std::string& word) const;

    //Method to obtain a frequency of the given word pair.
    unsigned request_internal_freq(const std::string& first_word, const std::string& second_word) const;

    //Method to obtain a normalized frequency of the given word.
    double request_internal(const std::string& word) const;

    //Method to obtain a normalized frequency of the given word pair.
    double request_internal(const std::string& first_word, const std::string& second_word) const;

    //Keeps references to internal databases.
    const std::vector<Xapian::Internal::intrusive_ptr<Xapian::Database::Internal> > & internal;
    std::string prefix;

public:
    SpellingBase(const std::vector<Xapian::Internal::intrusive_ptr<Xapian::Database::Internal> >& internal_, const std::string& prefix_);
    virtual ~SpellingBase();

    /** Find spelling correction for a given word.
     *
     *  @param word	The misspelled word.
     *  @param result	The string to receive the result.
     *
     *  @return		Returns relative frequency of the result.
     */
    virtual double get_spelling(const std::string& word, std::string& result) const = 0;

    /** Find spelling correction for a sequence of words.
     *
     *  @param words	The sequence of misspelled words.
     *  @param result	The vector to receive the result.
     *
     *  @return		Returns relative frequency of the result.
     */
    virtual double get_spelling(const std::vector<std::string>& words, std::vector<std::string>& result) const = 0;

    /** Find multiple spelling corrections for a given word.
     *
     *	@param word		The misspelled word.
     *	@param result_count	The result count.
     *	@param result		The map to receive the results (value) associated with relative frequency (key).
     */
    virtual void get_multiple_spelling(const std::string& word, unsigned result_count,
                                       std::multimap<double, std::string, std::greater<double> >& result) const;

    /** Find multiple spelling corrections for a sequence of words.
     *
     *	@param word		The sequency of misspelled words.
     *	@param result_count	The result count.
     *	@param result		The map to receive the results (value) associated with relative frequency (key).
     */
    virtual void get_multiple_spelling(const std::vector<std::string>& words, unsigned result_count,
                                       std::multimap<double, std::vector<std::string>, std::greater<double> >& result) const;
};

#endif // XAPIAN_INCLUDED_SPELLING_BASE_H
