/* stem_norwegian.h: header for norwegian stemming algorithm.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
/* Version 1: see http://xapian.sourceforge.net/ for further information */

#ifndef OM_HGUARD_STEM_NORWEGIAN_H
#define OM_HGUARD_STEM_NORWEGIAN_H

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

struct norwegian_stemmer;

extern void * setup_norwegian_stemmer();

extern const char * norwegian_stem(void * z, const char * q, int i0, int i1);

extern void closedown_norwegian_stemmer(void * z);


/* To set up the norwegian_stemming process:

       void * z = setup_norwegian_stemmer();

   to use it:

       const char * p = norwegian_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_norwegian_stemmer(z);

*/
#ifdef __cplusplus
}
#endif

#endif /* OM_HGUARD_STEM_NORWEGIAN_H */
