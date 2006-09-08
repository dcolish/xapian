/* values.h: constants and functions for document value handling.
 *
 * Copyright (C) 2006 Olly Betts
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

#include <string>

// Include these to get uint32_t.
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

enum value_slot {
    VALUE_LASTMOD = 0,	// 4 byte big endian value - seconds since 1970.
    VALUE_MD5 = 1	// 16 byte MD5 checksum of original document.
};

inline uint32_t binary_string_to_int(const std::string &s)
{
    if (s.size() != 4) return (uint32_t)-1;
    uint32_t v;
    memcpy(&v, s.data(), 4);
    return ntohl(v);
}

inline std::string int_to_binary_string(uint32_t v)
{
    v = htonl(v);
    return std::string(reinterpret_cast<const char*>(&v), 4);
}
