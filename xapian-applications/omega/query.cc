/* query.cc: query executor for omega
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Intercede 1749 Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <algorithm>
#include <vector>
#include <map>
#include <set>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#include <unistd.h>
#elif defined HAVE_FTIME
#include <sys/timeb.h>
#else
#include <time.h>
#endif

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#include "utils.h"
#include "omega.h"
#include "query.h"
#include "cgiparam.h"
#include "om/omparsequery.h"

using namespace std;

static bool done_query = false;
static om_docid last = 0;

static OmMSet mset;

static void ensure_match();

string raw_prob;
map<om_docid, bool> ticked;

static OmQuery query;
static string query_string;
OmQuery::op default_op = OmQuery::OP_OR; // default matching mode

static OmQueryParser qp;
static OmStem *stemmer = NULL;

static string eval_file(const string &fmtfile);

static set<om_termname> termset;

static string queryterms;

static string error_msg;

static long int sec = 0, usec = -1;

class MyStopper : public OmStopper {
  public:
    bool operator()(const om_termname &t) {
	switch (t[0]) {
	    case 'a':
		return (t == "a" || t == "about" || t == "an" || t == "and" ||
			t == "are" || t == "as" || t == "at");
	    case 'b':
		return (t == "be" || t == "by");
	    case 'e':
		return (t == "en");
	    case 'f':
		return (t == "for" || t == "from");
	    case 'h':
		return (t == "how");
	    case 'i':
		return (t == "i" || t == "in" || t == "is" || t == "it");
	    case 'o':
		return (t == "of" || t == "on" || t == "or");
	    case 't':
		return (t == "that" || t == "the" || t == "this" || t == "to");
	    case 'w':
		return (t == "was" || t == "what" || t == "when" ||
		       	t == "where" || t == "which" || t == "who" ||
		       	t == "why" || t == "will" || t == "with");
	    default:
		return false;
	}
    }
};

querytype
set_probabilistic(const string &newp, const string &oldp)
{
    // strip leading and trailing whitespace
    string::size_type first_nonspace = newp.find_first_not_of(" \t\r\n\v");
    if (first_nonspace == string::npos) {
	raw_prob = "";
    } else {
	string::size_type len = newp.find_last_not_of(" \t\r\n\v");
	raw_prob = newp.substr(first_nonspace, len + 1 - first_nonspace);
    }

    // call YACC generated parser
    qp.set_stemming_options(option["no_stem"] == "true" ? "" : "english",
			    option["all_stem"] == "true",
			    new MyStopper()); 
    qp.set_default_op(default_op);
    qp.set_database(omdb);
    try {
	query = qp.parse_query(raw_prob);
    } catch (const char *s) {
	error_msg = s;
	return BAD_QUERY;
    }

    om_termcount n_new_terms = 0;
    for (list<om_termname>::const_iterator i = qp.termlist.begin();
	 i != qp.termlist.end(); ++i) {
	if (termset.find(*i) == termset.end()) {
	    termset.insert(*i);
	    if (!queryterms.empty()) queryterms += '\t';
	    queryterms += *i;
	}		    
	n_new_terms++;
    }

    // Check new query against the previous one
    const char *pend;
    const char *term;
    unsigned int n_old_terms = 0;

    if (oldp.empty()) return newp.empty() ? SAME_QUERY : NEW_QUERY;

    // We used to use "word1#word2#" (with trailing #) but some broken old
    // browsers (versions of MSIE) don't quote # in form GET submissions
    // and everything after the # gets interpreted as an anchor.
    // So now we use "word1.word2." instead.
    term = oldp.c_str();
    pend = term;
    while ((pend = strchr(pend, '.')) != NULL) {
	pend++;
	n_old_terms++;
    }
    // short-cut: if the new query has fewer terms, it must be a new one
    if (n_new_terms < n_old_terms) return NEW_QUERY;
    
    while ((pend = strchr(term, '.')) != NULL) {
	if (termset.find(string(term, pend - term)) == termset.end())
	    return NEW_QUERY;
	term = pend + 1;
    }
    // Use termset.size() rather than n_new_terms so we correctly handle
    // the case when the query has repeated terms.
    // This works wrongly in the case when the user extends the query
    // by adding a term already in it, but that's unlikely and the behaviour
    // isn't too bad (we just don't reset page 1).  We also mishandle a few
    // other obscure cases e.g. adding quotes to turn a query into a phrase.
    if (termset.size() > n_old_terms) return EXTENDED_QUERY;
    return SAME_QUERY;
}

static multimap<string, string> filter_map;

typedef multimap<string, string>::const_iterator FMCI;
    
void add_bterm(const string &term) {
    string prefix;
    if (term[0] == 'X') {
	string::const_iterator i = term.begin() + 1;
	while (i != term.end() && isupper(*i)) ++i;
	if (i != term.end() && *i == ':') ++i;
	prefix = string(term.begin(), i);
    } else {
	prefix = term[0];
    }
	
    filter_map.insert(make_pair(prefix, term));
}

static int
last_day(int y, int m)
{
    static const int l[13] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (m != 2) return l[m];
    return 28 + (y % 4 == 0); // good until 2100
}

static OmQuery
date_range_filter(int y1, int m1, int d1, int y2, int m2, int d2)
{
    char buf[9];
    sprintf(buf, "%04d%02d", y1, m1);
    vector<OmQuery> v;

    int d_last = last_day(y1, m1);
    int d_end = d_last;
    if (y1 == y2 && m1 == m2 && d2 < d_last) {
	d_end = d2;
    }
    // Deal with any initial partial month
    if (d1 > 1 || d_end < d_last) {
    	for ( ; d1 <= d_end ; d1++) {
	    sprintf(buf + 6, "%02d", d1);
	    v.push_back(OmQuery('D' + string(buf)));
	}
    } else {
	v.push_back(OmQuery('M' + string(buf)));
    }
    
    if (y1 == y2 && m1 == m2) {
	return OmQuery(OmQuery::OP_OR, v.begin(), v.end());
    }

    int m_last = (y1 < y2) ? 12 : m2 - 1;
    while (++m1 <= m_last) {
	sprintf(buf + 4, "%02d", m1);
	v.push_back(OmQuery('M' + string(buf)));
    }
	
    if (y1 < y2) {
	while (++y1 < y2) {
	    sprintf(buf, "%04d", y1);
	    v.push_back(OmQuery('Y' + string(buf)));
	}
	sprintf(buf, "%04d", y2);
	for (m1 = 1; m1 < m2; m1++) {
	    sprintf(buf + 4, "%02d", m1);
	    v.push_back(OmQuery('M' + string(buf)));
	}
    }
	
    sprintf(buf + 4, "%02d", m2);

    // Deal with any final partial month
    if (d2 < last_day(y2, m2)) {
    	for (d1 = 1 ; d1 <= d2; d1++) {
	    sprintf(buf + 6, "%02d", d1);
	    v.push_back(OmQuery('D' + string(buf)));
	}
    } else {
	v.push_back(OmQuery('M' + string(buf)));
    }

    return OmQuery(OmQuery::OP_OR, v.begin(), v.end());
}

static void
parse_date(string date, int *y, int *m, int *d)
{
    // FIXME: for now only support YYYYMMDD (e.g. 20011119)
    // and don't error check
    *y = atoi(date.substr(0, 4).c_str());
    *m = atoi(date.substr(4, 2).c_str());
    *d = atoi(date.substr(6, 2).c_str());
}

static void
run_query()
{
    if (!filter_map.empty()) {
	// OR together filters with the same prefix, then AND together
	vector<OmQuery> filter_vec;
	vector<om_termname> or_vec;
	string current;
	for (FMCI i = filter_map.begin(); ; i++) {
	    bool over = (i == filter_map.end());
	    if (over || i->first != current) {
		switch (or_vec.size()) {
		    case 0:
		        break;
		    case 1:
			filter_vec.push_back(OmQuery(or_vec[0]));
		        break;
		    default:
			filter_vec.push_back(OmQuery(OmQuery::OP_OR,
						     or_vec.begin(),
						     or_vec.end()));
		        break;
		}
		or_vec.clear();
		if (over) break;
		current = i->first;
	    }
	    or_vec.push_back(i->second);
	}
	
	// if only boolean query is provided then promote that
	// to be THE query instead of filtering an empty query
	// So we can have pure boolean queries this way
	if (query.is_empty()) {
	    query = OmQuery(OmQuery::OP_AND,
			    filter_vec.begin(),
			    filter_vec.end());
	} else {
	    query = OmQuery(OmQuery::OP_FILTER,
		    query,
		    OmQuery(OmQuery::OP_AND,
			    filter_vec.begin(),
			    filter_vec.end()));
	}
    }

    if (!date_start.empty() || !date_end.empty() || !date_span.empty()) {
	int y1, m1, d1, y2, m2, d2;
	if (!date_span.empty()) {
	    time_t secs = atoi(date_span.c_str()) * (24 * 60 * 60);
	    if (!date_end.empty()) {
		parse_date(date_end, &y2, &m2, &d2);
		struct tm t;
		t.tm_year = y2 - 1900;
		t.tm_mon = m2 - 1;
		t.tm_mday = d2;
		t.tm_hour = 12;
		t.tm_min = t.tm_sec = 0;
		t.tm_isdst = -1;
		time_t then = mktime(&t) - secs;
		struct tm *t2 = localtime(&then);
		y1 = t2->tm_year + 1900;
		m1 = t2->tm_mon + 1;
		d1 = t2->tm_mday;
	    } else if (!date_start.empty()) {
		parse_date(date_start, &y1, &m1, &d1);	
		struct tm t;
		t.tm_year = y1 - 1900;
		t.tm_mon = m1 - 1;
		t.tm_mday = d1;
		t.tm_hour = 12;
		t.tm_min = t.tm_sec = 0;
		t.tm_isdst = -1;
		time_t end = mktime(&t) + secs;
		struct tm *t2 = localtime(&end);
		y2 = t2->tm_year + 1900;
		m2 = t2->tm_mon + 1;
		d2 = t2->tm_mday;
	    } else {
		time_t end = time(NULL);
		struct tm *t = localtime(&end);
		y2 = t->tm_year + 1900;
		m2 = t->tm_mon + 1;
		d2 = t->tm_mday;
		time_t then = end - secs;
		struct tm *t2 = localtime(&then);
		y1 = t2->tm_year + 1900;
		m1 = t2->tm_mon + 1;
		d1 = t2->tm_mday;
	    }
	} else {
	    if (date_start.empty()) {
		y1 = 1970;
		m1 = 1;
		d1 = 1;
	    } else {
		parse_date(date_start, &y1, &m1, &d1);	
	    }
	    if (date_end.empty()) {
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		y2 = t->tm_year + 1900;
		m2 = t->tm_mon + 1;
		d2 = t->tm_mday;
	    } else {
		parse_date(date_end, &y2, &m2, &d2);
	    }
	}

	query = OmQuery(OmQuery::OP_FILTER,
		       	query,
			OmQuery(OmQuery::OP_OR,
			date_range_filter(y1, m1, d1, y2, m2, d2),
			OmQuery("Dlatest")));
    }

    if (enquire) {
	enquire->set_cutoff(threshold);
        // match_min_hits will be moved into matcher soon
	// enquire->set_min_hits(min_hits); or similar...

	// Temporary bodge to allow experimentation with OmBiasFunctor
	MCI i;
	i = cgi_params.find("bias_weight");
	if (i != cgi_params.end()) {
	    om_weight bias_weight = atof(i->second.c_str());
	    int half_life = 2 * 24 * 60 * 60; // 2 days
	    i = cgi_params.find("bias_halflife");
	    if (i != cgi_params.end()) {
		half_life = atoi(i->second.c_str());
	    }
	    enquire->set_bias(bias_weight, half_life);
	}
	if (sort_bands) {
	    enquire->set_sorting(sort_key, sort_bands);
	    // FIXME: ignore sort_numeric for now
	}
	if (collapse) {
	    enquire->set_collapse_key(collapse_key);
	}
				
	// FIXME - set msetcmp to reverse?

#ifdef HAVE_GETTIMEOFDAY
	struct timeval tv;
	if (gettimeofday(&tv, 0) == 0) {
	    sec = tv.tv_sec;
	    usec = tv.tv_usec;
	}
#elif defined(HAVE_FTIME)
	struct timeb tp;
	if (ftime(&tp) == 0) {
	    sec = tp.time;
	    usec = tp.millitm * 1000;
	}
#else
	sec = time(NULL);
	if (sec != (time_t)-1) usec = 0;
#endif
	enquire->set_query(query);
	// We could use the value of topdoc as first parameter, but we
	// need to know the first few items on the mset to fake a
	// relevance set for topterms.
	//
	// Fetch one extra result so we know if we've reached the end of the
	// matches or not - then we can avoid offering a "next" button which
	// leads to an empty page
	mset = enquire->get_mset(0, topdoc + max(hits_per_page + 1,min_hits),
				 rset);
	if (usec != -1) {
#ifdef HAVE_GETTIMEOFDAY
	    if (gettimeofday(&tv, 0) == 0) {
		sec = tv.tv_sec - sec;
		usec = tv.tv_usec - usec;
		if (usec < 0) {
		    --sec;
		    usec += 1000000;
		}
	    } else {
		usec = -1;
	    }
#elif defined(HAVE_FTIME)
	    struct timeb tp;
	    if (ftime(&tp) == 0) {
		sec = tp.time - sec;
		usec = tp.millitm * 1000 - usec;
		if (usec < 0) {
		    --sec;
		    usec += 1000000;
		}
	    } else {
		usec = -1;
	    }
#else
	    usec = time(NULL);
	    if (usec != -1) {
		sec = sec - usec;
		usec = 0;
	    }
#endif
	}
    }
}

#if 0
// generate a sorted picker  FIXME: should be generatable by script language
//
// output looks like this:
//
// <SELECT NAME=B>
// <OPTION VALUE="" SELECTED>
// <OPTION VALUE="Mtext/html">HTML
// <OPTION VALUE="Mapplication/postscript">PostScript
// <OPTION VALUE="Mtext/plain">Text
// </SELECT>
static void
do_picker(string prefix, const char **opts)
{
    const char **p;
    bool do_current = false;
    string current;
    vector<string> picker;

    // FIXME: what if multiple values present for a prefix???
    FMCI i = filter_map.find(prefix);
    if (i != filter_map.end() && i->second.length() > prefix.length()) {
	current = i->second.substr(prefix.length());
	do_current = true;
    }

    cout << "<SELECT NAME=B>\n<OPTION VALUE=\"\"";

    // Some versions of MSIE don't default to selecting the first option,
    // so we explicitly have to
    if (!do_current) cout << " SELECTED";

    cout << '>';

    string tmp = option[string('B') + prefix];
    if (!tmp.empty())
	cout << tmp;
    else
	cout << "-Any-";
    
    for (p = opts; *p; p++) {
	string trans = option[string('B') + prefix + *p];
	if (trans.empty()) {
	    // FIXME: nasty special casing on prefix...
	    if (prefix == "N")
		trans = string(".") + *p;
	    else 
		trans = "[" + string(*p) + "]";
	}
	if (do_current && current == *p) {
	    trans += '\n';
	    do_current = false;
	}
	picker.push_back(trans + '\t' + string(*p));
    }
   
    sort(picker.begin(), picker.end());

    vector<string>::const_iterator i2;
    for (i2 = picker.begin(); i2 != picker.end(); i2++) {
	string::size_type j = (*i2).find('\t');
	if (j == string::npos) continue;
	const char *p = (*i2).c_str();
	cout << "\n<OPTION VALUE=" << prefix << string(p + j + 1);
	if (j >= 1 && p[j - 1] == '\n') cout << " SELECTED";
	cout << '>' << string(p, j);
    }

    if (do_current) {
	cout << "\n<OPTION VALUE=\"" << prefix << current << "\" SELECTED>"
             << current;
    }
    cout << "</SELECT>\n";
}
#endif

static string
percent_encode(const string &str)
{
    string res;
    const char *p = str.c_str();
    while (true) {
	char ch = *p++;
	if (ch == 0) return res;
	if (ch <= 32 || ch >= 127 || strchr("#%&,/:;<=>?@[\\]^_{|}", ch)) {
	    char buf[4];
	    sprintf(buf, "%%%02x", ch);
	    res.append(buf, 3);
	} else {
	    res += ch;
	}
    }
}

static string
html_escape(const string &str)
{
    string res;
    string::size_type p = 0;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
 	    case '<':
	        res += "&lt;";
	        continue;
	    case '>':
	        res += "&gt;";
	        continue;
	    case '&':
	        res += "&amp;";
	        continue;
	    case '"':
	        res += "&quot;";
	        continue;
	    default:
	        res += ch;
	}
    }
    return res;
}

static string
html_strip(const string &str)
{
    string res;
    string::size_type p = 0;
    bool skip = false;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
 	    case '<':
	        skip = true;
	        continue;
	    case '>':
	        skip = false;
	        continue;
	    default:
	        if (! skip) res += ch;
	}
    }
    return res;
}

// FIXME split list into hash or map and use that rather than linear lookup?
static bool word_in_list(const string& test_word, const string& list)
{
    string::size_type split = 0, split2;
    while ((split2 = list.find('\t', split)) != string::npos) {
	if (test_word == list.substr(split, split2 - split)) return true;
	split = split2 + 1;
    }
    return (test_word == list.substr(split, split2 - split));
}

// FIXME: this copied from om/indexer/index_utils.cc
static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while (i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

inline static bool
p_alnum(unsigned int c)
{
    return isalnum(c);
}

inline static bool
p_notalnum(unsigned int c)
{
    return !isalnum(c);
}

// Not a character in an indentifier
inline static bool
p_notid(unsigned int c)
{
    return !isalnum(c) && c != '_';
}

// Not a character in an HTML tag name
inline static bool
p_nottag(unsigned int c)
{
    return !isalnum(c) && c != '.' && c != '-';
}

inline static bool
p_notplusminus(unsigned int c)
{
    return c != '+' && c != '-';
}

// FIXME: shares algorithm with omindex.cc!
static string
html_highlight(const string &s, const string &list,
	       const string &bra, const string &ket)
{
    if (!stemmer) {
	stemmer = new OmStem(option["no_stem"] == "true" ? "" : "english");
    }
    string::const_iterator i, j = s.begin(), k, l;
    string res;
    while ((i = find_if(j, s.end(), p_alnum)) != s.end()) {
	om_termname term, word;
	l = j;
	if (isupper(*i)) {
	    j = i;
	    term = *j;
	    while (++j != s.end() && *j == '.' &&
		   ++j != s.end() && isupper(*j)) {
		term += *j;
	    } 
	    if (term.length() < 2 || (j != s.end() && isalnum(*j))) {
		term = "";
	    } else {
		word = s.substr(i - s.begin(), j - i);
	    }
	}
	if (term.empty()) {
	    k = i;
moreterm:
	    j = find_if(k, s.end(), p_notalnum);
	    if (j != s.end() && *j == '&') {
		if (j + 1 != s.end() && isalnum(j[1])) {
		    k = j + 1;
		    goto moreterm;
		}
	    }
	    k = find_if(j, s.end(), p_notplusminus);
	    if (k == s.end() || !isalnum(*k)) j = k;
	    term = s.substr(i - s.begin(), j - i);
	    word = term;
	}
        lowercase_term(term);

	res += html_escape(s.substr(l - s.begin(), i - l));
	bool match = false;
        if (isupper(*i) || isdigit(*i)) {
	    if (word_in_list('R' + term, list)) match = true;
        }
	if (!match && word_in_list(stemmer->stem_word(term), list)) match = true;
	if (match) res += bra;
	res += html_escape(word);
	if (match) res += ket;
    }
    if (j != s.end()) res += html_escape(s.substr(j - s.begin()));
    return res;
}

#if 0
static void
print_query_string(const char *after)
{
    if (after && strncmp(after, "&B=", 3) == 0) {
	char prefix = after[3];
	string::size_type start = 0, amp = 0;
	while (true) {
	    amp = query_string.find('&', amp);
	    if (amp == string::npos) {
		cout << query_string.substr(start);
		return;
	    }
	    amp++;
	    while (query_string[amp] == 'B' &&
		   query_string[amp + 1] == '=' &&
		   query_string[amp + 2] == prefix) {
		cout << query_string.substr(start, amp - start - 1);
		start = query_string.find('&', amp + 3);
		if (start == string::npos) return;
		amp = start + 1;
	    }
	}
    }
    cout << query_string;
}
#endif

static map<string, string> field;
static om_docid q0;
static om_doccount hit_no;
static int percent;
static om_doccount collapsed;

static string print_caption(const string &fmt, const vector<string> &param);

enum tagval {
CMD_,
CMD_add,
CMD_allterms,
CMD_and,
CMD_cgi,
CMD_cgilist,
CMD_collapsed,
CMD_date,
CMD_dbname,
CMD_dbsize,
CMD_def,
CMD_defaultop,
CMD_div,
CMD_eq,
CMD_env,
CMD_error,
CMD_field,
CMD_filesize,
CMD_filters,
CMD_fmt,
CMD_freq,
CMD_freqs,
CMD_ge,
CMD_gt,
CMD_highlight,
CMD_hit,
CMD_hitlist,
CMD_hitsperpage,
CMD_hostname,
CMD_html,
CMD_htmlstrip,
CMD_id,
CMD_if,
CMD_include,
CMD_last,
CMD_lastpage,
CMD_le,
CMD_list,
CMD_lt,
CMD_map,
CMD_max,
CMD_min,
CMD_mod,
CMD_msize,
CMD_msizeexact,
CMD_mul,
CMD_ne,
CMD_nice,
CMD_not,
CMD_opt,
CMD_or,
CMD_percentage,
CMD_prettyterm,
CMD_query,
CMD_querydescription,
CMD_queryterms,
CMD_range,
CMD_record,
CMD_relevant,
CMD_relevants,
CMD_score,
CMD_set,
CMD_setmap,
CMD_setrelevant,
CMD_slice,
CMD_sub,
CMD_terms,
CMD_thispage,
CMD_time,
CMD_topdoc,
CMD_topterms,
#ifdef HAVE_PCRE
CMD_transform,
#endif
CMD_uniq,
CMD_unstem,
CMD_url,
CMD_value,
CMD_version,
CMD_MACRO // special tag for macro evaluation
};

struct func_attrib {
    int tag;
    int minargs, maxargs, evalargs;
    bool ensure_match;
    bool cache; // FIXME: implement cache
};

#define STRINGIZE(N) _STRINGIZE(N)
#define _STRINGIZE(N) #N
    
#define T(F) STRINGIZE(F), { CMD_##F
struct func_desc {
    const char *name;
    struct func_attrib a;
};

#define N -1
static struct func_desc func_tab[] = {
//name minargs maxargs evalargs ensure_match cache
{"",{CMD_,	N, N, 0, 0, 0}}, // commented out code
{T(add),	0, N, N, 0, 0}}, // add a list of numbers
{T(allterms),	0, 1, N, 0, 0}}, // list of all terms matching document
{T(and),	1, N, 0, 0, 0}}, // logical shortcutting and of a list of values
{T(cgi),	1, 1, N, 0, 0}}, // return cgi parameter value
{T(cgilist),	1, 1, N, 0, 0}}, // return list of values for cgi parameter
{T(collapsed),	0, 0, N, 0, 0}}, // return number of hits collapsed into this
{T(date),	1, 2, N, 1, 0}}, // convert time_t to strftime format (default: YYYY-MM-DD)
{T(dbname),	0, 0, N, 0, 0}}, // database name
{T(dbsize),	0, 0, N, 0, 1}}, // database size (# of documents)
{T(def),	2, 2, 1, 0, 0}}, // define a macro
{T(defaultop),	0, 0, N, 0, 0}}, // default operator: "and" or "or"
{T(div),	2, 2, N, 0, 0}}, // integer divide
{T(env),	1, 1, N, 0, 0}}, // environment variable
{T(error),	0, 0, N, 0, 0}}, // error message
{T(eq),		2, 2, N, 0, 0}}, // test equality
{T(field),	1, 1, N, 0, 0}}, // lookup field in record
{T(filesize),	1, 1, N, 0, 0}}, // pretty printed filesize
{T(filters),	0, 0, N, 0, 0}}, // serialisation of current filters
{T(fmt),	0, 0, N, 0, 0}}, // name of current format
{T(freq),	1, 1, N, 0, 0}}, // frequency of a term
{T(freqs),	0, 0, N, 1, 1}}, // return HTML string listing query terms and frequencies
{T(ge),		2, 2, N, 0, 0}}, // test >=
{T(gt),		2, 2, N, 0, 0}}, // test >
{T(highlight),	2, 4, N, 0, 0}}, // html escape and highlight words from list
{T(hit),	0, 0, N, 0, 0}}, // hit number of current mset entry (starting
				 // from 0)
{T(hitlist),	1, 1, 0, 1, 0}}, // display hitlist using format in argument
{T(hitsperpage),0, 0, N, 0, 0}}, // hits per page
{T(hostname),	1, 1, N, 0, 0}}, // extract hostname from URL
{T(html),	1, 1, N, 0, 0}}, // html escape string (<>&")
{T(htmlstrip),	1, 1, N, 0, 0}}, // html strip tags string (s/<[^>]*>?//g)
{T(id),		0, 0, N, 0, 0}}, // docid of current doc
{T(if),		2, 3, 1, 0, 0}}, // conditional
{T(include),	1, 1, 1, 0, 0}}, // include another file
{T(last),	0, 0, N, 1, 0}}, // m-set number of last hit on page
{T(lastpage),	0, 0, N, 1, 0}}, // number of last hit page
{T(le),		2, 2, N, 0, 0}}, // test <=
{T(list),	2, 5, N, 0, 0}}, // pretty print list
{T(lt),		2, 2, N, 0, 0}}, // test <
{T(map),	1, 2, 1, 0, 0}}, // map a list into another list
{T(max),	1, N, N, 0, 0}}, // maximum of a list of values
{T(min),	1, N, N, 0, 0}}, // minimum of a list of values
{T(mod),	2, 2, N, 0, 0}}, // integer modulus
{T(msize),	0, 0, N, 1, 0}}, // number of matches
{T(msizeexact),	0, 0, N, 1, 0}}, // is $msize exact?
{T(mul),	2, N, N, 0, 0}}, // multiply a list of numbers
{T(ne), 	2, 2, N, 0, 0}}, // test not equal
{T(nice),	1, 1, N, 0, 0}}, // pretty print integer (with thousands sep)
{T(not),	1, 1, N, 0, 0}}, // logical not
{T(opt),	1, 2, N, 0, 0}}, // lookup an option value
{T(or),		1, N, 0, 0, 0}}, // logical shortcutting or of a list of values
{T(percentage),	0, 0, N, 0, 0}}, // percentage score of current hit
{T(prettyterm),	1, 1, N, 0, 0}}, // pretty print term name
{T(query),	0, 0, N, 0, 0}}, // query
{T(querydescription),	0, 0, N, 0, 0}}, // query.get_description()
{T(queryterms),	0, 0, N, 0, 0}}, // list of query terms
{T(range),	2, 2, N, 0, 0}}, // return list of values between start and end
{T(record),	0, 1, N, 1, 0}}, // record contents of document
{T(relevant),	0, 1, N, 1, 0}}, // is document relevant?
{T(relevants),	0, 0, N, 1, 0}}, // return list of relevant documents
{T(score),	0, 0, N, 0, 0}}, // score (0-10) of current hit
{T(set),	2, 2, N, 0, 0}}, // set option value
{T(setmap),	1, N, N, 0, 0}}, // set map of option values
{T(setrelevant),0, 1, N, 0, 0}}, // set rset
{T(slice),	2, 2, N, 0, 0}}, // slice a list using a second list
{T(sub),	2, 2, N, 0, 0}}, // subtract
{T(terms),	0, 0, N, 1, 0}}, // list of matching terms
{T(thispage),	0, 0, N, 1, 0}}, // page number of current page
{T(time),	0, 0, N, 1, 0}}, // how long the match took (in seconds)
{T(topdoc),	0, 0, N, 0, 0}}, // first document on current page of hit list (counting from 0)
// FIXME: cache really needs to be smart about parameter value...
{T(topterms),	0, 1, N, 1, 1}}, // list of up to N top relevance feedback terms (default 16)
#ifdef HAVE_PCRE
{T(transform),  3, 3, N, 0, 0}}, // transform with a regexp
#endif
{T(uniq),	1, 1, N, 0, 0}}, // removed duplicates from a sorted list
{T(unstem),	1, 1, N, 0, 0}}, // return list of probabilistic terms from
				 // the query which stemmed to this term
{T(url),	1, 1, N, 0, 0}}, // url encode argument
{T(value),	1, 2, N, 0, 0}}, // return document value/slot/key/whatever
{T(version),	0, 0, N, 0, 0}}, // omega version string
{ NULL,{0,      0, 0, 0, 0, 0}}
};

static vector<string> macros;

static string
eval(const string &fmt, const vector<string> &param)
{
    static map<string, const struct func_attrib *> func_map;
    if (func_map.empty()) {
	struct func_desc *p;
	for (p = func_tab; p->name != NULL; p++) {
	    func_map[string(p->name)] = &(p->a);
	}
    }
    string res;
    string::size_type p = 0, q;
    while ((q = fmt.find('$', p)) != string::npos) {
	res += fmt.substr(p, q - p);
	string::size_type code_start = q; // note down for error reporting
	q++;
	if (q >= fmt.size()) break;
	unsigned char ch = fmt[q];
	switch (ch) {
	    // Magic sequences:
	    // `$$' -> `$', `$(' -> `{', `$)' -> `}', `$.' -> `,'
	    case '$':
		res += '$';
		p = q + 1;
		continue;
	    case '(':
		res += '{';
		p = q + 1;
		continue;
	    case ')':
		res += '}';
		p = q + 1;
		continue;
	    case '.':
		res += ',';
		p = q + 1;
		continue;
	    case '_':
		ch = '0';
		// FALL THRU
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9':
		ch -= '0';
		if (ch < param.size()) res += param[ch];
		p = q + 1;
		continue;
	    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	    case 'y': case 'z': 
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
	    case '{':
		break;
	    default:
		string msg = "Unknown $ code in: $" + fmt.substr(q);
		throw msg;
	}
	p = find_if(fmt.begin() + q, fmt.end(), p_notid) - fmt.begin();
	string var = fmt.substr(q, p - q);
	map<string, const struct func_attrib *>::const_iterator i;
	i = func_map.find(var);
	if (i == func_map.end()) {
	    throw "Unknown function `" + var + "'";
	}
	vector<string> args;
	if (fmt[p] == '{') {
	    q = p + 1;
	    int nest = 1;
	    while (true) {
		p = fmt.find_first_of(",{}", p + 1);
		if (p == string::npos)
		    throw "missing } in " + fmt.substr(code_start);
		if (fmt[p] == '{') {
		    ++nest;
		} else {
		    if (nest == 1) {
			// should we split the args
			if (i->second->minargs != N) {
			    args.push_back(fmt.substr(q, p - q));
			    q = p + 1;
			}
		    }
		    if (fmt[p] == '}' && --nest == 0) break;
		}
	    }
	    if (i->second->minargs == N) args.push_back(fmt.substr(q, p - q));
	    p++;
	}

	if (i->second->minargs != N) {
	    if ((int)args.size() < i->second->minargs)
		throw "too few arguments to $" + var;
	    if (i->second->maxargs != N &&
		(int)args.size() > i->second->maxargs)
		throw "too many arguments to $" + var;

	    vector<string>::size_type n;
	    if (i->second->evalargs != N)
		n = i->second->evalargs;
	    else
		n = args.size();
	    
	    for (vector<string>::size_type j = 0; j < n; j++)
		args[j] = eval(args[j], param);
	}
	if (i->second->ensure_match) ensure_match();
	string value;
	switch (i->second->tag) {
	    case CMD_:
	        break;
	    case CMD_add: {
		int total = 0;
		vector<string>::const_iterator i;
		for (i = args.begin(); i != args.end(); i++)
		    total += string_to_int(*i);
		value = int_to_string(total);
		break;
	    }
	    case CMD_allterms: {
		// list of all terms indexing document
		int id = q0;
		if (!args.empty()) id = string_to_int(args[0]);
		OmTermIterator term = omdb.termlist_begin(id);
		for ( ; term != omdb.termlist_end(id); term++)
		    value = value + *term + '\t';

		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_and: {
		value = "true";
		for (vector<string>::const_iterator i = args.begin();
		     i != args.end(); i++) {
		    if (eval(*i, param).empty()) {
			value = "";
			break;
		    }
	        }
		break;
	    }
	    case CMD_cgi: {
		MCI i = cgi_params.find(args[0]);
		if (i != cgi_params.end()) value = i->second;
		break;
	    }
	    case CMD_cgilist: {
		pair<MCI, MCI> g;
		g = cgi_params.equal_range(args[0]);
		for (MCI i = g.first; i != g.second; i++)
		    value = value + i->second + '\t';
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_collapsed: {
		value = int_to_string(collapsed);
		break;
	    }
	    case CMD_date:
		value = args[0];
		if (!value.empty()) {
		    char buf[64] = "";
		    time_t date = string_to_int(value);
		    if (date != (time_t)-1) {
			struct tm *then;
			then = gmtime(&date);
			string fmt = "%Y-%m-%d";
			if (args.size() > 1) fmt = eval(args[1], param);
			strftime(buf, sizeof buf, fmt.c_str(), then);
		    }
		    value = buf;
		}
		break;
	    case CMD_dbname:
		value = dbname;
		break;
	    case CMD_dbsize:
		value = int_to_string(omdb.get_doccount());
		break;
	    case CMD_def: {
		func_attrib *fa = new func_attrib;
		fa->tag = CMD_MACRO + macros.size();
		fa->minargs = 0;
		fa->maxargs = 9;
		fa->evalargs = N; // FIXME: or 0?
		fa->ensure_match = fa->cache = false;
		
		macros.push_back(args[1]);
		func_map[args[0]] = fa;
		break;
	    }
	    case CMD_defaultop:
		if (default_op == OmQuery::OP_AND) {
		    value = "and";
		} else {
		    value = "or";
		}
		break;
	    case CMD_div: {
		int denom = string_to_int(args[1]);
		if (denom == 0) {
		    value = "divide by 0";
		} else {
		    value = int_to_string(string_to_int(args[0]) /
					  string_to_int(args[1]));
		}
		break;
	    }
	    case CMD_eq:
		if (args[0] == args[1]) value = "true";
		break;
	    case CMD_env: {
		char *env = getenv(args[0].c_str());
		if (env != NULL) value = env;
		break;
	    }
	    case CMD_error:
		if (error_msg.empty() && enquire == NULL) {
		    error_msg = "Database `" + dbname + "' couldn't be opened"; 
		}
		value = error_msg;
		break;
	    case CMD_field:
		value = field[args[0]];
		break;
	    case CMD_filesize: {
		// FIXME: rounding?
		// FIXME: for smaller sizes give decimal fractions, e.g. "1.4K"
		int size = string_to_int(args[0]);
		char buf[200] = "";
		if (size && size < 1024) {
		    sprintf(buf, "%d bytes", size);
		} else if (size < 1024*1024) {
		    sprintf(buf, "%dK", int(size/1024));
		} else if (size < 1024*1024*1024) {
		    sprintf(buf, "%dM", int(size/1024/1024));
		} else {
		    sprintf(buf, "%dG", int(size/1024/1024/1024));
		}
		value = buf;
		break;
	    }
	    case CMD_filters:
		value = filters;
		break;
	    case CMD_fmt:
		value = fmtname;
		break;
	    case CMD_freq: 
		try {
		    value = int_to_string(mset.get_termfreq(args[0]));
		} catch (...) {
		    value = int_to_string(omdb.get_termfreq(args[0]));
		}
		break;
	    case CMD_freqs:
		// for backward compatibility
		value = eval("$map{$queryterms,$_:&nbsp;$nice{$freq{$_}}}",
			     param);
		break;
            case CMD_ge:
		if (string_to_int(args[0]) >= string_to_int(args[1]))
		    value = "true";
		break;
            case CMD_gt:
		if (string_to_int(args[0]) > string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_highlight: {
		string bra, ket;
		if (args.size() > 2) {
		    bra = args[2];
		} else {
		    bra = "<strong>";
		}
		if (args.size() > 3) {
		    ket = args[3];
		} else {
		    string::const_iterator i;
		    i = find_if(bra.begin() + 2, bra.end(), p_nottag);
		    ket = "</";
		    ket += bra.substr(1, i - bra.begin() - 1);
		    ket += '>'; 
		}
		    
		value = html_highlight(args[0], args[1], bra, ket);
		break;
	    }
	    case CMD_hit:
		// 0-based mset index
		value = int_to_string(hit_no);
		break;
	    case CMD_hitlist:
#if 0
		const char *q;
		int ch;
		
		query_string = "?DB=";
		query_string += dbname;
		query_string += "&P=";
		q = raw_prob.c_str();
		while ((ch = *q++) != '\0') {
		    switch (ch) {
		     case '+':
			query_string += "%2b";
			break;
		     case '"':
			query_string += "%22";
			break;
		     case ' ':
			ch = '+';
			/* fall through */
		     default:
			query_string += ch;
		    }
		}
	        // add any boolean terms
		for (FMCI i = filter_map.begin(); i != filter_map.end(); i++) {
		    query_string += "&B=";
		    query_string += i->second;
		}
