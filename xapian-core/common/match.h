/* match.h: base class for matchers
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

#ifndef OM_HGUARD_MATCH_H
#define OM_HGUARD_MATCH_H

#include "database.h"
#include "om/omdocument.h"
#include "om/omenquire.h"
#include "om/omsettings.h"
#include "omqueryinternal.h"
#include "match.h"
#include "stats.h"
#include "rset.h"
#include "irweight.h"
#include "refcnt.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////
// Comparison functions to determine the order of elements in the MSet
// Return true if a should be listed before b

typedef bool (* mset_cmp)(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_forward(const OmMSetItem &, const OmMSetItem &);
bool msetcmp_reverse(const OmMSetItem &, const OmMSetItem &);

/// Compare an OmMSetItem, using a custom function
class OmMSetCmp {
    public:
	bool (* fn)(const OmMSetItem &a, const OmMSetItem &b);
	OmMSetCmp(bool (* fn_)(const OmMSetItem &a, const OmMSetItem &b))
		: fn(fn_) {}
	bool operator()(const OmMSetItem &a, const OmMSetItem &b) const {
	    return fn(a, b);
	}
};

class SubMatch : public RefCntBase {
    public:
	virtual ~SubMatch() { }

	/** Prepare to perform the match operation.
	 *  This must be called with a return value of true before
	 *  get_postlist().  It can be called more
	 *  than once.  If nowait is true, the operation has only succeeded
	 *  when the return value is true.
	 *
	 *  @param nowait	If true, then return as soon as
	 *  			possible even if the operation hasn't
	 *  			been completed.  If it hasn't, then
	 *  			the return value will be false.  The
	 *  			caller should retry until prepare_match
	 *  			returns true, or throws an exception to
	 *  			indicate an error.
	 *
	 *  @return  If nowait is true, and the match is being performed
	 *           over a network connection, and the result isn't
	 *           immediately available, this method returns false.
	 *           In all other circumstances it will return true.
	 */
	virtual bool prepare_match(bool nowait) = 0;

	/// Start the remote match going
	virtual void start_match(om_doccount maxitems) { }
	
	virtual PostList * get_postlist(om_doccount maxitems, MultiMatch *matcher) = 0;

	virtual Document * open_document(om_docid did) const = 0;

	virtual const std::map<om_termname, OmMSet::Internal::TermFreqAndWeight> get_term_info() const = 0;
};

#endif /* OM_HGUARD_MATCH_H */
