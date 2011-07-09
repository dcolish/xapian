/** @file spelling_phonetic_dmsoundex.cc
 * @brief Spelling phonetic Daitch-Mokotoff Soundex algorithm.
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
#include <algorithm>
#include <cctype>

#include "spelling_phonetic_dmsoundex.h"

using namespace std;

DMSoundexSpellingPhonetic::DMSoundexSpellingPhonetic() : max_entry_length(0)
{
    add_entry("ai", true, "0", "1", "", NULL);
    add_entry("aj", true, "0", "1", "", NULL);
    add_entry("ay", true, "0", "1", "", NULL);
    add_entry("au", true, "0", "7", "", NULL);
    add_entry("a", true, "0", "", "", NULL);
    add_entry("b", false, "7", "7", "7", NULL);
    add_entry("chs", false, "5", "54", "54", NULL);
    add_entry("ch", false, "5", "5", "5", "tch");
    add_entry("ck", false, "5", "5", "5", "tsk");
    add_entry("c", false, "5", "5", "5", "tz");
    add_entry("cz", false, "4", "4", "4", NULL);
    add_entry("cs", false, "4", "4", "4", NULL);
    add_entry("csz", false, "4", "4", "4", NULL);
    add_entry("czs", false, "4", "4", "4", NULL);
    add_entry("drz", false, "4", "4", "4", NULL);
    add_entry("drs", false, "4", "4", "4", NULL);
    add_entry("ds", false, "4", "4", "4", NULL);
    add_entry("dsh", false, "4", "4", "4", NULL);
    add_entry("dsh", false, "4", "4", "4", NULL);
    add_entry("dz", false, "4", "4", "4", NULL);
    add_entry("dzh", false, "4", "4", "4", NULL);
    add_entry("dzs", false, "4", "4", "4", NULL);
    add_entry("d", false, "3", "3", "3", NULL);
    add_entry("dt", false, "3", "3", "3", NULL);
    add_entry("ei", true, "0", "1", "", NULL);
    add_entry("ey", true, "0", "1", "", NULL);
    add_entry("ej", true, "0", "1", "", NULL);
    add_entry("eu", true, "1", "1", "", NULL);
    add_entry("e", true, "0", "", "", NULL);
    add_entry("fb", false, "7", "7", "7", NULL);
    add_entry("f", false, "7", "7", "7", NULL);
    add_entry("g", false, "5", "5", "5", NULL);
    add_entry("h", false, "5", "5", "", NULL);
    add_entry("ia", true, "1", "", "", NULL);
    add_entry("ie", true, "1", "", "", NULL);
    add_entry("io", true, "1", "", "", NULL);
    add_entry("iu", true, "1", "", "", NULL);
    add_entry("i", true, "0", "", "", NULL);
    add_entry("j", false, "1", "1", "1", "dzh");
    add_entry("ks", false, "5", "54", "54", NULL);
    add_entry("kh", false, "5", "5", "5", NULL);
    add_entry("k", false, "5", "5", "5", NULL);
    add_entry("l", false, "8", "8", "8", NULL);
    add_entry("mn", false, "", "66", "66", NULL);
    add_entry("m", false, "6", "6", "6", NULL);
    add_entry("nm", false, "", "66", "66", NULL);
    add_entry("n", false, "6", "6", "6", NULL);
    add_entry("oi", true, "0", "1", "", NULL);
    add_entry("oj", true, "0", "1", "", NULL);
    add_entry("oy", true, "0", "1", "", NULL);
    add_entry("o", true, "0", "", "", NULL);
    add_entry("p", false, "7", "7", "7", NULL);
    add_entry("pf", false, "7", "7", "7", NULL);
    add_entry("ph", false, "7", "7", "7", NULL);
    add_entry("q", false, "5", "5", "5", NULL);
    add_entry("rz", false, "94", "94", "94", NULL);
    add_entry("rs", false, "94", "94", "94", NULL);
    add_entry("r", false, "9", "9", "9", NULL);
    add_entry("schtsch", false, "2", "4", "4", NULL);
    add_entry("schtsh", false, "2", "4", "4", NULL);
    add_entry("schtch", false, "2", "4", "4", NULL);
    add_entry("sch", false, "4", "4", "4", NULL);
    add_entry("shtch", false, "2", "4", "4", NULL);
    add_entry("shch", false, "2", "4", "4", NULL);
    add_entry("shtsh", false, "2", "4", "4", NULL);
    add_entry("sht", false, "2", "43", "43", NULL);
    add_entry("scht", false, "2", "43", "43", NULL);
    add_entry("schd", false, "2", "43", "43", NULL);
    add_entry("sh", false, "4", "4", "4", NULL);
    add_entry("stch", false, "2", "4", "4", NULL);
    add_entry("stsch", false, "2", "4", "4", NULL);
    add_entry("sc", false, "2", "4", "4", NULL);
    add_entry("strz", false, "2", "4", "4", NULL);
    add_entry("strs", false, "2", "4", "4", NULL);
    add_entry("stsh", false, "2", "4", "4", NULL);
    add_entry("st", false, "2", "43", "43", NULL);
    add_entry("szcz", false, "2", "4", "4", NULL);
    add_entry("szcs", false, "2", "4", "4", NULL);
    add_entry("szt", false, "2", "43", "43", NULL);
    add_entry("shd", false, "2", "43", "43", NULL);
    add_entry("szd", false, "2", "43", "43", NULL);
    add_entry("sd", false, "2", "43", "43", NULL);
    add_entry("sz", false, "4", "4", "4", NULL);
    add_entry("s", false, "4", "4", "4", NULL);
    add_entry("tch", false, "4", "4", "4", NULL);
    add_entry("ttch", false, "4", "4", "4", NULL);
    add_entry("ttsch", false, "4", "4", "4", NULL);
    add_entry("th", false, "3", "3", "3", NULL);
    add_entry("trz", false, "4", "4", "4", NULL);
    add_entry("trs", false, "4", "4", "4", NULL);
    add_entry("trch", false, "4", "4", "4", NULL);
    add_entry("tsh", false, "4", "4", "4", NULL);
    add_entry("ts", false, "4", "4", "4", NULL);
    add_entry("tts", false, "4", "4", "4", NULL);
    add_entry("ttsz", false, "4", "4", "4", NULL);
    add_entry("tc", false, "4", "4", "4", NULL);
    add_entry("tz", false, "4", "4", "4", NULL);
    add_entry("ttz", false, "4", "4", "4", NULL);
    add_entry("tzs", false, "4", "4", "4", NULL);
    add_entry("tsz", false, "4", "4", "4", NULL);
    add_entry("t", false, "3", "3", "3", NULL);
    add_entry("ui", true, "0", "1", "", NULL);
    add_entry("uj", true, "0", "1", "", NULL);
    add_entry("uy", true, "0", "1", "", NULL);
    add_entry("u", true, "0", "", "", NULL);
    add_entry("ue", true, "0", "", "", NULL);
    add_entry("v", false, "7", "7", "7", NULL);
    add_entry("w", false, "7", "7", "7", NULL);
    add_entry("x", false, "5", "54", "54", NULL);
    add_entry("y", true, "1", "", "", NULL);
    add_entry("zh", false, "4", "4", "4", NULL);
    add_entry("zs", false, "4", "4", "4", NULL);
    add_entry("zsch", false, "4", "4", "4", NULL);
    add_entry("zhsh", false, "4", "4", "4", NULL);
    add_entry("z", false, "4", "4", "4", NULL);

}

void DMSoundexSpellingPhonetic::add_entry(const char* str, bool vowel, const char* first, const char* before,
					  const char* other, const char* alternate)
{
    Entry entry;
    entry.vowel = vowel;
    entry.first = first;
    entry.before = before;
    entry.other = other;
    entry.alternate = alternate;

    string key(str);
    entry_map.insert(make_pair(key, entry));
    max_entry_length = max(key.length(), max_entry_length);
}

unsigned DMSoundexSpellingPhonetic::find_entry(const string& word, unsigned offset, Entry& entry, string& buffer) const
{
    buffer.clear();

    unsigned entry_length = 0;
    for (unsigned i = offset; i < min(offset + max_entry_length, word.length()); ++i)
    {
	buffer.push_back(word[i]);

	unordered_map<string, Entry>::const_iterator it = entry_map.find(buffer);
	if (it == entry_map.end()) break;

	entry = it->second;
	entry_length++;
    }
    return entry_length;
}

const char* DMSoundexSpellingPhonetic::get_entry_value(const std::vector<Entry>& entries, unsigned index, const Entry& entry)
{
    if (index < entries.size() && entries[index + 1].vowel)
	return entry.before;
    return entry.other;
}

void DMSoundexSpellingPhonetic::get_phonetic(const string& input, vector<string>& result)
{
    vector<Entry> entries;

    string temp_string;
    Entry temp_entry;

    result.clear();

    string word = input;
    transform(word.begin(), word.end(), word.begin(), (int(*)(int))std::tolower);

    unsigned offset = 0;
    while (offset < word.length())
    {
	unsigned entry_length = find_entry(word, offset, temp_entry, temp_string);
	if (entry_length == 0) return;
	entries.push_back(temp_entry);
	offset += entry_length;
    }

    result.push_back(entries[0].first);
    if (entries[0].alternate != NULL)
    {
	temp_entry = entry_map.find(entries[0].alternate)->second;
	result.push_back(temp_entry.first);
    }

    for(unsigned i = 1; i < entries.size(); ++i)
    {
	unsigned result_size = result.size();
	const char* entry_value = get_entry_value(entries, i, entries[i]);
	if (entries[i].alternate != NULL)
	{
	    temp_entry = entry_map.find(entries[i].alternate)->second;
	    const char* entry_value_alt = get_entry_value(entries, i, temp_entry);

	    for (unsigned k = 0; k < result_size; ++k)
		result.push_back(result[k] + entry_value_alt);
	}

	for (unsigned k = 0; k < result_size; ++k)
	    result[k].append(entry_value);
    }

    for (unsigned i = 0; i < result.size(); ++i)
    {
	string& key = result[i];
	key.resize(unique(key.begin(), key.end()) - key.begin());
    }
}
