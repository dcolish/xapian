/* expand.h: class for finding expand terms
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

#ifndef OM_HGUARD_EXPAND_H
#define OM_HGUARD_EXPAND_H

#include "database.h"
#include "termlist.h"
#include "om/omenquire.h"

#include <queue>
#include <stack>
#include <vector>
#include "autoptr.h"

/** Expand decision functor which always decides to use the term. */
class OmExpandDeciderAlways : public OmExpandDecider {
    public:
	int operator()(const om_termname & tname) const { return true; }
};

/** Class for performing the expand operation. */
class OmExpand {
    private:
	// disallow copy
	OmExpand(const OmExpand &);
	void operator=(const OmExpand &);

	const OmDatabase &db;

        bool recalculate_maxweight;
	AutoPtr<TermList> build_tree(const RSet *rset,
				      const OmExpandWeight *ewt);
    public:
        OmExpand(const OmDatabase &db_);

	void expand(om_termcount max_esize,
		    OmESet & eset,
		    const RSet * rset,
		    const OmExpandDecider * decider,
		    bool use_exact_termfreq,
		    double expand_k );
};

inline OmExpand::OmExpand(const OmDatabase &db_) : db(db_)
{
}

#endif /* OM_HGUARD_EXPAND_H */
