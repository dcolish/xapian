/** @file extended_edit_distance.h
 * @brief Extended edit distance calculation algorithm.
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

#ifndef XAPIAN_INCLUDED_EXTENDED_EDIT_DISTANCE_H
#define XAPIAN_INCLUDED_EXTENDED_EDIT_DISTANCE_H

/** Calculate the edit distance between two sequences.
 *
 *  Edit distance is defined as the minimum number of edit operations
 *  required to move from one sequence to another.  The edit operations
 *  considered are:
 *   - Insertion of a character at an arbitrary position.
 *   - Deletion of a character at an arbitrary position.
 *   - Substitution of a character at an arbitrary position.
 *   - Transposition of two neighbouring characters at an arbitrary position
 *     in the string.
 *
 *  @param ptr1 A pointer to the start of the first sequence.
 *  @param len1 The length of the first sequence.
 *  @param ptr2 A pointer to the start of the second sequence.
 *  @param len2 The length of the first sequence.
 *  @param max_distance The greatest edit distance that's interesting to us.
 *			If the true edit distance is > max_distance, any
 *			value > max_distance may be returned instead (which
 *			allows the edit distance algorithm to avoid work for
 *			poor matces).
 *
 *  @return The edit distance from one item to the other.
 */

#include <string>
#include "spelling_keyboard.h"

class ExtendedEditDistance
{
    double* current_row;
    double* previous_row;
    double* transposition_row;
    unsigned row_capacity;

    SpellingKeyboard keyboard_layout;

    //Cost of position of a letter in a word
    inline double get_index_cost(unsigned index, unsigned length)
    {
	return std::max(1.0 - double(std::min(index, length - 1)) / double(std::max(length - 1, 1u)), 0.0);
    }

    //Cost of proximity of keys on a keyboard
    inline double get_keyboard_cost(unsigned first_ch, unsigned second_ch)
    {
	double proximity = keyboard_layout.get_key_proximity(first_ch, second_ch);
	return proximity > 0.9 ? proximity : 0.0;
    }

    //Insertion cost
    inline double get_insert_cost(unsigned index, unsigned length, unsigned /*ch*/)
    {
	return 0.85 + 0.4 * get_index_cost(index, length);
    }

    //Deletion cost
    inline double get_delete_cost(unsigned index, unsigned length, unsigned /*ch*/)
    {
	return 0.75 + 0.4 * get_index_cost(index, length);
    }

    //Substitution cost
    inline double get_replace_cost(unsigned index, unsigned length, unsigned first_ch, unsigned second_ch)
    {
	return 0.75 + 0.35 * get_index_cost(index, length) - get_keyboard_cost(first_ch, second_ch) * 0.25;
    }

    //Transposition cost
    inline double get_transposition_cost(unsigned index, unsigned length, unsigned /*first_ch*/, unsigned /*second_ch*/)
    {
	return 0.75 + 0.25 * get_index_cost(index, length);
    }

public:
    ExtendedEditDistance(const SpellingKeyboard& keyboard_layout_ = SpellingKeyboard()) :
	current_row(NULL), previous_row(NULL), transposition_row(NULL), row_capacity(0),
	keyboard_layout(keyboard_layout_)
    {
    }

    ~ExtendedEditDistance()
    {
	delete[] current_row;
	delete[] previous_row;
	delete[] transposition_row;
    }

    double edit_distance(const unsigned* first_word, unsigned first_length, const unsigned* second_word,
                         unsigned second_length, unsigned max_distance);
};

#endif // XAPIAN_INCLUDED_EXTENDED_EDIT_DISTANCE_H
