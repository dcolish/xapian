/* bm25weight.cc: C++ class for weight calculation routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <math.h>

#include "omdebug.h"
#include "bm25weight.h"
#include "stats.h"

///////////////////////////////////////////////////////////////////////////

// The following parameters cause BM25Weight to behave identically to
// TradWeight.
// param_A = 1;
// param_B = 1;
// param_C = 0;
// param_D = 1;

BM25Weight::BM25Weight(const OmSettings & opts)
{
    DEBUGCALL(MATCH, void, "BM25Weight", opts);
    param_A = opts.get_real("bm25weight_A", 1);
    param_B = opts.get_real("bm25weight_B", 1);
    param_C = opts.get_real("bm25weight_C", 0);
    param_D = opts.get_real("bm25weight_D", 0.5);
    min_normlen = opts.get_real("bm25weight_min_normlen", 0.5);

    if (param_A < 0) throw OmInvalidArgumentError("Parameter A in BM25 weighting formula must be at least 0.");
    if (param_B < 0) throw OmInvalidArgumentError("Parameter B in BM25 weighting formula must be at least 0.");
    if (param_C < 0) throw OmInvalidArgumentError("Parameter C in BM25 weighting formula must be at least 0.");
    if (param_D < 0) throw OmInvalidArgumentError("Parameter D in BM25 weighting formula must be at least 0.");
    if (param_D > 1) throw OmInvalidArgumentError("Parameter D in BM25 weighting formula must be less than or equal to 1.");
}

// Calculate weights using statistics retrieved from databases
void
BM25Weight::calc_termweight() const
{
    DEBUGCALL(MATCH, void, "BM25Weight::calc_termweight", "");
    Assert(initialised);

    om_doccount dbsize = stats->get_total_collection_size();
    param_BD = param_B * param_D;
    lenpart = stats->get_total_average_length();

    // Just to ensure okay behaviour: should only happen if no data
    // (though there could be empty documents).
    if (lenpart == 0) lenpart = 1;

    lenpart = 1 / lenpart;

    om_doccount termfreq = stats->get_total_termfreq(tname);

    DEBUGMSG(WTCALC, "Statistics: N=" << dbsize << " n_t=" << termfreq);

    om_weight tw = 0;
    om_doccount rsize = stats->get_total_rset_size();
    if (rsize != 0) {
	om_doccount rtermfreq = stats->get_total_reltermfreq(tname);

	DEBUGMSG(WTCALC, " R=" << rsize << " r_t=" << rtermfreq);

	tw = (rtermfreq + 0.5) * (dbsize - rsize - termfreq + rtermfreq + 0.5) /
	     ((rsize - rtermfreq + 0.5) * (termfreq - rtermfreq + 0.5));
    } else {
	tw = (dbsize - termfreq + 0.5) / (termfreq + 0.5);
    }
    if (tw < 2) {
	// if size and/or termfreq is estimated we can get tw <= 0
	// so handle this gracefully
	if (tw <= 1e-6) tw = 1e-6;
	tw = tw / 2 + 1;
    }
    tw = log(tw);
    
    tw *= (param_A + 1) * wqf / (param_A + wqf);

    DEBUGLINE(WTCALC, " => termweight = " << tw);
    termweight = tw;
    weight_calculated = true;
}

om_weight
BM25Weight::get_sumpart(om_termcount wdf, om_doclength len) const
{
    DEBUGCALL(MATCH, om_weight, "BM25Weight::get_sumpart", wdf << ", " << len);
    if (!weight_calculated) calc_termweight();

    om_doclength normlen = len * lenpart;
    if (normlen < min_normlen) normlen = min_normlen;

    double denom = (normlen * param_BD + param_B * (1 - param_D) + wdf);
    om_weight wt;
    if (denom != 0) {
	wt = (double) wdf * (param_B + 1) / denom;
    } else {
	wt = 0;
    }
    DEBUGMSG(WTCALC, "(wdf,len,lenpart) = (" << wdf << "," << len << "," <<
	     lenpart << ") =>  wtadj = " << wt);

    wt *= termweight;

    DEBUGLINE(WTCALC, " =>  sumpart = " << wt);

    RETURN(wt);
}

om_weight
BM25Weight::get_maxpart() const
{
    DEBUGCALL(MATCH, om_weight, "BM25Weight::get_maxpart", "");
    if(!weight_calculated) calc_termweight();
    DEBUGLINE(WTCALC, "maxpart = " << ((param_B + 1) * termweight));
    RETURN((param_B + 1) * termweight);
}

/* Should return param_C * querysize * (1-len) / (1+len)
 * However, want to return a positive value, so add (param_C * querysize) to
 * return.  ie: return param_C * querysize / (1 + len)  (factor of 2 is
 * incorporated into param_C)
 */
om_weight
BM25Weight::get_sumextra(om_doclength len) const
{
    DEBUGCALL(MATCH, om_weight, "BM25Weight::get_sumextra", len);
    om_doclength normlen = len * lenpart;
    if (normlen < min_normlen) normlen = min_normlen;
    om_weight extra = param_C * querysize / (1 + normlen);
    DEBUGLINE(WTCALC, "len = " << len <<
	      " querysize = " << querysize <<
	      " =>  normlen = " << normlen <<
	      " =>  sumextra = " << extra);
    RETURN(extra);
}

om_weight
BM25Weight::get_maxextra() const
{
    DEBUGCALL(MATCH, om_weight, "BM25Weight::get_maxextra", "");
    om_weight maxextra = param_C * querysize;
    DEBUGLINE(WTCALC, "querysize = " << querysize <<
	      " =>  maxextra = " << maxextra);
    RETURN(maxextra);
}
