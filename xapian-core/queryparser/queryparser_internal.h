/* queryparser_internal.h: The non-lemon-generated parts of the QueryParser
 * class.
 *
 * Copyright (C) 2005,2006,2007,2010 Olly Betts
 * Copyright (C) 2011 Nikita Smetanin
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
#define XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H

#include <xapian/base.h>
#include <xapian/database.h>
#include <xapian/query.h>
#include <xapian/queryparser.h>
#include <xapian/stem.h>
#include "../spelling/spelling_phonetic.h"

#include <list>
#include <map>
#include <set>
#include <vector>

using namespace std;

class State;

typedef enum { NON_BOOLEAN, BOOLEAN, BOOLEAN_EXCLUSIVE } filter_type;

/** Information about how to handle a prefix in the query string. */
struct PrefixInfo {
    /// The type of this prefix.
    filter_type type;

    /// Prefix strings.
    list<string> prefixes;

    PrefixInfo(filter_type type_, const string & prefix)
	: type(type_)
    {
	prefixes.push_back(prefix);
    }
};

namespace Xapian {

class Utf8Iterator;

class QueryParser::Internal : public Xapian::Internal::RefCntBase {
    friend class QueryParser;
    friend class ::State;
    Stem stemmer;
    stem_strategy stem_action;
    const Stopper * stopper;
    Query::op default_op;
    const char * errmsg;
    Database db;
    list<string> stoplist;
    multimap<string, string> unstem;

    SpellingPhonetic phonetic;
    set<string> phonetic_prefixes;
    map<string, string> language_prefixes;

    // Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
    // "foobar" -> "XFOO". FIXME: it does more than this now!
    map<string, PrefixInfo> prefixmap;

    list<ValueRangeProcessor *> valrangeprocs;

    string corrected_query;

    void add_prefix(const string &field, const string &prefix,
		    filter_type type);

    std::string parse_term(Utf8Iterator &it, const Utf8Iterator &end,
			   bool &was_acronym);

    void apply_spelling(State* state, std::vector<std::string>& prefixes,
                        std::vector<std::pair<size_t, size_t> >& terms,
                        std::string& correction_query,
                        int& correction_offset,
                        const std::string& query) const;

  public:
    Internal() : stem_action(STEM_NONE), stopper(NULL),
	default_op(Query::OP_OR), errmsg(NULL) { }
    Query parse_query(const string & query_string, unsigned int flags, const string & default_prefix);
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
