/* omstem.cc: Builder for stemming algorithms
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include <string>

#include "omdebug.h"
#include "om/omoutput.h"
#include "om/omstem.h"
#include "utils.h"

#include "api.h"
#include "snowball_danish.h"
#include "snowball_dutch.h"
#include "snowball_english.h"
#include "snowball_french.h"
#include "snowball_finnish.h"
#include "snowball_german.h"
#include "snowball_italian.h"
#include "snowball_norwegian.h"
#include "snowball_portuguese.h"
#include "snowball_russian.h"
#include "snowball_spanish.h"
#include "snowball_swedish.h"
#include "snowball_lovins.h"
#include "snowball_porter.h"

using std::string;

////////////////////////////////////////////////////////////

/** The available languages for the stemming algorithms to use.
 *  If you change this, change stemmers[], language_names[],
 *  and language_strings[] also.
 */
enum stemmer_language {
    STEMLANG_INVALID,
    STEMLANG_NONE,
    STEMLANG_DANISH,
    STEMLANG_DUTCH,
    STEMLANG_ENGLISH,
    STEMLANG_FINNISH,
    STEMLANG_FRENCH,
    STEMLANG_GERMAN,
    STEMLANG_ITALIAN,
    STEMLANG_NORWEGIAN,
    STEMLANG_PORTUGUESE,
    STEMLANG_RUSSIAN,
    STEMLANG_SPANISH,
    STEMLANG_SWEDISH,
    STEMLANG_LOVINS,
    STEMLANG_PORTER
};

struct stemmer_obj {
    /// Function pointer to initialise stemmer
    struct SN_env * (* setup)();
    
    /// Function pointer to stem a word.
    int (* stem)(struct SN_env *);

    /// Function pointer to close down the stemmer.
    void (* closedown)(struct SN_env *);
};

#define P(L) snowball_##L
#define E(L) { P(L##_create_env), P(L##_stem), P(L##_close_env) }

/** Structs of function pointers.
 *  This list must be in the same order as enum stemmer_language.
 *  If you change this, change language_names[], language_strings[], and
 *  enum stemmer_language also.
 */
struct stemmer_obj stemmers[] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	E(danish),
	E(dutch),
	E(english),
	E(finnish),
	E(french),
	E(german),
	E(italian),
	E(norwegian),
	E(portuguese),
	E(russian),
	E(spanish),
	E(swedish),
	E(lovins),
	E(porter),
};

/** The names of the languages.
 *  This list must be in the same order as enum stemmer_language.
 *  If you change this, change stemmers[], language_strings[], and
 *  enum stemmer_language also.
 */
static const char * language_names[] = {
    "",
    "none",
    "danish",
    "dutch",
    "english",
    "finnish",
    "french",
    "german",
    "italian",
    "norwegian",
    "portuguese",
    "russian",
    "spanish",
    "swedish",
    "english_lovins",
    "english_porter"
};

/** The mapping from language strings to language codes.
 *  This list must be in alphabetic order.
 *  If you change this, change language_names[] and enum stemmer_language
 *  also.
 */
static const StringAndValue language_strings[] = {
    {"da",		STEMLANG_DANISH},
    {"danish",		STEMLANG_DANISH},
    {"de",		STEMLANG_GERMAN},
    {"dutch",		STEMLANG_DUTCH},
    {"en",		STEMLANG_ENGLISH},
    {"english",		STEMLANG_ENGLISH},
    {"english_lovins",	STEMLANG_LOVINS},
    {"english_porter",	STEMLANG_PORTER},
    {"es",		STEMLANG_SPANISH},
    {"fi",		STEMLANG_FINNISH},
    {"finnish",		STEMLANG_FINNISH},
    {"fr",		STEMLANG_FRENCH},
    {"french",		STEMLANG_FRENCH},
    {"german",		STEMLANG_GERMAN},
    {"it",		STEMLANG_ITALIAN},
    {"italian",		STEMLANG_ITALIAN},
    {"lovins",		STEMLANG_LOVINS},
    {"nl",		STEMLANG_DUTCH},
    {"no",		STEMLANG_NORWEGIAN},
    {"none",		STEMLANG_NONE},
    {"norwegian",	STEMLANG_NORWEGIAN},
    {"porter",		STEMLANG_PORTER},
    {"portuguese",	STEMLANG_PORTUGUESE},
    {"pt",		STEMLANG_PORTUGUESE},
    {"ru",		STEMLANG_RUSSIAN},
    {"russian",		STEMLANG_RUSSIAN},
    {"spanish",		STEMLANG_SPANISH},
    {"sv",		STEMLANG_SWEDISH},
    {"swedish",		STEMLANG_SWEDISH},
    {"",		STEMLANG_INVALID}
};


