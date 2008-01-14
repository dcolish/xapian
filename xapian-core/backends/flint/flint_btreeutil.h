/* btree_util.h: common macros/functions in the Btree implementation.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2004 Olly Betts
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

#ifndef OM_HGUARD_FLINT_BTREEUTIL_H
#define OM_HGUARD_FLINT_BTREEUTIL_H

#include "flint_types.h"
#include "omassert.h"

#include <string.h>  /* memset */

/* The unit of access into the DB files is an unsigned char, which is defined
   as 'byte' with a typedef.

   Other integer values are built up from these bytes, either in pairs or fours.
   'int2' and 'int4' are signed types which will comfortably accommodate
   all 2 byte and 4 byte ranges:
*/

/*
   If a signed int cannot hold the full range of two bytes replace 'int' by
   'long' throughout the code.

   FIXME: surely if a signed int cannot hold the full range of two bytes,
   then the compiler violates ANSI?  Or am I misunderstanding...
*/

// FIXME: 65536 in Asserts below should really be block_size
inline int
getint1(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536);
    return p[c];
}

inline void
setint1(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536);
    p[c] = x;
}

inline int
getint2(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536 - 1);
    return p[c] << 8 | p[c + 1];
}

inline void
setint2(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536 - 1);
    p[c] = x >> 8;
    p[c + 1] = x;
}

inline int
getint4(const byte *p, int c)
{
    Assert(c >= 0);
    Assert(c < 65536 - 3);
    return p[c] << 24 | p[c + 1] << 16 | p[c + 2] << 8 | p[c + 3];
}

inline void
setint4(byte *p, int c, int x)
{
    Assert(c >= 0);
    Assert(c < 65536 - 3);
    p[c] = x >> 24;
    p[c + 1] = x >> 16;
    p[c + 2] = x >> 8;
    p[c + 3] = x;
}

#endif /* OM_HGUARD_FLINT_BTREEUTIL_H */
