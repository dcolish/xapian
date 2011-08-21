/** @file language_autodetect.h
 * @brief Language autodetection tool.
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

#ifndef XAPIAN_INCLUDED_LANGUAGE_AUTODETECT_H
#define XAPIAN_INCLUDED_LANGUAGE_AUTODETECT_H

#include <string>
#include <vector>
#include <map>
#include <xapian/visibility.h>

namespace Xapian {

//Class for automatic language detection using n-gram based TextCat method and unicode ranges
class XAPIAN_VISIBILITY_DEFAULT LanguageAutodetect {

    //Maximum n-gram length.
    static const unsigned MAX_N = 5;
    //Maximum n-grams count to check and include in language model.
    static const unsigned MAX_N_COUNT = 400;
    //Minimum n-gram frequency to include it in language model.
    static const unsigned MIN_N_FREQ = 1;

    //Map of languages, n-grams and its scores (less score - better result).
    std::map<std::string, std::map<std::string, unsigned> > languages;
    //Map of language ranges.
    std::map<std::string, std::vector<std::pair<unsigned, unsigned> > > language_ranges;

    //Return if character may be word character - not 0-9, \t, \n
    bool is_word_char(unsigned int ch) const;

    //Load language model (n-grams) from file with language.lm name into result map.
    bool load_language(const std::string& language, std::map<std::string, unsigned>& result) const;

    //Check language and return its score related to the given unknown language model
    //(which is generated from query string).
    unsigned check_language(const std::vector<std::string>& unknown,
                            const std::string& language) const;

    //Create language model (i.e. n-grams list) for the given text.
    std::vector<std::string> create_language_model(const std::string& text) const;

public:
    //Construct LanguageAutodetect object with all available languages, which described
    //in "languages" file.
    LanguageAutodetect();

    //Construct LanguageAutodetect object with the given languages.
    LanguageAutodetect(std::vector<std::string> languages_);

    //Get most possible language for the given text
    std::string get_language(const std::string& text) const;
};

}

#endif // XAPIAN_INCLUDED_LANGUAGE_AUTODETECT_H