#endif
		for (hit_no = topdoc; hit_no < last; hit_no++)
		    value += print_caption(args[0], param);
		hit_no = 0;
		break;
	    case CMD_hitsperpage:
		value = int_to_string(hits_per_page);
		break;
	    case CMD_hostname: {
	        value = args[0];
		// remove URL scheme and/or path
		string::size_type i = value.find("://");
		if (i == string::npos) i = 0; else i += 3;
		value = value.substr(i, value.find('/', i) - i);
		// remove user@ or user:password@
		i = value.find('@');
		if (i != string::npos) value = value.substr(i + 1);
		// remove :port
		i = value.find(':');
		if (i != string::npos) value = value.substr(0, i);
		break;
	    }
	    case CMD_html:
	        value = html_escape(args[0]);
		break;
	    case CMD_htmlstrip:
	        value = html_strip(args[0]);
		break;
	    case CMD_id:
		// document id
		value = int_to_string(q0);
		break;
	    case CMD_if:
		if (!args[0].empty())
		    value = eval(args[1], param);
		else if (args.size() > 2)
		    value = eval(args[2], param);
		break;
	    case CMD_include:
	        value = eval_file(args[0]);
	        break;
	    case CMD_last:
		value = int_to_string(last);
		break;
	    case CMD_lastpage: {
		int l = mset.get_matches_estimated();
		if (l > 0) l = (l - 1) / hits_per_page + 1;
		value = int_to_string(l);
		break;
	    }
            case CMD_le:
		if (string_to_int(args[0]) <= string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_list: {
		if (!args[0].empty()) {
		    string pre, inter, interlast, post;
		    switch (args.size()) {
		     case 2:
			inter = interlast = args[1];
			break;
		     case 3:
			inter = args[1];
			interlast = args[2];
			break;
		     case 4:
			pre = args[1];
			inter = interlast = args[2];
			post = args[3];
			break;
		     case 5:
			pre = args[1];
			inter = args[2];
			interlast = args[3];
			post = args[4];
			break;
		    }
		    value += pre;
		    string list = args[0];
		    string::size_type split = 0, split2;
		    while ((split2 = list.find('\t', split)) != string::npos) {
			if (split) value += inter;
			value += list.substr(split, split2 - split);
			split = split2 + 1;
		    }
		    if (split) value += interlast;
		    value += list.substr(split);
		    value += post;
		}
		break;
	    }
            case CMD_lt:
		if (string_to_int(args[0]) < string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_map:
		if (!args[0].empty()) {
		    string l = args[0], pat = args[1];
		    vector<string> p(param);
		    string::size_type i = 0, j;
		    while (true) {
			j = l.find('\t', i);
			string save_loopvar;
			p[0] = l.substr(i, j - i);
			value += eval(pat, p);
			if (j == string::npos) break;
			value += '\t';
			i = j + 1;
		    }
		}
	        break;
	    case CMD_max: {
		vector<string>::const_iterator i = args.begin();
		int val = string_to_int(*i++);
		for (; i != args.end(); i++) {
		    int x = string_to_int(*i);
		    if (x > val) val = x;
	        }
		value = int_to_string(val);
		break;
	    }
	    case CMD_min: {
		vector<string>::const_iterator i = args.begin();
		int val = string_to_int(*i++);
		for (; i != args.end(); i++) {
		    int x = string_to_int(*i);
		    if (x < val) val = x;
	        }
		value = int_to_string(val);
		break;
	    }
	    case CMD_msize:
		// number of matches
		value = int_to_string(mset.get_matches_estimated());
		break;
	    case CMD_msizeexact:
		// is msize exact?
		if (mset.get_matches_lower_bound()
		    == mset.get_matches_upper_bound())
		    value = "true";
		break;
	    case CMD_mod: {
		int denom = string_to_int(args[1]);
		if (denom == 0) {
		    value = "divide by 0";
		} else {
		    value = int_to_string(string_to_int(args[0]) %
					  string_to_int(args[1]));
		}
		break;
	    }
	    case CMD_mul: {
		vector<string>::const_iterator i = args.begin();
		int total = string_to_int(*i++);
		while (i != args.end())
		    total *= string_to_int(*i++);
		value = int_to_string(total);
		break;
	    }
            case CMD_ne:
		if (args[0] != args[1]) value = "true";
		break;
	    case CMD_nice: {
		string::const_iterator i = args[0].begin();
		int len = args[0].length();
		while (len) {
		    value += *i++;
		    if (--len && len % 3 == 0) value += option["thousand"];
		}
		break;
	    }
	    case CMD_not:
		if (args[0].empty()) value = "true";
		break;
	    case CMD_opt:
		if (args.size() == 2) {
		    value = option[args[0] + "," + args[1]];
		} else {
		    value = option[args[0]];
		}
		break;
	    case CMD_or: {
		for (vector<string>::const_iterator i = args.begin();
		     i != args.end(); i++) {
		    value = eval(*i, param);
		    if (!value.empty()) break;
	        }
		break;
	    }
	    case CMD_percentage:
		// percentage score
		value = int_to_string(percent);
		break;
	    case CMD_prettyterm:
		value = pretty_term(args[0]);
		break;
	    case CMD_query:
		value = raw_prob;
		break;
	    case CMD_querydescription:
		value = query.get_description();
		break;
	    case CMD_queryterms:
		value = queryterms;
		break;
	    case CMD_range: {
		int start = string_to_int(args[0]);
		int end = string_to_int(args[1]);
	        while (start <= end) {
		    value = value + int_to_string(start);
		    if (start < end) value += '\t';
		    start++;
		}
		break;
	    }
	    case CMD_record: {
		int id = q0;
		if (!args.empty()) id = string_to_int(args[0]);
		value = omdb.get_document(id).get_data();
		break;
	    }
	    case CMD_relevant: {
		// document id if relevant; empty otherwise
		int id = q0;
		if (!args.empty()) id = string_to_int(args[0]);
		map<om_docid, bool>::iterator i = ticked.find(id);
		if (i != ticked.end()) {
		    i->second = false; // icky side-effect
		    value = int_to_string(id);
		}
		break;
	    }
	    case CMD_relevants:	{
		for (map <om_docid, bool>::const_iterator i = ticked.begin();
		     i != ticked.end(); i++) {
		    if (i->second) value += int_to_string(i->first) + '\t';
		}
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_score:
	        // Score (0 to 10)
		value = int_to_string(percent / 10);
		break;
	    case CMD_set:
		option[args[0]] = args[1];
		break;
	    case CMD_setmap: {
		string base = args[0] + ',';
		for (unsigned int i = 1; i + 1 < args.size(); i++) {
		    option[base + args[i]] = args[i + 1];
		}
		break;
	    }
	    case CMD_setrelevant: {
		string::size_type i = 0, j;
	    	while (true) {
		    j = args[0].find_first_not_of("0123456789", i);
	    	    om_docid id = atoi(args[0].substr(i, j - i).c_str());
		    if (id) {
			rset->add_document(id);
			ticked[id] = true;
		    }
	    	    if (j == string::npos) break;
		    i = j + 1;
		}
		break;
	    }
	    case CMD_slice: {
		string list = args[0], pos = args[1];
		vector<string> items;
		string::size_type i = 0, j;
		while (true) {
		    j = list.find('\t', i);
		    items.push_back(list.substr(i, j - i));
		    if (j == string::npos) break;
		    i = j + 1;
		}
		i = 0;
		bool have_added = false;
		while (true) {
		    j = pos.find('\t', i);
		    int item = string_to_int(pos.substr(i, j - i));
		    if (item >= 0 && item < items.size()) {
			if (have_added) value += '\t';
			value += items[item];
			have_added = true;
		    }
		    if (j == string::npos) break;
		    i = j + 1;
		}
	        break;
	    }
	    case CMD_sub:
		value = int_to_string(string_to_int(args[0]) -
		       		      string_to_int(args[1]));
		break;
	    case CMD_terms:
		if (enquire) {
		    // list of matching terms
		    OmTermIterator term = enquire->get_matching_terms_begin(q0);
		    while (term != enquire->get_matching_terms_end(q0)) {
			// check term was in the typed query so we ignore
			// boolean filter terms
			if (termset.find(*term) != termset.end()) 
			    value = value + *term + '\t';
			++term;
		    }

		    if (!value.empty()) value.erase(value.size() - 1);
		}
		break;
	    case CMD_thispage:
		value = int_to_string(topdoc / hits_per_page + 1);
		break;
	    case CMD_time:
		if (usec != -1) {
		    char buf[64];
		    sprintf(buf, "%d.%06d", sec, usec);
		    value = buf;
		}
		break;
	    case CMD_topdoc:
		// first document on current page of hit list (counting from 0)
		value = int_to_string(topdoc);
		break;
	    case CMD_topterms:
		if (enquire) {
		    int howmany = 16;
		    if (!args.empty()) howmany = string_to_int(args[0]);
		    if (howmany < 0) howmany = 0;

		    // List of expand terms
		    OmESet eset;
		    ExpandDeciderOmega decider(omdb);

		    if (!rset->empty()) {
			eset = enquire->get_eset(howmany * 2, *rset, &decider);
		    } else if (mset.size()) {
			// invent an rset
			OmRSet tmp;

			int c = 5;
			// FIXME: what if mset does not start at first match?
			OmMSetIterator m = mset.begin();
			for ( ; m != mset.end(); ++m) {
			    tmp.add_document(*m);
			    if (--c == 0) break;
			}

			eset = enquire->get_eset(howmany * 2, tmp, &decider);
		    }

		    OmESetIterator i;
		    set<om_termname> seen;
		    {
			if (!stemmer)
			    stemmer = new OmStem(option["no_stem"] == "true" ? "" : "english");
			// Exclude terms "similar" to those already in
			// the query
			set<om_termname>::const_iterator t;
			for (t = termset.begin(); t != termset.end(); ++t) {
			    string term = *t;
			    if (term[0] == 'R') {
				term.erase(0, 1);
				term = stemmer->stem_word(term);
			    }
			    seen.insert(term);
			}
		    }
		    MyStopper stop;
		    for (i = eset.begin(); i != eset.end(); ++i) {
			string term = *i;
			if (term[0] == 'R') {
			    term.erase(0, 1);
			    if (stop(term)) continue;
			    term = stemmer->stem_word(term);
			}
			if (seen.find(term) != seen.end()) continue;
			seen.insert(term);
			value += *i;
			value += '\t';
			if (--howmany == 0) break;
		    }
		    if (!value.empty()) value.erase(value.size() - 1);
		}
		break;
#ifdef HAVE_PCRE
	    case CMD_transform: {
		pcre *re;
		const char *error;
		int erroffset;
		int offsets[30];
		int matches;
		re = pcre_compile(args[0].c_str(), 0, &error, &erroffset, NULL);
		matches = pcre_exec(re, NULL, args[2].data(), args[2].size(),
				    0, 0, offsets, 30);
		if (matches > 0) {
		    string::const_iterator i;
		    value = args[2].substr(0, offsets[0]);
		    for (i = args[1].begin(); i != args[1].end(); ++i) {
			if (*i != '\\') {
			    value += *i;
			} else {
			    ++i;
			    if (i != args[1].end()) {
				if (*i >= '0' && *i < '0' + matches) {
				    int c = (*i - '0') * 2;
				    value.append(args[2].substr(offsets[c],
						offsets[c + 1] - offsets[c]));
				} else {
				    value += *i;
				}
			    }
			}
		    }
		    value += args[2].substr(offsets[1]);
		} else {
		    value = args[2];
		}
		break;
	    }
#endif
	    case CMD_uniq: {
		const string &list = args[0];
		if (list.empty()) break;
		string::size_type split = 0, split2;
		string prev;
		do {
		    split2 = list.find('\t', split);
		    string item = list.substr(split, split2 - split);
		    if (split == 0) {
			value = item;
		    } else if (item != prev) {
			value += '\t';
			value += item;
		    }
		    prev = item;
		    split = split2 + 1;
		} while (split2 != string::npos);
		break;
	    }
	    case CMD_unstem: {
		const string &term = args[0];
		multimap<string, string>::const_iterator i;
		i = qp.unstem.find(term);
		while (i != qp.unstem.end() && i->first == term) {
		    if (!value.empty()) value += '\t';
		    value += i->second;
		    ++i;
		}
		break;
	    }
	    case CMD_url:
	        value = percent_encode(args[0]);
		break;
	    case CMD_value: {
		om_docid id = q0;
		om_valueno value_no = string_to_int(args[0]);
		if (args.size() > 1) id = string_to_int(args[1]);
		value = omdb.get_document(id).get_value(value_no);
		break;
	    }
	    case CMD_version:
		value = "Xapian - "PACKAGE" "VERSION;
		break;
	    default: {
		args.insert(args.begin(), param[0]);
		int n = i->second->tag - CMD_MACRO;
		assert(n >= 0 && (unsigned int)n < macros.size());
	       	// throw "Unknown function `" + var + "'";
		value = eval(macros[n], args);
		break;
	    }
	}
        res += value;
    }
	     
    res += fmt.substr(p);
    return res;
}

