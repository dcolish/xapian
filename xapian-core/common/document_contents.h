/* document_contents.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_DOCUMENT_CONTENTS_H
#define OM_HGUARD_DOCUMENT_CONTENTS_H

#include <string>
#include <vector>
#include <map>
#include "om/omtypes.h"
#include "indexer.h"

/** A term in a document. */
struct DocumentTerm {
    /** Make a new term.
     *
     *  This creates a new term, and adds one posting at the specified
     *  position.
     *
     *  @param tname_ The name of the new term.
     *  @param tpos   Optional positional information.
     */
    DocumentTerm(const om_termname & tname_, om_termpos tpos = 0);

    /** The name of this term.
     */
    om_termname tname;

    /** Within document frequency of the term.
     *  This is the number of occurrences of the term in the document.
     */
    om_termcount wdf;

    /** Type to store positional information in. */
    typedef vector<om_termpos> term_positions;

    /** Positional information. 
     *
     *  This is a list of positions at which the term occurs in the
     *  document. The list is in strictly increasing order of term
     *  position. 
     *
     *  The positions start at 1. 
     *
     *  Note that, even if positional information is present, the WDF might
     *  not be equal to the length of the position list, since a term might
     *  occur multiple times at a single position, but will only have one
     *  entry in the position list for each position. 
     */
    term_positions positions;

    /** Add an entry to the posting list.
     *
     *  This method increments the wdf.  If positional information is
     *  supplied, this also adds an entry to the list of positions, unless
     *  there is already one for the specified position.
     *  
     *  @param tpos The position within the document at which the term
     *              occurs.  If this information is not available, use
     *              the default value of 0.
     */
    void add_posting(om_termpos tpos = 0);
};

/** The information which is stored in a document.
 *
 *  This object contains all the information associated with a document,
 *  and can be used to build up that information and then add it to a
 *  writable database.
 */
struct DocumentContents {
    /** The (user defined) data associated with this document. */
    string data;

    /** Type to store keys in. */
    typedef map<om_keyno, string> document_keys;

    /** The keys associated with this document. */
    document_keys keys;

    /** Type to store terms in. */
    typedef map<om_termname, DocumentTerm> document_terms;

    /** The terms (and their frequencies and positions) in this document. */
    document_terms terms;

    /** Add an occurrence of a term to the document.
     *
     *  Multiple occurrences of the term at the same position are represented
     *  only once in the positional information, but do increase the wdf.
     *
     *  @param tname  The name of the term.
     *  @param tpos   The position of the term.
     */
    void add_posting(const om_termname & tname, om_termpos tpos = 0);
};

#endif /* OM_HGUARD_DOCUMENT_CONTENTS_H */
