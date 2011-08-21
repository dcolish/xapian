/** @file api_spelling.cc
 * @brief Test the spelling correction suggestion API.
 */
/* Copyright (C) 2007,2008,2009,2010,2011 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#include "api_spelling.h"

#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

#include <string>
#include "../spelling/spelling_phonetic.h"
#include <xapian/language_autodetect.h>

using namespace std;

// Test add_spelling() and remove_spelling(), which remote dbs support.
DEFINE_TESTCASE(spell0, spelling || remote) {
    Xapian::WritableDatabase db = get_writable_database();

    db.add_spelling("hello");
    db.add_spelling("cell", 2);
    db.commit();
    db.add_spelling("zig");
    db.add_spelling("ch");
    db.add_spelling("hello", 2);
    db.remove_spelling("hello", 2);
    db.remove_spelling("cell", 6);
    db.commit();
    db.remove_spelling("hello");
    db.remove_spelling("nonsuch");
    db.remove_spelling("zzzzzzzzz", 1000000);
    db.remove_spelling("aarvark");
    db.remove_spelling("hello");
    db.commit();
    db.remove_spelling("hello");

    return true;
}

// Test basic spelling correction features.
DEFINE_TESTCASE(spell1, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that the more frequent term is chosen.
    db.add_spelling("hello");
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "hello");
    db.add_spelling("cell", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    db.commit();
    Xapian::Database dbr(get_writable_database_as_database());
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    TEST_EQUAL(dbr.get_spelling_suggestion("hell"), "cell");

    // Check suggestions for single edit errors to "zig".
    db.add_spelling("zig");
    // Transpositions:
    TEST_EQUAL(db.get_spelling_suggestion("izg"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zgi"), "zig");
    // Substitutions:
    TEST_EQUAL(db.get_spelling_suggestion("sig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zog"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zif"), "zig");
    // Deletions:
    TEST_EQUAL(db.get_spelling_suggestion("ig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zg"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zi"), "zig");
    // Insertions:
    TEST_EQUAL(db.get_spelling_suggestion("azig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("zaig"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("ziag"), "zig");
    TEST_EQUAL(db.get_spelling_suggestion("ziga"), "zig");

    // Check suggestions for single edit errors to "ch".
    db.add_spelling("ch");
    // Transpositions:
    TEST_EQUAL(db.get_spelling_suggestion("hc"), "ch");
    // Substitutions - we don't handle these for two character words:
    TEST_EQUAL(db.get_spelling_suggestion("qh"), "");
    TEST_EQUAL(db.get_spelling_suggestion("cq"), "");
    // Deletions would leave a single character, and we don't handle those.
    TEST_EQUAL(db.get_spelling_suggestion("c"), "");
    TEST_EQUAL(db.get_spelling_suggestion("h"), "");
    // Insertions:
    TEST_EQUAL(db.get_spelling_suggestion("qch"), "ch");
    TEST_EQUAL(db.get_spelling_suggestion("cqh"), "ch");
    TEST_EQUAL(db.get_spelling_suggestion("chq"), "ch");

    // Check assorted cases:
    TEST_EQUAL(db.get_spelling_suggestion("shello"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("hellot"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("acell"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("cella"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("acella"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("helo"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("cll"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("helol"), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("clel"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("ecll"), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("cll"), "cell");

    // Check that edit distance 3 isn't found by default:
    TEST_EQUAL(db.get_spelling_suggestion("shelolx"), "");
    TEST_EQUAL(db.get_spelling_suggestion("celling"), "");
    TEST_EQUAL(db.get_spelling_suggestion("dellin"), "");

    // Check that edit distance 3 is found if specified:
    TEST_EQUAL(db.get_spelling_suggestion("shelolx", 3), "hello");
    TEST_EQUAL(db.get_spelling_suggestion("celling", 3), "cell");
    TEST_EQUAL(db.get_spelling_suggestion("dellin", 3), "cell");

    // Make "hello" more frequent than "cell" (3 vs 2).
    db.add_spelling("hello", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "hello");
    db.commit();
    TEST_EQUAL(db.get_spelling_suggestion("cello"), "hello");
    db.remove_spelling("hello", 2);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "cell");
    // Test "over-removing".
    db.remove_spelling("cell", 6);
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "hello");
    db.commit();
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "hello");
    db.remove_spelling("hello");
    TEST_EQUAL(db.get_spelling_suggestion("cell"), "");

    // Test removing words not in the table.
    db.remove_spelling("nonsuch");
    db.remove_spelling("zzzzzzzzz", 1000000);
    db.remove_spelling("aarvark");

    // Try removing word which was present but no longer is.
    db.remove_spelling("hello");
    db.commit();
    db.remove_spelling("hello");

    return true;
}

// Test spelling correction for Unicode.
DEFINE_TESTCASE(spell2, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that a UTF-8 sequence counts as a single character.
    db.add_spelling("h\xc3\xb6hle");
    db.add_spelling("ascii");
    TEST_EQUAL(db.get_spelling_suggestion("hohle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("hhle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("\xf0\xa8\xa8\x8f\xc3\xb6le", 2), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("hh\xc3\xb6l"), "h\xc3\xb6hle");
    TEST_EQUAL(db.get_spelling_suggestion("as\xc3\xb6\xc3\xb7i"), "ascii");
    TEST_EQUAL(db.get_spelling_suggestion("asc\xc3\xb6i\xc3\xb7i"), "ascii");
    db.commit();
    Xapian::Database dbr(get_writable_database_as_database());
    TEST_EQUAL(dbr.get_spelling_suggestion("hohle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("hhle", 1), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("\xf0\xa8\xa8\x8f\xc3\xb6le", 2), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("hh\xc3\xb6l"), "h\xc3\xb6hle");
    TEST_EQUAL(dbr.get_spelling_suggestion("as\xc3\xb6\xc3\xb7i"), "ascii");
    TEST_EQUAL(dbr.get_spelling_suggestion("asc\xc3\xb6i\xc3\xb7i"), "ascii");

    return true;
}

// Test spelling correction with multi databases
DEFINE_TESTCASE(spell3, spelling) {
    Xapian::WritableDatabase db1 = get_writable_database();
    // We can't just call get_writable_database() since it would delete db1
    // which doesn't work at all under __WIN32__ and will go wrong elsewhere if
    // changes to db1 are committed.
    Xapian::WritableDatabase db2 = get_named_writable_database("spell3", "");

    db1.add_spelling("hello");
    db1.add_spelling("cell", 2);
    db2.add_spelling("hello", 2);
    db2.add_spelling("helo");

    Xapian::Database db;
    db.add_database(db1);
    db.add_database(db2);

    TEST_EQUAL(db.get_spelling_suggestion("hello"), "");
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "hello");
    TEST_EQUAL(db1.get_spelling_suggestion("hell"), "cell");
    TEST_EQUAL(db2.get_spelling_suggestion("hell"), "hello");


    // Test spelling iterator
    Xapian::TermIterator i(db1.spellings_begin());
    TEST_EQUAL(*i, "cell");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db1.spellings_end());

    i = db2.spellings_begin();
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db2.spellings_end());

    i = db.spellings_begin();
    TEST_EQUAL(*i, "cell");
    TEST_EQUAL(i.get_termfreq(), 2);
    ++i;
    TEST_EQUAL(*i, "hello");
    TEST_EQUAL(i.get_termfreq(), 3);
    ++i;
    TEST_EQUAL(*i, "helo");
    TEST_EQUAL(i.get_termfreq(), 1);
    ++i;
    TEST(i == db.spellings_end());

    return true;
}

// Regression test - check that appending works correctly.
DEFINE_TESTCASE(spell4, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    db.add_spelling("check");
    db.add_spelling("pecks", 2);
    db.commit();
    db.add_spelling("becky");
    db.commit();

    TEST_EQUAL(db.get_spelling_suggestion("jeck", 2), "pecks");

    return true;
}

// Regression test - used to segfault with some input values.
DEFINE_TESTCASE(spell5, spelling) {
    const char * target = "\xe4\xb8\x80\xe4\xba\x9b";

    Xapian::WritableDatabase db = get_writable_database();
    db.add_spelling(target);
    db.commit();

    string s = db.get_spelling_suggestion("\xe4\xb8\x8d", 3);
    TEST_EQUAL(s, target);

    return true;
}

// Test basic spelling correction features.
DEFINE_TESTCASE(spell6, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that the more frequent term is chosen.
    db.add_spelling("hello", 2);
    db.add_spelling("sell", 3);
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "sell");
    db.commit();
    Xapian::Database dbr(get_writable_database_as_database());
    TEST_EQUAL(db.get_spelling_suggestion("hell"), "sell");
    TEST_EQUAL(dbr.get_spelling_suggestion("hell"), "sell");

    return true;
}

// Test suggestions when there's an exact match.
DEFINE_TESTCASE(spell7, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    // Check that the more frequent term is chosen.
    db.add_spelling("word", 57);
    db.add_spelling("wrod", 3);
    db.add_spelling("sword", 56);
    db.add_spelling("words", 57);
    db.add_spelling("ward", 58);
    db.commit();
    TEST_EQUAL(db.get_spelling_suggestion("ward"), "");
    TEST_EQUAL(db.get_spelling_suggestion("words"), "word");
    TEST_EQUAL(db.get_spelling_suggestion("sword"), "word");
    TEST_EQUAL(db.get_spelling_suggestion("wrod"), "word");

    return true;
}

vector<string> split_string(const string& s);

vector<string> split_string(const string& s)
{
    istringstream is(s);

    vector<string> result;
    string temp;
    while (getline(is, temp, ' '))
	result.push_back(temp);

    return result;
}

string merge_strings(const vector<string>& ss);

string merge_strings(const vector<string>& ss)
{
    string result;
    for (unsigned i = 0; i < ss.size(); ++i) {
	if (i > 0) result.push_back(' ');
	result.append(ss[i]);
    }
    return result;
}

//Test suggestions for word sequences and multiple suggestions.
DEFINE_TESTCASE(spell8, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    vector<string> source = split_string("the quick brown fox jumps over the lazy dog");

    db.add_spelling(source[0]);
    for (unsigned i = 1; i < source.size(); ++i) {
	db.add_spelling(source[i]);
	db.add_spelling(source[i - 1], source[i]);
    }

    db.commit();

    vector<string> request = split_string("the unknown brown quik fors jum psover the lazy fogx");
    vector<string> max_correct = split_string("the unknown brown quick fox jumps over the lazy dog");
    vector<string> result = db.get_spelling_suggestion(request);
    TEST_EQUAL(merge_strings(result), merge_strings(max_correct));

    return true;
}

//Test prefixed spelling.
DEFINE_TESTCASE(spell9, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    //Prefix with common group (for unprefixed terms)
    db.enable_spelling("prefix", "");
    //Prefix with its own group
    db.enable_spelling("prefix1", "prefix1");
    //Prefix with spelling group united with prefix1
    db.enable_spelling("prefix2", "prefix1");
    db.enable_spelling("prefix3", "prefix3");

    db.add_spelling("unprefixedword", 1, "");
    db.add_spelling("wordwithprefixdefault", 1, "prefix");
    db.add_spelling("wordwithprefixfirst", 1, "prefix1");
    db.add_spelling("wordwithprefixsecond", 1, "prefix2");
    db.add_spelling("wordwithprefixthird", 1, "prefix3");

    db.commit();

    TEST_EQUAL(db.get_spelling_suggestion("unprefixedwordz", ""), "unprefixedword");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixdefaultZ", ""), "wordwithprefixdefault");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixdefaultZ", "prefix"), "wordwithprefixdefault");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixfirstZ", ""), "");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixfirstZ", "prefix1"), "wordwithprefixfirst");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixfirstZ", "prefix2"), "wordwithprefixfirst");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixsecondZ", "prefix1"), "wordwithprefixsecond");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixsecondZ", "prefix3"), "");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixsecondZ", ""), "");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixthirdZ", "prefix2"), "");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixthirdZ", "prefix3"), "wordwithprefixthird");

    db.disable_spelling("prefix");
    TEST_EQUAL(db.get_spelling_suggestion("unprefixedwordZ", "prefix"), "");

    db.disable_spelling("prefix1");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixfirstZ", "prefix1"), "");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixfirstZ", "prefix2"), "wordwithprefixfirst");

    db.disable_spelling("prefix2");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixsecondZ", "prefix2"), "");

    db.disable_spelling("prefix3");
    TEST_EQUAL(db.get_spelling_suggestion("wordwithprefixthirdZ", "prefix3"), "");
    return true;
}

/// Regression test - repeated trigrams cancelled in 1.2.5 and earlier.
DEFINE_TESTCASE(spell10, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    // kin and kin used to cancel out in "skinking".
    db.add_spelling("skinking", 2);
    db.add_spelling("stinking", 1);
    db.commit();
    TEST_EQUAL(db.get_spelling_suggestion("scimking", 2), "skinking");
    return true;
}

// Multiple spelling suggestions test
DEFINE_TESTCASE(spell11, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    db.add_spelling("ducking");
    db.add_spelling("duck");
    db.add_spelling("the");
    db.add_spelling("ingthe");
    db.add_spelling("docking", 2);

    vector<string> words = split_string("ducking the ducking the");

    vector<vector<string> > result = db.get_spelling_suggestions(words, 2);
    //Test that the first variant is the most frequent one
    TEST_EQUAL(merge_strings(result[0]), "docking the docking the");
    //Test that the second variant is distanced from the first one.
    TEST_EQUAL(merge_strings(result[1]), "duck ingthe duck ingthe");

    return true;
}

// Transliteration and keyboard layouts test
DEFINE_TESTCASE(spell12, spelling) {
    Xapian::WritableDatabase db = get_writable_database();

    //Russain transliteration
    db.add_spelling("privet");
    db.add_spelling("how");
    db.add_spelling("are");
    db.add_spelling("ti");
    db.add_spelling("поживаешь");

    vector<string> words = split_string("привет how are ты pozhivaesh'");
    vector<string> result = db.get_spelling_suggestion(words, string(), "russian");
    TEST_EQUAL(merge_strings(result), "privet how are ti поживаешь");

    //Japanese transliteration
    db.add_spelling("mitsubishi");
    db.add_spelling("otsukaresan");

    words = split_string("みつびし お疲れさん");
    result = db.get_spelling_suggestion(words, string(), "japanese");
    TEST_EQUAL(merge_strings(result), "mitsubishi otsukaresan");

    //Chinese transliteration
    db.add_spelling("pinyin");
    db.add_spelling("hanzi");
    db.add_spelling("guanhua");

    words = split_string("拼音 漢字 汉字 官話");
    result = db.get_spelling_suggestion(words, string(), "chinese");
    TEST_EQUAL(merge_strings(result), "pinyin hanzi hanzi guanhua");

    //Arabic transliteration
    db.add_spelling("alti");
    db.add_spelling("aystti");
    db.add_spelling("aljmi");

    words = split_string("التي يستطيع الجميع");
    result = db.get_spelling_suggestion(words, string(), "arabic");
    TEST_EQUAL(merge_strings(result), "alti aystti aljmi");

    //Russian keyboard layout
    db.add_spelling("екатеринбург");
    db.add_spelling("is");
    db.add_spelling("depressing");

    words = split_string("trfnthby,ehu is вузкуыыштп");
    result = db.get_spelling_suggestion(words, string(), "russian");
    TEST_EQUAL(merge_strings(result), "екатеринбург is depressing");

    //French keyboard layout
    db.add_spelling("madeleine");
    db.add_spelling("ou");
    db.add_spelling("wilkinson");

    words = split_string(";qdeleine ou zilkinson");
    result = db.get_spelling_suggestion(words, string(), "french");
    TEST_EQUAL(merge_strings(result), "madeleine ou wilkinson");

    return true;
}

//Phonetic tests
DEFINE_TESTCASE(spell13, spelling) {
    Xapian::SpellingPhonetic phonetic("english");
    TEST_EQUAL(phonetic.get_phonetic("gammon"), phonetic.get_phonetic("gamin"));

    Xapian::SpellingPhonetic russian_phonetic("russian");
    TEST_EQUAL(russian_phonetic.get_phonetic("шварцнегер"),
               russian_phonetic.get_phonetic("швэртснегир"));

    return true;
}

//LanguageAutodetect tests
DEFINE_TESTCASE(spell14, spelling) {
    Xapian::LanguageAutodetect lang;

    TEST_EQUAL(lang.get_language("london is the capital of great britain"), "english");
    TEST_EQUAL(lang.get_language("мама мыла раму"), "russian");
    TEST_EQUAL(lang.get_language("みつびし お疲れさん"), "japanese");
    TEST_EQUAL(lang.get_language("拼音 漢字 汉字 官話"), "chinese");
    TEST_EQUAL(lang.get_language("ek het nie geweet dat hy sou kom nie."), "afrikaans");
    TEST_EQUAL(lang.get_language("мова корінного населення України"), "ukrainian");
    TEST_EQUAL(lang.get_language("што тутака зьявілася новага Беларус газ"), "belarus");
    TEST_EQUAL(lang.get_language("constituido en estado social y democrático de derecho"), "spanish");
    TEST_EQUAL(lang.get_language("mony writers nou evites apostrophes whaur thay're thocht tae shaw letters fae"), "scots");
    TEST_EQUAL(lang.get_language("الموسوعة الحرة التي يستطيع الجميع"), "arabic");
    TEST_EQUAL(lang.get_language("това означава, че те са свободни и винаги ще бъдат такива"), "bulgarian");
    TEST_EQUAL(lang.get_language("ויקיפדיה היא מיזם רב לשוני לחיבור אנציקלופדיה"), "hebrew");
    TEST_EQUAL(lang.get_language("w ostatnich latach, w związku z utworzeniem"), "polish");
    TEST_EQUAL(lang.get_language("l'espressione italiana più corretta per questa lingua è"), "italian");
    TEST_EQUAL(lang.get_language("am besten haben sich die traditionellen, das heißt ein"), "german");
    TEST_EQUAL(lang.get_language("en raison de différences existant entre les dialectes du scots et de la"), "french");
    TEST_EQUAL(lang.get_language("foneettisesti suomen kieli on melko yksinkertainen"), "finnish");
    TEST_EQUAL(lang.get_language("võrdlevgrammatiliste uurimuste kohaselt on eesti keel maailma"), "estonian");
    TEST_EQUAL(lang.get_language("taalvormen die sterk verwant zijn aan het Standaardnederlands zijn"), "dutch");
    TEST_EQUAL(lang.get_language("euskararen jatorria ezezaguna da. Gaur egun, euskararen hizkuntzalaritza"), "basque");
    TEST_EQUAL(lang.get_language("bošnjaci su izvorni govornici nekoliko dijalekata štokavskog narječja"), "bosnian");
    TEST_EQUAL(lang.get_language("Hrvatski jezik obuhvaća standardni, odnosno književni ili opći hrvatski"), "croatian");
    TEST_EQUAL(lang.get_language("Čeština je blízká a vzájemně srozumitelná se slovenštinou"), "czech");

    return true;
}