static string
eval_file(const string &fmtfile)
{
    // don't allow ".." in format names as this would allow people to open
    // a format "../../etc/passwd" or similar
    // FIXME: make this check more exact ("foo..bar" is safe)
    // FIXME: log when this check fails
    string err;
    string::size_type i = fmtfile.find("..");
    if (i == string::npos) {
	string file = template_dir + fmtfile;
	struct stat st;
	int fd = open(file.c_str(), O_RDONLY);
	if (fd >= 0) {
	    if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode)) {
		char *blk;
		blk = (char*)malloc(st.st_size);
		if (blk) {
		    char *p = blk;
		    int len = st.st_size;
		    while (len) {
			int r = read(fd, p, len);
			if (r < 0) break;
			p += r;
			len -= r;
		    }
		    if (len == 0) {
			string fmt = string(blk, st.st_size);
			free(blk);			
			close(fd);
			vector<string> noargs;
			noargs.resize(1);
			return eval(fmt, noargs);
		    }
		    free(blk);
		}
	    }
	    err = strerror(errno);
	    close(fd);
	} else {
	    err = strerror(errno);
	}
    } else {
	err = "name contains `..'";
    }

    // FIXME: report why!
    string msg = string("Couldn't read format template `") + fmtfile + '\'';
    if (!err.empty()) msg += " (" + err + ')';
    throw msg;
}

