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

class XAPIAN_VISIBILITY_DEFAULT LanguageAutodetect {

    static const unsigned MAX_N = 5;
    static const unsigned MAX_N_COUNT = 400;
    static const unsigned MIN_N_FREQ = 1;

    std::map<std::string, std::map<std::string, unsigned> > languages;

    bool is_word_char(unsigned int ch) const;

    bool load_language(const std::string& language, std::map<std::string, unsigned>& result) const;

    unsigned check_language(const std::vector<std::string>& unknown,
                            const std::map<std::string, unsigned>& language_map) const;

public:
    LanguageAutodetect();
    LanguageAutodetect(std::vector<std::string> languages_);

    std::vector<std::string> create_language_model(const std::string& text) const;

    std::string get_language(const std::string& text) const;
};

}

#endif // XAPIAN_INCLUDED_LANGUAGE_AUTODETECT_H
