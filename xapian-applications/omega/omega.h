/* omega.h: Main header for omega
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Lemur Consulting Ltd
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>

#include <om/om.h>

#define PROGRAM_NAME "omega"

#include "configfile.h"

#include <map>
#include <vector>

using namespace std;

extern string dbname;
extern string fmtname;
extern string filters;

extern OmDatabase omdb;
extern OmEnquire * enquire;
extern OmRSet * rset;

extern om_docid topdoc;
extern om_docid hits_per_page;
extern om_docid min_hits;

extern int threshold;

extern bool sort_numeric;
extern om_valueno sort_key;
extern int sort_bands;

extern map<string, string> option;

extern string date1, date2, daysminus;

extern const string default_dbname;

class ExpandDeciderOmega : public OmExpandDecider {
    private:
	OmDatabase db;
    public:
	ExpandDeciderOmega(const OmDatabase &db_) : db(db_) { }
	int operator()(const om_termname & tname) const {
	    // only suggest 4 or more letter words for now to
	    // avoid italian problems FIXME: fix this at index time
	    if (tname.length() <= 3) return false;

	    // Ignore a term that only occurs once (a hapax) since it's not
	    // useful for finding related documents - it only occurs in a
	    // document that's already been marked as relevant
	    if (db.get_termfreq(tname) <= 1) return false;

	    // Raw terms are OK, otherwise avoid terms with a prefix or with a space in
	    if (tname[0] == 'R') return true;
	    if (isupper(tname[0]) || tname.find(' ') != string::npos)
		return false;

	    return true;
	}
};