extern string
pretty_term(const string & term)
{
    if (term.empty()) return term;
    
    if (term.length() >= 2 && term[0] == 'R')
	return char(toupper(term[1])) + term.substr(2);

    // If there's an unstemmed version in the query, use that
    // (FIXME currently uses the first of multiple forms - might be
    // "better" to pick the one with the highest termfreq)
    multimap<string, string>::const_iterator i = qp.unstem.find(term);
    if (i != qp.unstem.end()) return i->second;

    // If the term wasn't indexed unstemmed, it's probably a non-term
    // e.g. "litr" - the stem of "litre"
    // FIXME: perhaps ought to check termfreq > some threshold
    if (!omdb.term_exists('R' + term))
	return term + '.';

    if (!stemmer)
	stemmer = new OmStem(option["no_stem"] == "true" ? "" : "english");

    // The term is present unstemmed, but if it would stem further it still
    // needs protecting
    if (stemmer->stem_word(term) != term)
	return term + '.';
 
    return term;
}
    
/* return a sane (1-100) percentage value for ratio */
static int
percentage(double ratio)
{
    /* default to 100 so pure boolean queries give 100% not 0%) */
    long int percent = 100;
    percent = (long)(100.0 * ratio + 0.5);
    if (percent > 100) percent = 100;
    else if (percent < 1) percent = 1;
    return (int)percent;
}

