/* queryparser.h: build a Xapian::Query object from a user query string.
 *
 * Copyright (C) 2005 Olly Betts
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
 */

#ifndef XAPIAN_INCLUDED_QUERYPARSER_H
#define XAPIAN_INCLUDED_QUERYPARSER_H

#include <xapian.h>

#include <string>

namespace Xapian {

/// Base class for stop-word decision functor.
class Stopper {
  public:
    /// Is term a stop-word?
    virtual bool operator()(const std::string & term) const = 0;

    /// Class has virtual methods, so provide a virtual destructor.
    virtual ~Stopper() { }
};

/// Build a Xapian::Query object from a user query string.
class QueryParser {
  public:
    /// Class representing the queryparser internals.
    class Internal;
    /// @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Enum of feature flags.
    typedef enum {
	FLAG_BOOLEAN = 1,
	FLAG_PHRASE = 2,
	FLAG_LOVEHATE = 4
    } feature_flag;

    typedef enum { STEM_NONE, STEM_SOME, STEM_ALL } stem_strategy;

    /// Copy constructor.
    QueryParser(const QueryParser & o);

    /// Assignment.
    QueryParser & operator=(const QueryParser & o);

    /// Default constructor.
    QueryParser();

    /// Destructor.
    ~QueryParser();

    /// Set the stemmer.
    void set_stemmer(const Xapian::Stem & stemmer);

    /// Set the stemming options.
    void set_stemming_options(stem_strategy strategy);

    void set_stopper(Stopper *stop = NULL);

    /** Set the default boolean operator. */
    void set_default_op(Query::op default_op);

    /** Get the default boolean operator. */
    Query::op get_default_op() const;

    /// Specify the database being searched.
//    void set_database(const Database &db);

    /// Parse a query.
    Query parse_query(const std::string &query_string);

    void add_prefix(const std::string &field, const std::string &prefix);

    void add_boolean_prefix(const std::string & field, const std::string &prefix);

    TermIterator termlist_begin() const;
    TermIterator termlist_end() const;

    TermIterator stoplist_begin() const;
    TermIterator stoplist_end() const;

    TermIterator unstem_begin(const std::string &term) const;
    TermIterator unstem_end(const std::string &term) const;
};

}

#endif // XAPIAN_INCLUDED_QUERYPARSER_H
