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

class EnglishSpellingKeyboard : public SpellingKeyboard {

public:
    EnglishSpellingKeyboard() : SpellingKeyboard("english", "en")
    {
    }
};

class RussianSpellingKeyboard : public SpellingKeyboard {

public:
    RussianSpellingKeyboard() : SpellingKeyboard("russian", "ru")
    {
	add_char_mapping(0x0439, 'q');
	add_char_mapping(0x0446, 'w');
	add_char_mapping(0x0443, 'e');
	add_char_mapping(0x043a, 'r');
	add_char_mapping(0x0435, 't');
	add_char_mapping(0x043d, 'y');
	add_char_mapping(0x0433, 'u');
	add_char_mapping(0x0448, 'i');
	add_char_mapping(0x0449, 'o');
	add_char_mapping(0x0437, 'p');
	add_char_mapping(0x0445, 0x005b);
	add_char_mapping(0x044a, 0x005d);
	add_char_mapping(0x0444, 'a');
	add_char_mapping(0x044b, 's');
	add_char_mapping(0x0432, 'd');
	add_char_mapping(0x0430, 'f');
	add_char_mapping(0x043f, 'g');
	add_char_mapping(0x0440, 'h');
	add_char_mapping(0x043e, 'j');
	add_char_mapping(0x043b, 'k');
	add_char_mapping(0x0434, 'l');
	add_char_mapping(0x0436, 0x003b);
	add_char_mapping(0x044d, 0x0027);
	add_char_mapping(0x0451, 0x0060);
	add_char_mapping(0x044f, 'z');
	add_char_mapping(0x0447, 'x');
	add_char_mapping(0x0441, 'c');
	add_char_mapping(0x043c, 'v');
	add_char_mapping(0x0438, 'b');
	add_char_mapping(0x0442, 'n');
	add_char_mapping(0x044c, 'm');
	add_char_mapping(0x0431, 0x002c);
	add_char_mapping(0x044e, 0x002e);
	add_char_mapping(0x002e, 0x002f);
	add_char_mapping(0x002f, 0x007c);
	add_char_mapping(0x002c, 0x002e);

	add_char_mapping(0x0022, 0x0040);
	add_char_mapping(0x2116, 0x0023);
	add_char_mapping(0x003b, 0x0024);
	add_char_mapping(0x003a, 0x005e);
	add_char_mapping(0x003f, 0x0026);
	add_char_mapping(0x0419, 'Q');
	add_char_mapping(0x0426, 'W');
	add_char_mapping(0x0423, 'E');
	add_char_mapping(0x041a, 'R');
	add_char_mapping(0x0415, 'T');
	add_char_mapping(0x041d, 'Y');
	add_char_mapping(0x0413, 'U');
	add_char_mapping(0x0428, 'I');
	add_char_mapping(0x0429, 'O');
	add_char_mapping(0x0417, 'P');
	add_char_mapping(0x0425, 0x007b);
	add_char_mapping(0x042a, 0x007d);
	add_char_mapping(0x0424, 'A');
	add_char_mapping(0x042b, 'S');
	add_char_mapping(0x0412, 'D');
	add_char_mapping(0x0410, 'F');
	add_char_mapping(0x041f, 'G');
	add_char_mapping(0x0420, 'H');
	add_char_mapping(0x041e, 'J');
	add_char_mapping(0x041b, 'K');
	add_char_mapping(0x0414, 'L');
	add_char_mapping(0x0416, 0x003a);
	add_char_mapping(0x042d, 0x0022);
	add_char_mapping(0x0401, 0x007e);
	add_char_mapping(0x002f, 0x007c);
	add_char_mapping(0x042f, 'Z');
	add_char_mapping(0x0427, 'X');
	add_char_mapping(0x0421, 'C');
	add_char_mapping(0x041c, 'V');
	add_char_mapping(0x0418, 'B');
	add_char_mapping(0x0422, 'N');
	add_char_mapping(0x042c, 'M');
	add_char_mapping(0x0411, 0x003c);
	add_char_mapping(0x042e, 0x003e);
	add_char_mapping(0x002c, 0x003f);
    }
};