static string
print_caption(const string &fmt, const vector<string> &param)
{
    q0 = *(mset[hit_no]);

    percent = mset.convert_to_percent(mset[hit_no]);
    collapsed = mset[hit_no].get_collapse_count();

    OmDocument doc = omdb.get_document(q0);
    string text = doc.get_data();

    // parse record
    field.clear();
    string::size_type i = 0;
    while (true) {
	string::size_type old_i = i;
	i = text.find('\n', i);
	string line = text.substr(old_i, i - old_i);
	string::size_type j = line.find('=');
	if (j != string::npos) {
	    string key = line.substr(0, j);
	    string value = field[key];
	    if (!value.empty()) value += '\t';
	    value += line.substr(j + 1);
	    field[key] = value;
	} else if (!line.empty()) {
	    // FIXME: bodge for now
	    if (field["caption"].empty()) field["caption"] = line;
	    field["sample"] += line;
	}
	if (i == string::npos) break;
	i++;
    }

    return eval(fmt, param);
}

om_doccount
do_match()
{
    cout << eval_file(fmtname);
    return mset.get_matches_estimated();
}

// run query if we haven't already
static void
ensure_match()
{
    if (done_query) return;
    
    run_query();
    done_query = true;
    last = mset.get_matches_lower_bound();
    if (last == 0) {
	// Otherwise topdoc ends up being -6 if it's non-zero!
	topdoc = 0;
    } else {
	if (topdoc >= last)
	    topdoc = ((last - 1) / hits_per_page) * hits_per_page;
	// last is the count of documents up to the end of the current page
	// (as returned by $last)
	if (topdoc + hits_per_page < last)
	    last = topdoc + hits_per_page;
    }
}
