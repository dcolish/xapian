/* queryparser_internal.h: The non-lemon-generated parts of the QueryParser
 * class.
 *
 * Copyright (C) 2005,2006,2007 Olly Betts
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

#include <list>
#include <map>

using namespace std;

class State;

struct PrefixInfo {
    typedef enum {
	FREE_TEXT,
	BOOL_FILTER
    } prefix_type;
    PrefixInfo::prefix_type type;
    string str;
    PrefixInfo(PrefixInfo::prefix_type t, const string &s) : type(t), str(s) { }
};

class Xapian::QueryParser::Internal : public Xapian::Internal::RefCntBase {
    friend class Xapian::QueryParser;
    friend class ::State;
    Xapian::Stem stemmer;
    stem_strategy stem_action;
    const Xapian::Stopper * stopper;
    Xapian::Query::op default_op;
    const char * errmsg;
    Xapian::Database db;
    list<string> stoplist;
    multimap<string, string> unstem;

    // Map "from" -> "A" ; "subject" -> "C" ; "newsgroups" -> "G" ;
    // "foobar" -> "XFOO"
    map<string, PrefixInfo> prefixes;

    list<Xapian::ValueRangeProcessor *> valrangeprocs;

  public:
    Internal() : stem_action(STEM_NONE), stopper(NULL),
	default_op(Xapian::Query::OP_OR), errmsg(NULL) { }
    Xapian::Query parse_query(const string & query_string, unsigned int flags, const string & default_prefix);
};

#endif // XAPIAN_INCLUDED_QUERYPARSER_INTERNAL_H
