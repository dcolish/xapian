/* omega.h: Main header for omega
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Lemur Consulting Ltd
 * Copyright 2001 Ananova Ltd
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

#include <om/om.h>

#include "config.h"

#define PROGRAM_NAME "omega"

#include "configfile.h"

#include <map>
#include <vector>

// FIXME: yuckky
using std::string;
using std::vector;
using std::map;
using std::multimap;
using std::cout;
using std::endl;

extern string dbname;
extern string fmtname;

extern OmDatabase * omdb;
extern OmEnquire * enquire;
extern OmRSet * rset;

extern om_docid topdoc;
extern om_docid hits_per_page;

extern int threshold;

extern map<string, string> option;

extern string date1, date2, daysminus;

extern const string default_dbname;

class ExpandDeciderOmega : public OmExpandDecider {
    public:
	int operator()(const om_termname & tname) const {
	    // only suggest 4 or more letter words for now to
	    // avoid italian problems FIXME: fix this at index time
	    if (tname.length() <= 3) return false;

	    // also avoid terms with a prefix and with a space in
	    if (isupper(tname[0]) || tname.find(' ') != string::npos)
		return false;

	    return true;
	}
};