class FrenchSpellingKeyboard : public SpellingKeyboard {

public:
    FrenchSpellingKeyboard() : SpellingKeyboard("french", "fr")
    {
	add_char_mapping(0x0026, '1');
	add_char_mapping(0x00e9, '2');
	add_char_mapping(0x0022, '3');
	add_char_mapping(0x0027, '4');
	add_char_mapping(0x0028, '5');
	add_char_mapping(0x002d, '6');
	add_char_mapping(0x00e8, '7');
	add_char_mapping(0x005f, '8');
	add_char_mapping(0x00e7, '9');
	add_char_mapping(0x00e0, '0');
	add_char_mapping(0x0029, 0x002d);
	add_char_mapping('a', 'q');
	add_char_mapping('z', 'w');
	add_char_mapping(0x005e, 0x005b);
	add_char_mapping(0x0024, 0x005d);
	add_char_mapping('q', 'a');
	add_char_mapping('m', 0x003b);
	add_char_mapping(0x00f9, 0x0027);
	add_char_mapping(0x00b2, 0x0060);
	add_char_mapping(0x002a, 0x005c);
	add_char_mapping('w', 'z');
	add_char_mapping(0x002c, 'm');
	add_char_mapping(0x003b, 0x002c);
	add_char_mapping(0x003a, 0x002e);
	add_char_mapping(0x0021, 0x002f);
	add_char_mapping(0x003e, 0x007c);

	add_char_mapping('1', 0x0021);
	add_char_mapping('2', 0x0040);
	add_char_mapping('3', 0x0023);
	add_char_mapping('4', 0x0024);
	add_char_mapping('5', 0x0025);
	add_char_mapping('6', 0x005e);
	add_char_mapping('7', 0x0026);
	add_char_mapping('8', 0x002a);
	add_char_mapping('9', 0x0028);
	add_char_mapping('0', 0x0029);
	add_char_mapping(0x00b0, 0x005f);
	add_char_mapping('A', 'Q');
	add_char_mapping('Z', 'W');
	add_char_mapping(0x00a8, 0x007b);
	add_char_mapping(0x00a3, 0x007d);
	add_char_mapping('Q', 'A');
	add_char_mapping('M', 0x003a);
	add_char_mapping(0x0025, 0x0022);
	add_char_mapping(0x00b5, 0x007c);
	add_char_mapping('W', 'Z');
	add_char_mapping(0x003f, 'M');
	add_char_mapping(0x002e, 0x003c);
	add_char_mapping(0x002f, 0x003e);
	add_char_mapping(0x00a7, 0x003f);
    }
};

class SpainSpellingKeyboard : public SpellingKeyboard {

public:
    SpainSpellingKeyboard() : SpellingKeyboard("spain", "sp")
    {
	add_char_mapping(0x0027, 0x002d);
	add_char_mapping(0x00a1, 0x003d);
	add_char_mapping(0x0060, 0x005b);
	add_char_mapping(0x002b, 0x005d);
	add_char_mapping(0x00f1, 0x003b);
	add_char_mapping(0x00b4, 0x0027);
	add_char_mapping(0x00ba, 0x0060);
	add_char_mapping(0x00e7, 0x005c);
	add_char_mapping(0x002d, 0x002f);
	add_char_mapping(0x003e, 0x007c);

	add_char_mapping(0x0022, 0x0040);
	add_char_mapping(0x00b7, 0x0023);
	add_char_mapping(0x0026, 0x005e);
	add_char_mapping(0x002f, 0x0026);
	add_char_mapping(0x0028, 0x002a);
	add_char_mapping(0x0029, 0x0028);
	add_char_mapping(0x003d, 0x0029);
	add_char_mapping(0x003f, 0x005f);
	add_char_mapping(0x00bf, 0x002b);
	add_char_mapping(0x005e, 0x007b);
	add_char_mapping(0x002a, 0x007d);
	add_char_mapping(0x00d1, 0x003a);
	add_char_mapping(0x00a8, 0x0022);
	add_char_mapping(0x00aa, 0x007e);
	add_char_mapping(0x00c7, 0x007c);
	add_char_mapping(0x003b, 0x003c);
	add_char_mapping(0x003a, 0x003e);
	add_char_mapping(0x005f, 0x003f);
    }
};

