/* parsequery.ll: scanner for query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

%option noyywrap

%{
#include "omega.h"
#include "query.h"
//#include "omassert.h"

#define YY_DECL void parse_prob(const string &prob_query)

#define yyterminate() return

#define YY_NEVER_INTERACTIVE 1

// ECHO should never get called
#define ECHO abort()

static string query;
static size_t query_index = 0;

// FIXME: transliterate using get_next_char...
#define YY_INPUT(buf, result, max_size) \
    do { \
	if (query_index >= query.size()) { \
	    result = YY_NULL; \
	} else { \
            query.copy(buf, max_size, query_index); \
	    result = strlen(buf); \
	    query_index += result; \
	} \
    } while (0)

/**************************************************************/
/* transliterate accented characters in step with what the indexers do */
static int
get_next_char(const char **p)
{
    static int cache = 0;
    int ch;
    if (cache) {
	ch = cache;
	cache = 0;
	return ch;
    }
    ch = (int)(unsigned char)(*(*p)++);
    switch (ch) {
#include "symboltab.h"
    }
    return ch;
}

%}

%x INQUOTES AFTERTERM 

%%

%{
    query = prob_query;
    query_index = 0;
    vector<string> quoted_terms;
    OmStem stemmer("english");
    termtype type = NORMAL;
    int stem, stem_all;

    stem = !atoi(option["no_stem"].c_str());
    /* stem capitalised words too -- needed for EuroFerret */
    stem_all = atoi(option["all_stem"].c_str());
    // FIXME: allow domain:uk in query...
    // don't allow + and & in term then ?
    // or allow +&.-_ ?
    // domain/site/language/host ?
%}

<INITIAL>\+ {
    type = PLUS;
}

<INITIAL>- {
    type = MINUS;
}

<INITIAL>[^-+"A-Za-z0-9]+ {
    type = NORMAL;
}

<INITIAL>\" {
    BEGIN(INQUOTES);
}

<INITIAL>[A-Za-z0-9][A-Za-z0-9+&]*\.[ \t\r\n] {
    // FIXME: termname with trailing dot at end of query?
    // termname with trailing dot, but not something like "muscat.com"    
    string termname;
    yyless(yyleng - 1); // push back whitespace
    for (int i = 0; i < yyleng - 1; i++) termname += tolower(yytext[i]);

    check_term(termname, type);
    BEGIN(AFTERTERM);
}

<INITIAL>[A-Za-z0-9][A-Za-z0-9+&]* {
    string termname;
    for (int i = 0; i < yyleng; i++) termname += tolower(yytext[i]);

    if (stem && (stem_all || !isupper(yytext[0])))
        termname = stemmer.stem_word(termname);

    check_term(termname, type);
    BEGIN(AFTERTERM);
}

<AFTERTERM>- {
    // Treat "-" after a term as a hyphen, not a term negation.
    // Also index hyphenated words as multiple terms, so it makes most
    // sense to keep same "+"/"-" weighting for terms joined by hyphens
    BEGIN(INITIAL);
}

<AFTERTERM>[^-][^-+"A-Za-z0-9]* {
    type = NORMAL; // Reset any currently active "+" or "-"
    BEGIN(INITIAL);
}

<INQUOTES>[^"A-Za-z0-9]*\" {
    // end quoted term
    if (quoted_terms.size() == 1) {
	// FIXME: don't always stem...
	check_term(stemmer.stem_word(quoted_terms[0]), type);
    } else {
	// we only index word pairs, so add a series of pairs for
	// phrases of more than 2 words
	for (size_t i = 0; i + 1 < quoted_terms.size(); i++) {
	    string phrase = quoted_terms[i] + " " + quoted_terms[i + 1];
	    // FIXME: add these terms AND-ed
	    check_term(phrase, type);
	}
    }
    quoted_terms.clear();
    BEGIN(INITIAL);
}

<INQUOTES>[^"A-Za-z0-9]* {
    // skip punctuation, etc
}

<INQUOTES>[A-Za-z0-9][A-Za-z0-9+&]* {
    string termname;
    for (int i = 0; i < yyleng; i++) termname += tolower(yytext[i]);

    quoted_terms.push_back(termname);
}

%{
    // handle unterminated quotes
    // FIXME: find where to put this...
    if (quoted_terms.size() == 1) {
    	string termname = quoted_terms[0];
	if (stem) termname = stemmer.stem_word(termname);
	check_term(termname, type);
    } else {
	// FIXME: add phrase in quoted_terms
    }
    quoted_terms.clear();
%}
