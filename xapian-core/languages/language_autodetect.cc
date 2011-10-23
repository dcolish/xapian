/** @file language_autodetect.cc
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

#include <fstream>
#include <sstream>
#include <xapian/unicode.h>
#include <xapian/language_autodetect.h>
#include <iostream>
using namespace std;
using namespace Xapian;

LanguageAutodetect::range
LanguageAutodetect::range_from_string(const string& data) const
{
    range result;
    result.start = 0;
    result.end = 0;
    if (data.empty()) return result;

    stringstream datastream(data);
    result.required = (data[0] == '!');

    if (result.required) {
	char dummy;
	datastream >> dummy;
    }

    string unicode_ch;
    if (getline(datastream, unicode_ch, '-')) {
	stringstream start_stream;
	start_stream << hex << unicode_ch;
	start_stream >> result.start;
	if (getline(datastream, unicode_ch)) {
	    stringstream end_stream;
	    end_stream << hex << unicode_ch;
	    end_stream >> result.end;
	}
	else result.start = 0;
    }
    return result;
}

LanguageAutodetect::LanguageAutodetect()
{
    ifstream lm("../languages/classification/languages", ifstream::in);
    if (lm.good()) {
	string language_line;
	while (getline(lm, language_line)) {
	    stringstream stream(language_line);
	    string language;
	    stream >> language;

	    string value;
	    while (stream >> value) {
		if (value.empty()) continue;
		range r = range_from_string(value);
		if (r.end >= r.start && r.end > 0)
		    language_ranges[language].push_back(r);
	    }
	    load_language(language, languages[language]);
	}
    }
}

LanguageAutodetect::LanguageAutodetect(vector<string> languages_)
{
    for (unsigned i = 0; i < languages_.size(); ++i)
	load_language(languages_[i], languages[languages_[i]]);
}

bool
LanguageAutodetect::is_word_char(unsigned int ch) const
{
    return Unicode::get_category(ch) != Unicode::DECIMAL_DIGIT_NUMBER && !Unicode::is_whitespace(ch);
}

bool
LanguageAutodetect::load_language(const string& language, map<string, unsigned>& result) const
{
    unsigned index = 1;

    ifstream lm(("../languages/classification/" + language + ".lm").c_str(), ifstream::in);
    if (!lm.good()) return false;

    string line;
    string result_line;
    while (getline(lm, line)) {
	result_line.clear();

	for (Utf8Iterator it(line); it != Utf8Iterator() && is_word_char(*it); ++it)
	    Unicode::append_utf8(result_line, *it);

	if (!result_line.empty())
	    result.insert(make_pair(result_line, index++));
    }
    lm.close();
    return true;
}

unsigned
LanguageAutodetect::check_language(const vector<string>& unknown,
                                   const std::string& language) const
{
    map<string, vector<range> >::const_iterator rit = language_ranges.find(language);
    if (rit != language_ranges.end()) {
	const vector<range>& ranges = rit->second;
	Utf8Iterator end_cit;
	bool required_match = true;

	for (unsigned i = 0; i < ranges.size() && required_match; ++i)
	    required_match = required_match && !ranges[i].required;

	for (unsigned i = 0; i < unknown.size(); ++i) {
	    for (Utf8Iterator cit(unknown[i]); cit != end_cit; ++cit) {
		unsigned ch = *cit;
		if (ch == '_') continue;

		bool match = false;
		for (unsigned k = 0; k < ranges.size() && !match; ++k) {
		    bool m = (ch >= ranges[k].start && ch <= ranges[k].end);
		    required_match = required_match || (ranges[k].required && m);
		    match = match || m;
		}
		if (!match) return unknown.size() * MAX_N_COUNT;
	    }
	}
	if (!required_match) return unknown.size() * MAX_N_COUNT;
    }

    const map<string, unsigned>& language_map = languages.at(language);

    unsigned result = 0;
    for (unsigned i = 0; i < unknown.size(); ++i) {
	map<string, unsigned>::const_iterator it = language_map.find(unknown[i]);
	if (it != language_map.end())
	    result += (it->second > i) ? it->second - i : i - it->second;
	else result += MAX_N_COUNT;
    }
    return result;
}

vector<string>
LanguageAutodetect::create_language_model(const string& text) const
{
    const char placeholder = '_';
    map<string, unsigned> language_map;

    string word;
    word.push_back(placeholder);
    bool word_empty = true;

    vector<unsigned> unicode_map;
    unicode_map.push_back(0);

    for (Utf8Iterator it(text), end_it;; ++it) {
	if (it == end_it || !is_word_char(*it)) {
	    if (!word_empty) {
		//Place placeholders at the start and the end.
		unicode_map.push_back(word.size());
		word.push_back(placeholder);
		unicode_map.push_back(word.size());

		for (unsigned w = 0; w < unicode_map.size() - 1; ++w)
		    for (unsigned l = 1; l <= min<size_t>(MAX_N, unicode_map.size() - w - 1); ++l)
			++language_map[word.substr(unicode_map[w],
			                           unicode_map[w + l] - unicode_map[w])];

		word.clear();
		word.push_back(placeholder);
		unicode_map.clear();
		unicode_map.push_back(0);
		word_empty = true;
	    }
	    if (it == end_it) break;
	} else {
	    unicode_map.push_back(word.size());
	    Unicode::append_utf8(word, *it);
	    word_empty = false;
	}
    }

    //Sort by count
    multimap<unsigned, string, greater<unsigned> > result_map;
    map<string, unsigned>::const_iterator lang_it;
    for (lang_it = language_map.begin(); lang_it != language_map.end(); ++lang_it)
	if (lang_it->second >= MIN_N_FREQ)
	    result_map.insert(make_pair(lang_it->second, lang_it->first));

    //Copy to result vector
    vector<string> result;
    multimap<unsigned, string, greater<unsigned> >::const_iterator rit;
    for (rit = result_map.begin(); rit != result_map.end() && result.size() <= MAX_N_COUNT; ++rit)
	result.push_back(rit->second);

    return result;
}

string
LanguageAutodetect::get_language(const string& text) const
{
    vector<string> unknown = create_language_model(text);

    map<string, unsigned> language_map;
    multimap<unsigned, string> result_map;

    map<string, map<string, unsigned> >::const_iterator it;
    for (it = languages.begin(); it != languages.end(); ++it) {
	unsigned score = check_language(unknown, it->first);
	result_map.insert(make_pair(score, it->first));
    }
    return !result_map.empty() ? result_map.begin()->second : string();
}