////////////////////////////////////////////////////////////
// OmStem::Internal class

class OmStem::Internal {
    private:
        // Prevent copying
        Internal(const OmStem::Internal &);
        Internal & operator=(const OmStem::Internal &);

    public:
	/** Initialise the state based on the specified language name.
	 */
	Internal(const string &language);

	/** Initialise the state based on the specified language code.
	 */
	Internal(enum stemmer_language langcode_);

	/** Destructor.
	 */
	~Internal();

	/** The code representing the language being stemmed.
	 */
	enum stemmer_language langcode;

	/** Stem the given word.
	 */
	string stem_word(const string &word) const;

    private:
	/** Data used by the stemming algorithm.
	 */
	struct SN_env * stemmer_data;

	/** Set the stemming language.
	 */
	void set_language(stemmer_language langcode_);

	/** Return a stemmer_language enum value from a language
	 *  string.
	 */
	stemmer_language get_stemtype(const string &language);
};

OmStem::Internal::Internal(const string &language)
	: stemmer_data(0)
{
    stemmer_language langcode_ = get_stemtype(language);
    if (langcode_ == STEMLANG_INVALID) {
        // FIXME: use a separate InvalidLanguage exception?
        throw OmInvalidArgumentError("Unknown language `" +
				     language + "' specified");
    }
    set_language(langcode_);
}

OmStem::Internal::Internal(enum stemmer_language langcode_)
	: stemmer_data(0)
{
    Assert(langcode_ != STEMLANG_INVALID);
    set_language(langcode_);
}

OmStem::Internal::~Internal()
{
    if (stemmer_data != 0) {
	stemmers[langcode].closedown(stemmer_data);
    }
}

void
OmStem::Internal::set_language(stemmer_language langcode_)
{
    Assert(langcode_ != STEMLANG_INVALID); 
    if (stemmer_data != 0) {
	stemmers[langcode].closedown(stemmer_data);
    }
    langcode = langcode_;
    stemmer_data = stemmers[langcode].setup ? stemmers[langcode].setup() : 0;
}

stemmer_language
OmStem::Internal::get_stemtype(const string &language)
{
    return static_cast<stemmer_language> (
		map_string_to_value(language_strings, language));
}

string
OmStem::Internal::stem_word(const string &word) const
{
    if (!stemmer_data || word.empty()) return word;
    SN_set_current(stemmer_data, word.length(),
		   (const unsigned char *)(word.data()));
    // FIXME should we look at the return value of the stem function?
    stemmers[langcode].stem(stemmer_data);
    return string((const char *)(stemmer_data->p), stemmer_data->l);
}

///////////////////////
// Methods of OmStem //
///////////////////////

OmStem::OmStem(const string &language)
	: internal(new OmStem::Internal(language))
{
    DEBUGAPICALL(void, "OmStem::OmStem", language);
}

OmStem::OmStem() : internal(new OmStem::Internal(STEMLANG_NONE))
{
    DEBUGAPICALL(void, "OmStem::OmStem", "");
}

OmStem::~OmStem()
{
    DEBUGAPICALL(void, "OmStem::~OmStem", "");
    delete internal;
}

OmStem::OmStem(const OmStem &other)
{
    DEBUGAPICALL(void, "OmStem::OmStem", other);
    internal = new OmStem::Internal(other.internal->langcode);
}

void
OmStem::operator=(const OmStem &other)
{
    DEBUGAPICALL(void, "OmStem::operator=", other);
    delete internal;
    internal = new OmStem::Internal(other.internal->langcode);
}

string
OmStem::stem_word(const string &word) const
{
    DEBUGAPICALL(string, "OmStem::stem_word", word);
    RETURN(internal->stem_word(word));
}

string
OmStem::get_available_languages()
{
    DEBUGAPICALL_STATIC(string, "OmStem::get_available_languages", "");
    string languages;

    const char ** pos;
    for (pos = language_names + 1;
	 pos != language_names + (sizeof(language_names) / sizeof(char *));
	 pos++) {
	if (!languages.empty()) languages += ' ';
	languages += *pos;
    }
    RETURN(languages);
}

string
OmStem::get_description() const
{
    DEBUGCALL(INTRO, string, "OmStem::get_description", "");
    RETURN("OmStem(" + string(language_names[internal->langcode]) + ")");
}
