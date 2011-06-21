/** @file pretty.h
 * @brief Convert types to pretty representations
 */
/* Copyright (C) 2010,2011 Olly Betts
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

#ifndef XAPIAN_INCLUDED_PRETTY_H
#define XAPIAN_INCLUDED_PRETTY_H

#include <list>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "xapian/intrusive_ptr.h"
#include "xapian/types.h"

namespace Xapian {
namespace Internal {

template<class S>
struct PrettyOStream {
    /// The std::ostream object we're outputting to.
    S & os;

    PrettyOStream(S & os_) : os(os_) { }
};

struct Literal {
    const char * _lit;
    Literal(const char * lit) : _lit(lit) { }
    Literal(const std::string & s) : _lit(s.c_str()) { }
};

}
}

using Xapian::Internal::PrettyOStream;
using Xapian::Internal::Literal;

/// Default is to output as std::ostream would.
template<class S, class T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const T & t)
{
    ps.os << t;
    return ps;
}

/** Allow writing literal strings.
 *
 *  For example:
 *
 *  PrettyOStream<std::ostream> ps(std::cout);
 *  ps << Literal("x = ") << x << Literal(", y = ") << y << endl;
 */
template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Literal & t)
{
    ps.os << t._lit;
    return ps;
}

template<class S, class T *>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const T * t)
{
    ps.os << '&';
    return ps << *t;
}

template<class S, class T **>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const T ** t)
{
    ps.os << (void*)t;
    return ps;
}

// FIXME: We probably don't want to inline this, but need to arrange to
// put it somewhere sane out-of-line.
inline void write_ch(std::ostream & os, unsigned char ch)
{
    if (ch < 32 || ch >= 127) {
	os << '\\';
	if (ch >= 7 && ch <= 13) {
	    os << "abtnvfr"[ch - 7];
	} else {
	    os << char('0' | (ch >> 6));
	    os << char('0' | ((ch >> 3) & 7));
	    os << char('0' | (ch & 7));
	}
    } else if (ch == '\\') {
	os << "\\\\";
    } else if (ch == '"') {
	os << "\\\"";
    } else {
	os << ch;
    }
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const char * str)
{
    ps.os << '"';
    while (*str) {
	write_ch(ps.os, *str++);
    }
    ps.os << '"';
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::string & str)
{
    ps.os << '"';
    for (std::string::const_iterator i = str.begin(); i != str.end(); ++i) {
	write_ch(ps.os, *i);
    }
    ps.os << '"';
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::string &)
{
    ps.os << "std::string&";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::string *)
{
    ps.os << "std::string*";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, unsigned char ch)
{
    ps.os << '\'';
    if (ch < 32 || ch >= 127) {
	ps.os << '\\';
	if (ch >= 7 && ch <= 13) {
	    ps.os << "abtnvfr"[ch - 7];
	} else if (ch == '\0') {
	    ps.os << "\\0";
	} else {
	    ps.os << "0123456789abcdef"[ch >> 4];
	    ps.os << "0123456789abcdef"[ch & 0x0f];
	}
    } else if (ch == '\\') {
	ps.os << "\\\\";
    } else if (ch == '\'') {
	ps.os << "\\'";
    } else {
	ps.os << ch;
    }
    ps.os << '\'';
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, bool b)
{
    ps.os << (b ? "true" : "false");
    return ps;
}

/*
template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, bool &)
{
    ps.os << "bool&";
    return ps;
}
*/

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, Xapian::termcount * p)
{
    ps.os << "(Xapian::termcount*)" << (void*)p;
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::list<T> &) {
    ps.os << "std::list&";
    return ps;
}

template<class S, typename T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::list<T> &) {
    ps.os << "std::list";
    // FIXME: could show first up to N elements.
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::map<K, V> *) {
    ps.os << "std::map*";
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::map<K, V> &) {
    ps.os << "std::map&";
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::map<K, V> & m) {
    ps.os << "std::map(" << m.size() << ')';
    // FIXME: could show first up to N elements.
    return ps;
}

template<class S, typename T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::vector<T> & v) {
    ps.os << "std::vector(" << v.size() << ')';
    // FIXME: could show first up to N elements.
    return ps;
}

namespace Xapian {
    class Registry;
    class Weight;
    namespace Internal {
	class ExpandStats;
	class ExpandWeight;
    }
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Registry &) {
    ps.os << "Xapian:Registry";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Weight &) {
    ps.os << "Xapian:Weight";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Weight *p) {
    ps.os << "(Xapian:Weight*)" << (void*)p;
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Internal::ExpandStats &) {
    ps.os << "Xapian:Internal::ExpandStats";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Internal::ExpandWeight &) {
    ps.os << "Xapian:Internal::ExpandWeight";
    return ps;
}

class RemoteConnection;

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const RemoteConnection &) {
    ps.os << "RemoteConnection";
    return ps;
}

class BrassCursor;

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const BrassCursor *) {
    ps.os << "BrassCursor*";
    return ps;
}

class ChertCursor;

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const ChertCursor *) {
    ps.os << "ChertCursor*";
    return ps;
}

template<class S, class T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, Xapian::Internal::intrusive_ptr<const T> t) {
    ps.os << "RefCntPtr->";
    return ps << t;
}

template<class S, typename T>
inline PrettyOStream<S> &
operator|(PrettyOStream<S> &ps, const T & t) {
    ps.os << ", ";
    return ps << t;
}

#endif // XAPIAN_INCLUDED_PRETTY_H
