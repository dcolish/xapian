/** @file spelling_keyboard.h
 * @brief Spelling keyboard layout.
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

#ifndef XAPIAN_INCLUDED_SPELLING_KEYBOARD_H
#define XAPIAN_INCLUDED_SPELLING_KEYBOARD_H

#include <vector>
#include <string>

#include <xapian/unordered_map.h>
#include <xapian/unordered_set.h>

/**
 * Base class for a word keyboard layout convertion.
 */
class SpellingKeyboard {

    class DefaultKeyboard {

	std::unordered_set<unsigned> default_set;
	std::unordered_map<unsigned, std::pair<double, double> > distance_map;
	double max_distance;

    public:
	DefaultKeyboard();

	bool is_default(unsigned ch) const;

	double get_key_proximity(unsigned first_ch, unsigned second_ch) const;
    };

    static DefaultKeyboard default_keyboard;

    std::unordered_map<unsigned, unsigned> to_char_map;
    std::unordered_map<unsigned, unsigned> from_char_map;

    std::string language_name;
    std::string language_code;


    bool convert_layout(const std::string& word,
                        const std::unordered_map<unsigned, unsigned>& char_map,
			std::string& result) const;

protected:
    void add_char_mapping(unsigned lang_char, unsigned default_char);

public:
    SpellingKeyboard(const std::string& language_name_, const std::string& language_code_);

    bool convert_to_layout(const std::string& word, std::string& result) const;

    bool convert_from_layout(const std::string& word, std::string& result) const;

    double get_key_proximity(unsigned first_ch, unsigned second_ch) const;

    const std::string& get_lang_name() const;

    const std::string& get_lang_code() const;
};

#endif // XAPIAN_INCLUDED_SPELLING_KEYBOARD_H
