/** @file spelling_keyboard.cc
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

#include <config.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <xapian/unicode.h>

#include "spelling_keyboard.h"

using namespace std;
using namespace Xapian;

SpellingKeyboard::DefaultKeyboard SpellingKeyboard::default_keyboard;

SpellingKeyboard::DefaultKeyboard::DefaultKeyboard() : max_distance(0.0)
{
    default_set.insert(0x0060);
    default_set.insert('1');
    default_set.insert('2');
    default_set.insert('3');
    default_set.insert('4');
    default_set.insert('5');
    default_set.insert('6');
    default_set.insert('7');
    default_set.insert('8');
    default_set.insert('9');
    default_set.insert('0');
    default_set.insert(0x002d);
    default_set.insert(0x003d);
    default_set.insert('q');
    default_set.insert('w');
    default_set.insert('e');
    default_set.insert('r');
    default_set.insert('t');
    default_set.insert('y');
    default_set.insert('u');
    default_set.insert('i');
    default_set.insert('o');
    default_set.insert('p');
    default_set.insert(0x005b);
    default_set.insert(0x005d);
    default_set.insert(0x005c);
    default_set.insert('a');
    default_set.insert('s');
    default_set.insert('d');
    default_set.insert('f');
    default_set.insert('g');
    default_set.insert('h');
    default_set.insert('j');
    default_set.insert('k');
    default_set.insert('l');
    default_set.insert(0x003b);
    default_set.insert(0x0027);
    default_set.insert('z');
    default_set.insert('x');
    default_set.insert('c');
    default_set.insert('v');
    default_set.insert('b');
    default_set.insert('n');
    default_set.insert('m');
    default_set.insert(0x002c);
    default_set.insert(0x002e);
    default_set.insert(0x002f);
    default_set.insert(0x007c);

    default_set.insert(0x007e);
    default_set.insert(0x0021);
    default_set.insert(0x0040);
    default_set.insert(0x0023);
    default_set.insert(0x0024);
    default_set.insert(0x0025);
    default_set.insert(0x005e);
    default_set.insert(0x0026);
    default_set.insert(0x002a);
    default_set.insert(0x0028);
    default_set.insert(0x0029);
    default_set.insert(0x005f);
    default_set.insert(0x002b);
    default_set.insert('Q');
    default_set.insert('W');
    default_set.insert('E');
    default_set.insert('R');
    default_set.insert('T');
    default_set.insert('Y');
    default_set.insert('U');
    default_set.insert('I');
    default_set.insert('O');
    default_set.insert('P');
    default_set.insert(0x007b);
    default_set.insert(0x007d);
    default_set.insert(0x007c);
    default_set.insert('A');
    default_set.insert('S');
    default_set.insert('D');
    default_set.insert('F');
    default_set.insert('G');
    default_set.insert('H');
    default_set.insert('J');
    default_set.insert('K');
    default_set.insert('L');
    default_set.insert(0x003a);
    default_set.insert(0x0022);
    default_set.insert('Z');
    default_set.insert('X');
    default_set.insert('C');
    default_set.insert('V');
    default_set.insert('B');
    default_set.insert('N');
    default_set.insert('M');
    default_set.insert(0x003c);
    default_set.insert(0x003e);
    default_set.insert(0x003f);
    default_set.insert(0x007c);

    distance_map.insert(make_pair(0x0060, make_pair(0.0, 0.0)));
    distance_map.insert(make_pair('1', make_pair(0.0, 1.0)));
    distance_map.insert(make_pair('2', make_pair(0.0, 2.0)));
    distance_map.insert(make_pair('3', make_pair(0.0, 3.0)));
    distance_map.insert(make_pair('4', make_pair(0.0, 4.0)));
    distance_map.insert(make_pair('5', make_pair(0.0, 5.0)));
    distance_map.insert(make_pair('6', make_pair(0.0, 6.0)));
    distance_map.insert(make_pair('7', make_pair(0.0, 7.0)));
    distance_map.insert(make_pair('8', make_pair(0.0, 8.0)));
    distance_map.insert(make_pair('9', make_pair(0.0, 9.0)));
    distance_map.insert(make_pair('0', make_pair(0.0, 10.0)));
    distance_map.insert(make_pair(0x002d, make_pair(0.0, 11.0)));
    distance_map.insert(make_pair(0x003d, make_pair(0.0, 12.0)));
    distance_map.insert(make_pair('q', make_pair(1.0, 1.4)));
    distance_map.insert(make_pair('w', make_pair(1.0, 2.4)));
    distance_map.insert(make_pair('e', make_pair(1.0, 3.4)));
    distance_map.insert(make_pair('r', make_pair(1.0, 4.4)));
    distance_map.insert(make_pair('t', make_pair(1.0, 5.4)));
    distance_map.insert(make_pair('y', make_pair(1.0, 6.4)));
    distance_map.insert(make_pair('u', make_pair(1.0, 7.4)));
    distance_map.insert(make_pair('i', make_pair(1.0, 8.4)));
    distance_map.insert(make_pair('o', make_pair(1.0, 9.4)));
    distance_map.insert(make_pair('p', make_pair(1.0, 10.4)));
    distance_map.insert(make_pair(0x005b, make_pair(1.0, 11.4)));
    distance_map.insert(make_pair(0x005d, make_pair(1.0, 12.4)));
    distance_map.insert(make_pair(0x005c, make_pair(1.0, 13.4)));
    distance_map.insert(make_pair('a', make_pair(2.0, 1.7)));
    distance_map.insert(make_pair('s', make_pair(2.0, 2.7)));
    distance_map.insert(make_pair('d', make_pair(2.0, 3.7)));
    distance_map.insert(make_pair('f', make_pair(2.0, 4.7)));
    distance_map.insert(make_pair('g', make_pair(2.0, 5.7)));
    distance_map.insert(make_pair('h', make_pair(2.0, 6.7)));
    distance_map.insert(make_pair('j', make_pair(2.0, 7.7)));
    distance_map.insert(make_pair('k', make_pair(2.0, 8.7)));
    distance_map.insert(make_pair('l', make_pair(2.0, 9.7)));
    distance_map.insert(make_pair(0x003b, make_pair(2.0, 10.7)));
    distance_map.insert(make_pair(0x0027, make_pair(2.0, 11.7)));
    distance_map.insert(make_pair('z', make_pair(3.0, 2.0)));
    distance_map.insert(make_pair('x', make_pair(3.0, 3.0)));
    distance_map.insert(make_pair('c', make_pair(3.0, 4.0)));
    distance_map.insert(make_pair('v', make_pair(3.0, 5.0)));
    distance_map.insert(make_pair('b', make_pair(3.0, 6.0)));
    distance_map.insert(make_pair('n', make_pair(3.0, 7.0)));
    distance_map.insert(make_pair('m', make_pair(3.0, 8.0)));
    distance_map.insert(make_pair(0x002c, make_pair(3.0, 9.0)));
    distance_map.insert(make_pair(0x002e, make_pair(3.0, 10.0)));
    distance_map.insert(make_pair(0x002f, make_pair(3.0, 11.0)));
    distance_map.insert(make_pair(0x007c, make_pair(3.0, 12.0)));

    distance_map.insert(make_pair(0x007e, distance_map[0x0060]));
    distance_map.insert(make_pair(0x0021, distance_map['1']));
    distance_map.insert(make_pair(0x0040, distance_map['2']));
    distance_map.insert(make_pair(0x0023, distance_map['3']));
    distance_map.insert(make_pair(0x0024, distance_map['4']));
    distance_map.insert(make_pair(0x0025, distance_map['5']));
    distance_map.insert(make_pair(0x005e, distance_map['6']));
    distance_map.insert(make_pair(0x0026, distance_map['7']));
    distance_map.insert(make_pair(0x002a, distance_map['8']));
    distance_map.insert(make_pair(0x0028, distance_map['9']));
    distance_map.insert(make_pair(0x0029, distance_map['0']));
    distance_map.insert(make_pair(0x005f, distance_map[0x002d]));
    distance_map.insert(make_pair(0x002b, distance_map[0x003d]));
    distance_map.insert(make_pair('Q', distance_map['q']));
    distance_map.insert(make_pair('W', distance_map['w']));
    distance_map.insert(make_pair('E', distance_map['e']));
    distance_map.insert(make_pair('R', distance_map['r']));
    distance_map.insert(make_pair('T', distance_map['t']));
    distance_map.insert(make_pair('Y', distance_map['y']));
    distance_map.insert(make_pair('U', distance_map['u']));
    distance_map.insert(make_pair('I', distance_map['i']));
    distance_map.insert(make_pair('O', distance_map['o']));
    distance_map.insert(make_pair('P', distance_map['p']));
    distance_map.insert(make_pair(0x007b, distance_map[0x005b]));
    distance_map.insert(make_pair(0x007d, distance_map[0x005d]));
    distance_map.insert(make_pair(0x007c, distance_map[0x005c]));
    distance_map.insert(make_pair('A', distance_map['a']));
    distance_map.insert(make_pair('S', distance_map['s']));
    distance_map.insert(make_pair('D', distance_map['d']));
    distance_map.insert(make_pair('F', distance_map['f']));
    distance_map.insert(make_pair('G', distance_map['g']));
    distance_map.insert(make_pair('H', distance_map['h']));
    distance_map.insert(make_pair('J', distance_map['j']));
    distance_map.insert(make_pair('K', distance_map['k']));
    distance_map.insert(make_pair('L', distance_map['l']));
    distance_map.insert(make_pair(0x003a, distance_map[0x003b]));
    distance_map.insert(make_pair(0x0022, distance_map[0x0027]));
    distance_map.insert(make_pair('Z', distance_map['z']));
    distance_map.insert(make_pair('X', distance_map['x']));
    distance_map.insert(make_pair('C', distance_map['c']));
    distance_map.insert(make_pair('V', distance_map['v']));
    distance_map.insert(make_pair('B', distance_map['b']));
    distance_map.insert(make_pair('N', distance_map['n']));
    distance_map.insert(make_pair('M', distance_map['m']));
    distance_map.insert(make_pair(0x003c, distance_map[0x002c]));
    distance_map.insert(make_pair(0x003e, distance_map[0x002e]));
    distance_map.insert(make_pair(0x003f, distance_map[0x002f]));
    distance_map.insert(make_pair(0x007c, distance_map[0x007c]));

    unordered_map<unsigned, pair<double, double> >::const_iterator it;
    for (it = distance_map.begin(); it != distance_map.end(); ++it) {

	double distance = sqrt(it->second.first * it->second.first +
	                       it->second.second * it->second.second);

	if (distance > max_distance) max_distance = distance;
    }
}

bool
SpellingKeyboard::DefaultKeyboard::is_default(unsigned ch) const
{
    return default_set.find(ch) != default_set.end();
}

double
SpellingKeyboard::DefaultKeyboard::get_key_proximity(unsigned first_ch, unsigned second_ch) const
{
    unordered_map<unsigned, pair<double, double> >::const_iterator first_it = distance_map.find(first_ch);
    unordered_map<unsigned, pair<double, double> >::const_iterator second_it = distance_map.find(second_ch);

    if (first_it == distance_map.end() || second_it == distance_map.end()) return 0.0;

    double dx = first_it->second.first - second_it->second.first;
    double dy = first_it->second.second - second_it->second.second;

    return 1 - sqrt(dx * dx + dy * dy) / max_distance;
}

SpellingKeyboard::SpellingKeyboard(const std::string& language_name_,
                                   const std::string& language_code_) :
                                   language_name(language_name_),
                                   language_code(language_code_)
{
}

void
SpellingKeyboard::add_char_mapping(unsigned lang_char, unsigned default_char)
{
    from_char_map.insert(make_pair(lang_char, default_char));
    to_char_map.insert(make_pair(default_char, lang_char));
}

bool
SpellingKeyboard::convert_layout(const string& word,
                                      const unordered_map<unsigned, unsigned>& char_map,
				      string& result) const
{
    result.clear();

    for (Utf8Iterator it(word); it != Utf8Iterator(); ++it) {
	unsigned ch = *it;
	unordered_map<unsigned, unsigned>::const_iterator char_it = char_map.find(ch);

	if (char_it != char_map.end()) {
	    ch = char_it->second;
	}
	else if (!default_keyboard.is_default(ch)) return false;

	Unicode::append_utf8(result, ch);
    }
    return true;
}

bool
SpellingKeyboard::convert_to_layout(const string& word, string& result) const
{
    return convert_layout(word, to_char_map, result);
}

bool
SpellingKeyboard::convert_from_layout(const string& word, string& result) const
{
    return convert_layout(word, from_char_map, result);
}

double
SpellingKeyboard::get_key_proximity(unsigned first_ch, unsigned second_ch) const
{
    unordered_map<unsigned, unsigned>::const_iterator first_it = from_char_map.find(first_ch);
    unordered_map<unsigned, unsigned>::const_iterator second_it = from_char_map.find(second_ch);

    if (first_it != from_char_map.end())
	first_ch = first_it->second;
    else if (!default_keyboard.is_default(first_ch)) return 0.0;

    if (second_it != from_char_map.end())
	second_ch = second_it->second;
    else if (!default_keyboard.is_default(second_ch)) return 0.0;

    return default_keyboard.get_key_proximity(first_ch, second_ch);
}

const string&
SpellingKeyboard::get_lang_name() const
{
    return language_name;
}

const string&
SpellingKeyboard::get_lang_code() const
{
    return language_code;
}
