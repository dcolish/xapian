/** @file spelling_phonetic_metaphone.cc
 * @brief Spelling phonetic Metaphone algorithm.
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
/**
 * Ideas from C Gazette, June/July 1991, pp 56-57,
 * author Gary A. Parker, with changes by Bernard Tiffany of the
 * University of Michigan, and more changes by Tim Howes of the
 * University of Michigan.
 */

#include <config.h>
#include <algorithm>
#include <xapian/unicode.h>

#include "spelling_phonetic_metaphone.h"

using namespace std;
using namespace Xapian;

const char MetaphoneSpellingPhonetic::alpha[] = { VOWEL, NOGHF, VAR_SOUND, NOGHF,
                                                  VOWEL | FRONT_VOWEL, SAME,
						  VAR_SOUND, NOGHF, VOWEL | FRONT_VOWEL,
						  SAME, 0, SAME, SAME, SAME, VOWEL,
						  VAR_SOUND, 0, SAME, VAR_SOUND, VAR_SOUND,
						  VOWEL, 0, 0, 0, FRONT_VOWEL, 0 };

char
MetaphoneSpellingPhonetic::at(const std::string& word, int index)
{
    if (index < 0 || index >= int(word.size())) return 0;
    return word[index];
}

bool
MetaphoneSpellingPhonetic::is(char ch, Flag flag)
{
    return alpha[ch - 'A'] & flag;
}

bool
MetaphoneSpellingPhonetic::get_phonetic(const string& input, vector<string>& result) const
{
    string key;
    string word = Xapian::Unicode::toupper(input);

    for (unsigned i = 0; i < word.length(); ++i)
	if (word[i] < 'A' || word[i] > 'Z') return false;

    result.clear();

    if (word.length() <= 1) {
	result.push_back(word);
	return true;
    }

    if (((word[0] == 'P' || word[0] == 'K' || word[0] == 'G') && word[1] == 'N') ||
	(word[0] == 'A' && word[0] == 'E') || (word[0] == 'W' && word[1] == 'R')) {
	word.erase(0, 1);
    }

    if (word[0] == 'W' && word[1] == 'H') {
	word.erase(0, 1);
	word[0] = 'W';
    }

    if (word[0] == 'X') word[0] = 'S';

    int word_size = int(word.size());
    for (int i = 0; i < word_size; ++i) {

	char ch = at(word, i);
	char chm1 = at(word, i - 1);

	/* Drop duplicates except for CC */
	if (chm1 == ch && ch != 'C') continue;

	/* Check for FJLMNR or first letter vowel */
	if (is(ch, SAME) || (i == 0 && is(ch, VOWEL))) {
	    key.push_back(ch);

	} else {
	    char ch1 = at(word, i + 1);
	    char ch2 = at(word, i + 2);

	    switch (ch) {
		case 'B':
		    /*
		     * -MB => -M at the end
		     */
		    if (chm1 != 'M' || i + 1 < word_size) key.push_back('B');
		    break;

		case 'C':
		    /*
		     * in SCI, SCE, SCY => dropped
		     * in CIA, CH => X
		     * in CI, CE, CY => S
		     * else => K
		     */
		    if (chm1 != 'S' || !is(ch1, FRONT_VOWEL)) {

			if (ch1 == 'I' && ch2 == 'A')
			    key.push_back('X');
			else if (is(ch1, FRONT_VOWEL))
			    key.push_back('S');
			else if (ch1 == 'H')
			    key.push_back(((i > 0 || is(ch2, VOWEL)) && chm1 != 'S') ? 'X' : 'K');
			else key.push_back('K');
		    }
		    break;

		case 'D':
		    /*
		     * in DGE, DGI, DGY => J
		     * else => T
		     */
		    key.push_back((ch1 == 'G' && is(ch2, FRONT_VOWEL)) ? 'J' : 'T');
		    break;

		case 'G':
		    /*
		     * in -GH and not B--GH, D--GH, -H--GH, -H---GH => F
		     * in -GNED, -GN, -DGE-, -DGI-, -DGY- => dropped
		     * in -GE-, -GI-, -GY- and not GG => J
		     * else => K
		     */
		    if ((ch1 != 'G' || is(ch2, VOWEL)) &&
			(ch1 != 'N' || (i + 1 < word_size && (ch2 != 'E' || at(word, i + 3) != 'D'))) &&
			(chm1 != 'D' || !is(ch1, FRONT_VOWEL)))
			key.push_back((is(ch1, FRONT_VOWEL) && ch2 != 'G') ? 'J' : 'K');
		    else
			if (ch1 == 'H' && !is(at(word, i - 3), NOGHF) && at(word, i - 4) != 'H') key.push_back('F');
		    break;

		case 'H':
		    /*
		     * keep H before a vowel and not after CGPST (VAR_SOUND group)
		     */
		    if (!is(chm1, VAR_SOUND) && (!is(chm1, VOWEL) || is(ch1, VOWEL))) key.push_back('H');
		    break;

		case 'K':
		    /*
		     * keep if not after C
		     */
		    if (chm1 != 'C') key.push_back('K');
		    break;

		case 'P':
		    /*
		     * PH => F
		     */
		    key.push_back(ch1 == 'H' ? 'F' : 'P');
		    break;

		case 'Q':
		    key.push_back('K');
		    break;

		case 'S':
		    /*
		     * SH, SIO, SIA => X
		     */
		    key.push_back((ch1 == 'H' || (ch1 == 'I' && (ch2 == 'O' || ch2 == 'A'))) ? 'X' : 'S');
		    break;

		case 'T':
		    /*
		     * TIA, TIO => X
		     * TH => 0 (theta)
		     * TCH => dropped
		     */
		    if (ch1 == 'I' && (ch2 == 'O' || ch2 == 'A'))
			key.push_back('X');
		    else if (ch1 == 'H')
			key.push_back('0');
		    else if (ch1 != 'C' || ch2 != 'H') key.push_back('T');
		    break;

		case 'V':
		    key.push_back('F');
		    break;

		case 'W':
		case 'Y':
		    if (is(ch1, VOWEL)) key.push_back(ch);
		    break;

		case 'X':
		    key.append(i > 0 ? "KS" : "S");
		    break;

		case 'Z':
		    key.push_back('S');
		    break;
	    }
	}
    }
    result.push_back(key);
    return true;
}
