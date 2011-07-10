/** @file extended_edit_distance.cc
 * @brief Extended edit distance calculation algorithm.
 *
 *  Based on that described in:
 *
 *  "An extension of Ukkonen's enhanced dynamic programming ASM algorithm"
 *  by Hal Berghel, University of Arkansas
 *  and David Roach, Acxiom Corporation
 *
 *  http://berghel.net/publications/asm/asm.php
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

#include "extended_edit_distance.h"

#include <algorithm>
#include <limits>

using namespace std;

double ExtendedEditDistance::edit_distance(const unsigned* first_word, unsigned first_length,
		const unsigned* second_word, unsigned second_length, unsigned max_distance)
{
	const double INF = numeric_limits<double>::max();

	if (first_length == 0)
		return second_length;
	else if (second_length == 0)
		return first_length;

	if (first_length > second_length)
	{
		swap(first_word, second_word);
		swap(first_length, second_length);
	}

	if (second_length - first_length > max_distance)
		return second_length - first_length;

	if (row_capacity < first_length + 1)
	{
		unsigned new_row_capacity = (first_length + 1) * 2;
		delete[] current_row;
		current_row = new double[new_row_capacity];

		delete[] previous_row;
		previous_row = new double[new_row_capacity];

		delete[] transposition_row;
		transposition_row = new double[new_row_capacity];
		row_capacity = new_row_capacity;
	}

	previous_row[0] = 0;
	for (unsigned i = 1; i <= first_length; ++i)
		previous_row[i] = i * get_insert_cost(i - 1, first_length, first_word[i - 1]);

	unsigned last_second_ch = 0;
	for (unsigned i = 1; i <= second_length; ++i)
	{
		fill_n(current_row, first_length + 1, INF);

		const unsigned second_ch = second_word[i - 1];
		current_row[0] = i * get_insert_cost(i - 1, second_length, second_ch);

		unsigned from = (i > max_distance + 1) ? (i - max_distance - 1) : 1;
		unsigned to = min(i + max_distance + 1, first_length);

		unsigned last_first_ch = 0;
		for (unsigned j = from; j <= to; ++j)
		{
			const unsigned first_index = j - 1;
			const unsigned first_ch = first_word[first_index];

			const double insert_value = current_row[j - 1] + get_insert_cost(first_index, first_length, first_ch);
			const double delete_value = previous_row[j] + get_delete_cost(first_index, first_length, second_ch);
			const double replace_value = previous_row[j - 1] + ((first_ch != second_ch) ? get_replace_cost(first_index,
					first_length, first_ch, second_ch) : 0);

			double value = min(min(insert_value, delete_value), replace_value);

			if (first_ch != second_ch && first_ch == last_second_ch && second_ch == last_first_ch)
				value = min(value, transposition_row[j - 2] + get_transposition_cost(first_index, first_length,
						first_ch, second_ch));

			current_row[j] = value;
			last_first_ch = first_ch;
		}
		last_second_ch = second_ch;

		swap(transposition_row, previous_row);
		swap(previous_row, current_row);
	}
	return previous_row[first_length];
}