class ArabicSpellingKeyboard : public SpellingKeyboard {

public:
    ArabicSpellingKeyboard() : SpellingKeyboard("arabic", "ar")
    {
	add_char_mapping(0x0636, 'q');
	add_char_mapping(0x0635, 'w');
	add_char_mapping(0x062b, 'e');
	add_char_mapping(0x0642, 'r');
	add_char_mapping(0x0641, 't');
	add_char_mapping(0x063a, 'y');
	add_char_mapping(0x0639, 'u');
	add_char_mapping(0x0647, 'i');
	add_char_mapping(0x062e, 'o');
	add_char_mapping(0x062d, 'p');
	add_char_mapping(0x062c, 0x005b);
	add_char_mapping(0x062f, 0x005d);
	add_char_mapping(0x0634, 'a');
	add_char_mapping(0x0633, 's');
	add_char_mapping(0x064a, 'd');
	add_char_mapping(0x0628, 'f');
	add_char_mapping(0x0644, 'g');
	add_char_mapping(0x0627, 'h');
	add_char_mapping(0x062a, 'j');
	add_char_mapping(0x0646, 'k');
	add_char_mapping(0x0645, 'l');
	add_char_mapping(0x0643, 0x003b);
	add_char_mapping(0x0637, 0x0027);
	add_char_mapping(0x0630, 0x0060);
	add_char_mapping(0x0626, 'z');
	add_char_mapping(0x0621, 'x');
	add_char_mapping(0x0624, 'c');
	add_char_mapping(0x0631, 'v');
	add_char_mapping(0x0649, 'n');
	add_char_mapping(0x0629, 'm');
	add_char_mapping(0x0648, 0x002c);
	add_char_mapping(0x0632, 0x002e);
	add_char_mapping(0x0638, 0x002f);

	add_char_mapping(0x0029, 0x0028);
	add_char_mapping(0x0028, 0x0029);
	add_char_mapping(0x064e, 'Q');
	add_char_mapping(0x064b, 'W');
	add_char_mapping(0x064f, 'E');
	add_char_mapping(0x064c, 'R');
	add_char_mapping(0x0625, 'Y');
	add_char_mapping(0x2018, 'U');
	add_char_mapping(0x00f7, 'I');
	add_char_mapping(0x00d7, 'O');
	add_char_mapping(0x061b, 'P');
	add_char_mapping(0x003c, 0x007b);
	add_char_mapping(0x003e, 0x007d);
	add_char_mapping(0x0650, 'A');
	add_char_mapping(0x064d, 'S');
	add_char_mapping(0x005d, 'D');
	add_char_mapping(0x005b, 'F');
	add_char_mapping(0x0623, 'H');
	add_char_mapping(0x0640, 'J');
	add_char_mapping(0x060c, 'K');
	add_char_mapping(0x002f, 'L');
	add_char_mapping(0x0651, 0x007e);
	add_char_mapping(0x007e, 'Z');
	add_char_mapping(0x0652, 'X');
	add_char_mapping(0x007d, 'C');
	add_char_mapping(0x007b, 'V');
	add_char_mapping(0x0622, 'N');
	add_char_mapping(0x2019, 'M');
	add_char_mapping(0x002c, 0x003c);
	add_char_mapping(0x002e, 0x003e);
	add_char_mapping(0x061f, 0x003f);
    }
};

#endif // XAPIAN_INCLUDED_SPELLING_KEYBOARD_LAYOUTS_H
