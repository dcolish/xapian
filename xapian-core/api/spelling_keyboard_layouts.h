/** @file spelling_keyboard_russian.h
 * @brief Spelling russian keyboard layout.
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

#ifndef XAPIAN_INCLUDED_SPELLING_KEYBOARD_LAYOUTS_H
#define XAPIAN_INCLUDED_SPELLING_KEYBOARD_LAYOUTS_H

#include <string>

#include "spelling_keyboard.h"

class RussianSpellingKeyboard : public SpellingKeyboard {

public:
    RussianSpellingKeyboard()
    {
	from_char_map.insert(make_pair(0x0439, 'q'));
	from_char_map.insert(make_pair(0x0446, 'w'));
	from_char_map.insert(make_pair(0x0443, 'e'));
	from_char_map.insert(make_pair(0x043a, 'r'));
	from_char_map.insert(make_pair(0x0435, 't'));
	from_char_map.insert(make_pair(0x043d, 'y'));
	from_char_map.insert(make_pair(0x0433, 'u'));
	from_char_map.insert(make_pair(0x0448, 'i'));
	from_char_map.insert(make_pair(0x0449, 'o'));
	from_char_map.insert(make_pair(0x0437, 'p'));
	from_char_map.insert(make_pair(0x0445, 0x005b));
	from_char_map.insert(make_pair(0x044a, 0x005d));
	from_char_map.insert(make_pair(0x0444, 'a'));
	from_char_map.insert(make_pair(0x044b, 's'));
	from_char_map.insert(make_pair(0x0432, 'd'));
	from_char_map.insert(make_pair(0x0430, 'f'));
	from_char_map.insert(make_pair(0x043f, 'g'));
	from_char_map.insert(make_pair(0x0440, 'h'));
	from_char_map.insert(make_pair(0x043e, 'j'));
	from_char_map.insert(make_pair(0x043b, 'k'));
	from_char_map.insert(make_pair(0x0434, 'l'));
	from_char_map.insert(make_pair(0x0436, 0x003b));
	from_char_map.insert(make_pair(0x044d, 0x0027));
	from_char_map.insert(make_pair(0x0451, 0x0060));
	from_char_map.insert(make_pair(0x044f, 'z'));
	from_char_map.insert(make_pair(0x0447, 'x'));
	from_char_map.insert(make_pair(0x0441, 'c'));
	from_char_map.insert(make_pair(0x043c, 'v'));
	from_char_map.insert(make_pair(0x0438, 'b'));
	from_char_map.insert(make_pair(0x0442, 'n'));
	from_char_map.insert(make_pair(0x044c, 'm'));
	from_char_map.insert(make_pair(0x0431, 0x002c));
	from_char_map.insert(make_pair(0x044e, 0x002e));
	from_char_map.insert(make_pair(0x002e, 0x002f));
	from_char_map.insert(make_pair(0x002f, 0x007c));
	from_char_map.insert(make_pair(0x002c, 0x002e));

	from_char_map.insert(make_pair(0x0022, 0x0040));
	from_char_map.insert(make_pair(0x2116, 0x0023));
	from_char_map.insert(make_pair(0x003b, 0x0024));
	from_char_map.insert(make_pair(0x003a, 0x005e));
	from_char_map.insert(make_pair(0x003f, 0x0026));
	from_char_map.insert(make_pair(0x0419, 'Q'));
	from_char_map.insert(make_pair(0x0426, 'W'));
	from_char_map.insert(make_pair(0x0423, 'E'));
	from_char_map.insert(make_pair(0x041a, 'R'));
	from_char_map.insert(make_pair(0x0415, 'T'));
	from_char_map.insert(make_pair(0x041d, 'Y'));
	from_char_map.insert(make_pair(0x0413, 'U'));
	from_char_map.insert(make_pair(0x0428, 'I'));
	from_char_map.insert(make_pair(0x0429, 'O'));
	from_char_map.insert(make_pair(0x0417, 'P'));
	from_char_map.insert(make_pair(0x0425, 0x007b));
	from_char_map.insert(make_pair(0x042a, 0x007d));
	from_char_map.insert(make_pair(0x0424, 'A'));
	from_char_map.insert(make_pair(0x042b, 'S'));
	from_char_map.insert(make_pair(0x0412, 'D'));
	from_char_map.insert(make_pair(0x0410, 'F'));
	from_char_map.insert(make_pair(0x041f, 'G'));
	from_char_map.insert(make_pair(0x0420, 'H'));
	from_char_map.insert(make_pair(0x041e, 'J'));
	from_char_map.insert(make_pair(0x041b, 'K'));
	from_char_map.insert(make_pair(0x0414, 'L'));
	from_char_map.insert(make_pair(0x0416, 0x003a));
	from_char_map.insert(make_pair(0x042d, 0x0022));
	from_char_map.insert(make_pair(0x0401, 0x007e));
	from_char_map.insert(make_pair(0x002f, 0x007c));
	from_char_map.insert(make_pair(0x042f, 'Z'));
	from_char_map.insert(make_pair(0x0427, 'X'));
	from_char_map.insert(make_pair(0x0421, 'C'));
	from_char_map.insert(make_pair(0x041c, 'V'));
	from_char_map.insert(make_pair(0x0418, 'B'));
	from_char_map.insert(make_pair(0x0422, 'N'));
	from_char_map.insert(make_pair(0x042c, 'M'));
	from_char_map.insert(make_pair(0x0411, 0x003c));
	from_char_map.insert(make_pair(0x042e, 0x003e));
	from_char_map.insert(make_pair(0x002c, 0x003f));

	copy_map(from_char_map, to_char_map);
    }
};

class FrenchSpellingKeyboard : public SpellingKeyboard {

public:
    FrenchSpellingKeyboard()
    {
	from_char_map.insert(make_pair(0x0026, '1'));
	from_char_map.insert(make_pair(0x00e9, '2'));
	from_char_map.insert(make_pair(0x0022, '3'));
	from_char_map.insert(make_pair(0x0027, '4'));
	from_char_map.insert(make_pair(0x0028, '5'));
	from_char_map.insert(make_pair(0x002d, '6'));
	from_char_map.insert(make_pair(0x00e8, '7'));
	from_char_map.insert(make_pair(0x005f, '8'));
	from_char_map.insert(make_pair(0x00e7, '9'));
	from_char_map.insert(make_pair(0x00e0, '0'));
	from_char_map.insert(make_pair(0x0029, 0x002d));
	from_char_map.insert(make_pair('a', 'q'));
	from_char_map.insert(make_pair('z', 'w'));
	from_char_map.insert(make_pair(0x005e, 0x005b));
	from_char_map.insert(make_pair(0x0024, 0x005d));
	from_char_map.insert(make_pair('q', 'a'));
	from_char_map.insert(make_pair('m', 0x003b));
	from_char_map.insert(make_pair(0x00f9, 0x0027));
	from_char_map.insert(make_pair(0x00b2, 0x0060));
	from_char_map.insert(make_pair(0x002a, 0x005c));
	from_char_map.insert(make_pair('w', 'z'));
	from_char_map.insert(make_pair(0x002c, 'm'));
	from_char_map.insert(make_pair(0x003b, 0x002c));
	from_char_map.insert(make_pair(0x003a, 0x002e));
	from_char_map.insert(make_pair(0x0021, 0x002f));
	from_char_map.insert(make_pair(0x003e, 0x007c));

	from_char_map.insert(make_pair('1', 0x0021));
	from_char_map.insert(make_pair('2', 0x0040));
	from_char_map.insert(make_pair('3', 0x0023));
	from_char_map.insert(make_pair('4', 0x0024));
	from_char_map.insert(make_pair('5', 0x0025));
	from_char_map.insert(make_pair('6', 0x005e));
	from_char_map.insert(make_pair('7', 0x0026));
	from_char_map.insert(make_pair('8', 0x002a));
	from_char_map.insert(make_pair('9', 0x0028));
	from_char_map.insert(make_pair('0', 0x0029));
	from_char_map.insert(make_pair(0x00b0, 0x005f));
	from_char_map.insert(make_pair('A', 'Q'));
	from_char_map.insert(make_pair('Z', 'W'));
	from_char_map.insert(make_pair(0x00a8, 0x007b));
	from_char_map.insert(make_pair(0x00a3, 0x007d));
	from_char_map.insert(make_pair('Q', 'A'));
	from_char_map.insert(make_pair('M', 0x003a));
	from_char_map.insert(make_pair(0x0025, 0x0022));
	from_char_map.insert(make_pair(0x00b5, 0x007c));
	from_char_map.insert(make_pair('W', 'Z'));
	from_char_map.insert(make_pair(0x003f, 'M'));
	from_char_map.insert(make_pair(0x002e, 0x003c));
	from_char_map.insert(make_pair(0x002f, 0x003e));
	from_char_map.insert(make_pair(0x00a7, 0x003f));

	copy_map(from_char_map, to_char_map);
    }
};

class SpainSpellingKeyboard : public SpellingKeyboard {

public:
    SpainSpellingKeyboard()
    {
	from_char_map.insert(make_pair(0x0027, 0x002d));
	from_char_map.insert(make_pair(0x00a1, 0x003d));
	from_char_map.insert(make_pair(0x0060, 0x005b));
	from_char_map.insert(make_pair(0x002b, 0x005d));
	from_char_map.insert(make_pair(0x00f1, 0x003b));
	from_char_map.insert(make_pair(0x00b4, 0x0027));
	from_char_map.insert(make_pair(0x00ba, 0x0060));
	from_char_map.insert(make_pair(0x00e7, 0x005c));
	from_char_map.insert(make_pair(0x002d, 0x002f));
	from_char_map.insert(make_pair(0x003e, 0x007c));

	from_char_map.insert(make_pair(0x0022, 0x0040));
	from_char_map.insert(make_pair(0x00b7, 0x0023));
	from_char_map.insert(make_pair(0x0026, 0x005e));
	from_char_map.insert(make_pair(0x002f, 0x0026));
	from_char_map.insert(make_pair(0x0028, 0x002a));
	from_char_map.insert(make_pair(0x0029, 0x0028));
	from_char_map.insert(make_pair(0x003d, 0x0029));
	from_char_map.insert(make_pair(0x003f, 0x005f));
	from_char_map.insert(make_pair(0x00bf, 0x002b));
	from_char_map.insert(make_pair(0x005e, 0x007b));
	from_char_map.insert(make_pair(0x002a, 0x007d));
	from_char_map.insert(make_pair(0x00d1, 0x003a));
	from_char_map.insert(make_pair(0x00a8, 0x0022));
	from_char_map.insert(make_pair(0x00aa, 0x007e));
	from_char_map.insert(make_pair(0x00c7, 0x007c));
	from_char_map.insert(make_pair(0x003b, 0x003c));
	from_char_map.insert(make_pair(0x003a, 0x003e));
	from_char_map.insert(make_pair(0x005f, 0x003f));

	copy_map(from_char_map, to_char_map);
    }
};

class ArabicSpellingKeyboard : public SpellingKeyboard {

public:
    ArabicSpellingKeyboard()
    {
	from_char_map.insert(make_pair(0x0636, 'q'));
	from_char_map.insert(make_pair(0x0635, 'w'));
	from_char_map.insert(make_pair(0x062b, 'e'));
	from_char_map.insert(make_pair(0x0642, 'r'));
	from_char_map.insert(make_pair(0x0641, 't'));
	from_char_map.insert(make_pair(0x063a, 'y'));
	from_char_map.insert(make_pair(0x0639, 'u'));
	from_char_map.insert(make_pair(0x0647, 'i'));
	from_char_map.insert(make_pair(0x062e, 'o'));
	from_char_map.insert(make_pair(0x062d, 'p'));
	from_char_map.insert(make_pair(0x062c, 0x005b));
	from_char_map.insert(make_pair(0x062f, 0x005d));
	from_char_map.insert(make_pair(0x0634, 'a'));
	from_char_map.insert(make_pair(0x0633, 's'));
	from_char_map.insert(make_pair(0x064a, 'd'));
	from_char_map.insert(make_pair(0x0628, 'f'));
	from_char_map.insert(make_pair(0x0644, 'g'));
	from_char_map.insert(make_pair(0x0627, 'h'));
	from_char_map.insert(make_pair(0x062a, 'j'));
	from_char_map.insert(make_pair(0x0646, 'k'));
	from_char_map.insert(make_pair(0x0645, 'l'));
	from_char_map.insert(make_pair(0x0643, 0x003b));
	from_char_map.insert(make_pair(0x0637, 0x0027));
	from_char_map.insert(make_pair(0x0630, 0x0060));
	from_char_map.insert(make_pair(0x0626, 'z'));
	from_char_map.insert(make_pair(0x0621, 'x'));
	from_char_map.insert(make_pair(0x0624, 'c'));
	from_char_map.insert(make_pair(0x0631, 'v'));
	from_char_map.insert(make_pair(0x0649, 'n'));
	from_char_map.insert(make_pair(0x0629, 'm'));
	from_char_map.insert(make_pair(0x0648, 0x002c));
	from_char_map.insert(make_pair(0x0632, 0x002e));
	from_char_map.insert(make_pair(0x0638, 0x002f));

	from_char_map.insert(make_pair(0x0029, 0x0028));
	from_char_map.insert(make_pair(0x0028, 0x0029));
	from_char_map.insert(make_pair(0x064e, 'Q'));
	from_char_map.insert(make_pair(0x064b, 'W'));
	from_char_map.insert(make_pair(0x064f, 'E'));
	from_char_map.insert(make_pair(0x064c, 'R'));
	from_char_map.insert(make_pair(0x0625, 'Y'));
	from_char_map.insert(make_pair(0x2018, 'U'));
	from_char_map.insert(make_pair(0x00f7, 'I'));
	from_char_map.insert(make_pair(0x00d7, 'O'));
	from_char_map.insert(make_pair(0x061b, 'P'));
	from_char_map.insert(make_pair(0x003c, 0x007b));
	from_char_map.insert(make_pair(0x003e, 0x007d));
	from_char_map.insert(make_pair(0x0650, 'A'));
	from_char_map.insert(make_pair(0x064d, 'S'));
	from_char_map.insert(make_pair(0x005d, 'D'));
	from_char_map.insert(make_pair(0x005b, 'F'));
	from_char_map.insert(make_pair(0x0623, 'H'));
	from_char_map.insert(make_pair(0x0640, 'J'));
	from_char_map.insert(make_pair(0x060c, 'K'));
	from_char_map.insert(make_pair(0x002f, 'L'));
	from_char_map.insert(make_pair(0x0651, 0x007e));
	from_char_map.insert(make_pair(0x007e, 'Z'));
	from_char_map.insert(make_pair(0x0652, 'X'));
	from_char_map.insert(make_pair(0x007d, 'C'));
	from_char_map.insert(make_pair(0x007b, 'V'));
	from_char_map.insert(make_pair(0x0622, 'N'));
	from_char_map.insert(make_pair(0x2019, 'M'));
	from_char_map.insert(make_pair(0x002c, 0x003c));
	from_char_map.insert(make_pair(0x002e, 0x003e));
	from_char_map.insert(make_pair(0x061f, 0x003f));

	copy_map(from_char_map, to_char_map);
    }
};

#endif // XAPIAN_INCLUDED_SPELLING_KEYBOARD_LAYOUTS_H
