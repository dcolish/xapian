/* scriptindex.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Sam Liddicott
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

#include <om/om.h>

#include <algorithm>
#include <fstream>
// Not in GCC 2.95.2: #include <limits>
#include <map>
#include <string>
#include <vector>
#include <list>

#include <unistd.h>

#include "htmlparse.h"

using namespace std;

static const char *argv0;
static bool verbose;
static int addcount;
static int repcount;
static int delcount;

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;

class MyHtmlParser : public HtmlParser {
    public:
    	string dump;
	void process_text(const string &text) {
	    // some tags are meaningful mid-word so this is a little
	    // simplistic
	    if (!dump.empty()) dump += ' ';
	    dump += text;
	}
	void closing_tag(const string &tag) {
	    if (tag == "title") dump = "";
	}
};

inline static bool 
p_space(unsigned int c)
{
    return isspace(c);
}

inline static bool 
p_notspace(unsigned int c)
{
    return !isspace(c);
}

inline static bool
p_notalpha(unsigned int c)
{
    return !isalpha(c);
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

inline static bool
p_notplusminus(unsigned int c)
{
    return c != '+' && c != '-';
}

class Action {
public:
    typedef enum {
	BAD, NEW,
	BOOLEAN, DATE, FIELD, INDEX, INDEXNOPOS, LOWER,
	TRUNCATE, UNHTML, UNIQUE, VALUE, WEIGHT
    } type;
private:
    type action;
    int num_arg;
    string string_arg;
public:
    Action(type action_, string arg = "") : action(action_), string_arg(arg) {
	num_arg = atoi(string_arg.c_str());
    }
    type get_action() const { return action; }
    int get_num_arg() const { return num_arg; }
    string get_string_arg() const { return string_arg; }
};

static map<string, vector<Action> > index_spec;

static void
parse_index_script(const string &filename)
{
    ifstream script(filename.c_str());
    string line;
    while (getline(script, line)) {
	vector<string> fields;
	vector<Action> actions;
	string::const_iterator i, j;
	const string &s = line;
	i = find_if(s.begin(), s.end(), p_notspace);
	if (i == s.end() || *i == '#') continue;
	while (true) {
	    if (!isalnum(*i)) {
		cout << argv0 << ": field name must start with alphanumeric"
		     << endl;
		exit(1);
	    }
	    j = find_if(i, s.end(), p_notalnum);
	    fields.push_back(string(i, j));
	    i = find_if(j, s.end(), p_notspace);
	    if (i == s.end()) break;
	    if (*i == ':') {
		++i;
		i = find_if(i, s.end(), p_notspace);
		break;
	    }
	}
	j = i;
	while (j != s.end()) {
	    i = find_if(j, s.end(), p_notalpha);
	    string action = s.substr(j - s.begin(), i - j);
	    Action::type code = Action::BAD;
	    enum {NO, OPT, YES} arg = NO;
	    if (!action.empty()) {
		switch (action[0]) {
		    case 'b':
			if (action == "boolean") {
			    code = Action::BOOLEAN;  
			    arg = OPT;
			}
			break;
		    case 'd':
			if (action == "date") {
			    code = Action::DATE;  
			    arg = YES;
			}
			break;
		    case 'f':
			if (action == "field") {
			    code = Action::FIELD;  
			    arg = OPT;
			}
			break;
		    case 'i':
			if (action == "index") {
			    code = Action::INDEX;
			    arg = OPT;
			} else if (action == "indexnopos") {
			    code = Action::INDEXNOPOS;
			    arg = OPT;
			}
			break;
		    case 'l':
			if (action == "lower") {
			    code = Action::LOWER;  
			}
			break;
		    case 't':
			if (action == "truncate") {
			    code = Action::TRUNCATE;  
			    arg = YES;
			}
			break;
		    case 'u':
			if (action == "unhtml") {
			    code = Action::UNHTML;  
			} else if (action == "unique") {
			    code = Action::UNIQUE;  
			    arg = YES; // to enable hash unique: OPT;
			}
			break;
		    case 'v':
			if (action == "value") {
			    code = Action::VALUE;  
			    arg = YES;
			}
			break;
		    case 'w':
			if (action == "weight") {
			    code = Action::WEIGHT;  
			    arg = YES;
			}
			break;
		}
	    }
	    if (code == Action::BAD) {
		cout << argv0 << ": unknown index action `" << action
		     << "'" << endl;
		exit(1);
	    }
	    i = find_if(i, s.end(), p_notspace);

	    if (i != s.end() && *i == '=') {
		if (arg == NO) {
		    cout << argv0 << ": index action `" << action
			 << "' doesn't take an argument" << endl;
		    exit(1);
		}
		++i;
		j = find_if(i, s.end(), p_notspace);
		i = find_if(j, s.end(), p_space);
		string arg = string(j, i);
		if (code == Action::INDEX && arg == "nopos") {
		    // index used to take an optional argument which could
		    // be "nopos" to mean the same that indexnopos now does.
		    // translate this to allow older scripts to work (this
		    // is safe to do since nopos isn't a sane prefix value)
		    actions.push_back(Action(Action::INDEXNOPOS));
		} else {
		    actions.push_back(Action(code, string(j, i)));
		}
		i = find_if(i, s.end(), p_notspace);
	    } else {
		if (arg == YES) {
		    cout << argv0 << ": index action `" << action
			 << "' must have an argument" << endl;
		    exit(1);
		}
		actions.push_back(Action(code));
	    }
	    j = i;
	}
	vector<string>::const_iterator field;
	for (field = fields.begin(); field != fields.end(); ++field) {
	    vector<Action> &v = index_spec[*field];
	    if (v.empty()) {
		v = actions;
	    } else {
		v.push_back(Action(Action::NEW));
		v.insert(v.end(), actions.begin(), actions.end());
	    }
	}
    }
}

static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while (i != term.end()) {
        *i = tolower(*i);
        i++;
    }
} 

static void
lowercase_string(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
        *i = tolower(*i);
        i++;
    }
} 

// FIXME: this function is almost identical to one in omindex.cc...
static om_termpos
index_text(const string &s, OmDocument &doc, OmStem &stemmer,
	   om_termcount wdfinc, const string &prefix,
	   om_termpos pos = static_cast<om_termpos>(-1)
	   // Not in GCC 2.95.2 numeric_limits<om_termpos>::max()
	   )
{
    string::const_iterator i, j = s.begin(), k;
    while ((i = find_if(j, s.end(), p_alnum)) != s.end()) {
	om_termname term;
	k = i;
	if (isupper(*k)) {
	    j = k;
	    term = *j;
	    while (++j != s.end() && *j == '.' &&
		   ++j != s.end() && isupper(*j)) {
		term += *j;
	    } 
	    if (term.length() < 2 || (j != s.end() && isalnum(*j))) {
		term = "";
	    }
	}
	if (term.empty()) {
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
	}

	if (term.length() <= MAX_PROB_TERM_LENGTH) {
	    lowercase_term(term);
	    if (isupper(*i) || isdigit(*i)) {
		if (pos != static_cast<om_termpos>(-1)
			// Not in GCC 2.95.2 numeric_limits<om_termpos>::max()
		   ) {
		    doc.add_posting(prefix + 'R' + term, pos, wdfinc);
		} else {
		    doc.add_term_nopos(prefix + 'R' + term, wdfinc);
		}
	    }

	    term = stemmer.stem_word(term);
	    if (pos != static_cast<om_termpos>(-1)
		    // Not in GCC 2.95.2 numeric_limits<om_termpos>::max()
	       ) {
		doc.add_posting(prefix + term, pos++, wdfinc);
	    } else {
		doc.add_term_nopos(prefix + term, wdfinc);
	    }
	}
    }
    return pos;
}                           

#if 0
static unsigned int
hash(const string &s)
{
    unsigned int h = 1;
    for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
	h += (h << 5) + static_cast<unsigned int>(*i);
    }
    return h;
}
#endif

static bool
index_file(istream &stream, OmWritableDatabase &database, OmStem &stemmer)
{
    string line;
    if (!getline(stream, line)) {
	// empty file !?!
	return true;
    }
    
    while (true) {
	OmDocument doc;
	om_docid docid = 0;
	om_termpos wordcount = 0;
	map<string, list<string> > fields;
	bool seen_content = 0;
	while (true) {
	    om_termcount weight = 1;
	    // Cope with files from MS Windows (\r\n end of lines)
	    if (line[line.size() - 1] == '\r') line.resize(line.size() - 1);

	    string::size_type eq = line.find('=');
	    string field = line.substr(0, eq);
	    string value = line.substr(eq + 1);
	    while (getline(stream, line) && !line.empty() && line[0] == '=') {
		// Cope with files from MS Windows (\r\n end of lines)
		if (line[line.size() - 1] == '\r') line.resize(line.size() - 1);
		value += '\n' + line.substr(1);
	    }

	    vector<Action> &v = index_spec[field];
	    string old_value = value;
	    vector<Action>::const_iterator i;
	    bool this_field_is_content = 1;
	    for (i = v.begin(); i != v.end(); ++i) {
		switch (i->get_action()) {
		    case Action::BAD:
			abort();
		    case Action::NEW:
			value = old_value;
			break;
		    case Action::FIELD:
			if (!value.empty()) {
			    string f = i->get_string_arg();
			    if (f.empty()) f = field;
			    // replace newlines with spaces
			    string s = value;
			    string::size_type i = 0;
			    while ((i = s.find('\n', i)) != string::npos)
				s[i] = ' ';
			    fields[f].push_back(s);
			}
			break;
		    case Action::INDEX:
			wordcount = index_text(value, doc, stemmer, weight,
					       i->get_string_arg(), wordcount);
			break;
		    case Action::INDEXNOPOS:
			// No positional information so phrase searching
			// won't work.  However, the database will use much
			// less diskspace.
			index_text(value, doc, stemmer, weight,
				   i->get_string_arg());	
			break;
		    case Action::BOOLEAN:
			doc.add_term_nopos(i->get_string_arg() + value);
			break;
		    case Action::LOWER:
			lowercase_string(value); 
			break;
		    case Action::TRUNCATE: {
			string::size_type l = i->get_num_arg();
			if (l < value.size()) {
			    while (l > 0 && !isspace(value[l - 1])) --l;
			    while (l > 0 && isspace(value[l - 1])) --l;

			    // If the first word is too long, just truncate it
			    if (l == 0) l = i->get_num_arg();

			    value = value.substr(0, l);
			}
			break;
		    }
		    case Action::UNHTML: {
			MyHtmlParser p;
			p.parse_html(value);
			value = p.dump;
			break;
		    }
		    case Action::UNIQUE: {
			// Ensure that the value of this field is unique.
			// If a record already exists with the same value,
			// it will be replaced with the new record.

			// Unique fields aren't considered content - if
			// there are no other fields in the document, the
			// document is to be deleted.
			this_field_is_content = 0;

			// Argument is the prefix to add to the field value
			// to get the unique term.
			string t = i->get_string_arg();
#if 0
			if (t.empty()) {
			    // Generate the docid from a hash of the value
			    // - quicker than performing a lookup in the
			    // database and the probability of a collision
			    // is very low unless the number of documents
			    // is vast.
			    // 
			    // FIXME: this doesn't current work since
			    // replace_document() requires that the document id
			    // already exists in the DB...
			    docid = hash(value);
			    if (docid == 0) docid = 1;
			    break;
			}
#endif
			t += value;
again:
			try {
			    // FIXME: seems to tickle a quartz problem
			    OmPostListIterator p = database.postlist_begin(t);
			    if (!(p == database.postlist_end(t))) {
				docid = *p;
			    }
			} catch (const OmError &e) {
			    // Hmm, what happened?
			    cout << "Caught exception in UNIQUE!" << endl;
			    cout << "E: " << e.get_msg() << endl;
			    database.flush();
			    database.reopen();
			    goto again;
			}
			break;
		    }
		    case Action::VALUE:
			if (!value.empty())
			    doc.add_value(i->get_num_arg(), value);
			break;
		    case Action::WEIGHT:
			weight = i->get_num_arg();
			break;
		    case Action::DATE: {
			string type = i->get_string_arg();
			switch (type[0]) {
			    case 'u':
				if (type == "unix") {
				    char buf[9];
				    struct tm *tm;
				    time_t t = atoi(value.c_str());
				    tm = localtime(&t);
				    sprintf(buf, "%04d%02d%02d",
					    tm->tm_year + 1900, tm->tm_mon + 1,
					    tm->tm_mday);
				    value = buf;
				    break;
				}
				value = "";
				break;
			    case 'y':
				if (type == "yyyymmdd") {
				    if (value.length() == 8) break;
				}
				value = "";
				break;
			    default:
				value = "";
				break;
			}
			if (value.empty()) break;
			// Date (YYYYMMDD)
			doc.add_term_nopos("D" + value);
#if 0 // "Weak" terms aren't currently used by omega
			value.resize(7);
			if (value[6] == '3') value[6] = '2';
			// "Weak" - 10ish day interval
			newdocument.add_term_nopos("W" + value);
#endif
			value.resize(6);
			// Month (YYYYMM)
			doc.add_term_nopos("M" + value);
			value.resize(4);
			// Year (YYYY)
			doc.add_term_nopos("Y" + value);
			break;
		    }
		}
	    }
	    if (this_field_is_content) seen_content = 1;
	    if (line.empty() || line == "\r") break;
	}

	// If we havn't seen any fields (other than unique identifiers)
	// the document is to be deleted.
	if (!seen_content) {
	    if (docid) {
		database.delete_document(docid);
		if (verbose) cout << "Del: " << docid << endl;
		delcount ++;
	    }
	} else {
	    string data;
	    map<string, list<string> >::const_iterator i;
	    for (i = fields.begin(); i != fields.end(); ++i) {
		list<string>::const_iterator j;
		for (j = i->second.begin(); j != i->second.end(); j++) {
		    data += i->first;
		    data += '=';
		    data += *j;
		    data += '\n';
		}
	    }

	    // Put the data in the document
	    doc.set_data(data);

	    // Add the document to the database
	    if (docid) {
		try {
		    database.replace_document(docid, doc);
		    if (verbose) cout << "Replace: " << docid << endl;
		    repcount ++;
		} catch (const OmError &e) {
		    cout << "E: " << e.get_msg() << endl;
		    // Possibly the document was deleted by another
		    // process in the meantime...?
		    docid = database.add_document(doc);
		    cout << "Replace failed, adding as new: " << docid << endl;
		}
	    } else {
		docid = database.add_document(doc);
		if (verbose) cout << "Add: " << docid << endl;
		addcount ++;
	    }
	}
	if (stream.eof() || !getline(stream, line)) break;
    }

    //cout << "Flushing: " << endl;
    //database.flush();

    return true;
}

int
main(int argc, char **argv)
{
    // If update_db is true, the database will be updated rather than created.
    bool update_db = 0;
    bool quiet = 0;
    verbose = 0;

    argv0 = argv[0];

    // Simplest possible options parsing: we require two or more parameters,
    // preceded by options.

    while (argc >= 3) {
	if (argv[1][0] != '-') break;

	switch (argv[1][1]) {
	    case 'u':
		update_db = 1;
		break;
	    case 'q':
		quiet = 1;
		break;
	    case 'v':
		verbose = 1;
		break;
	    default:
		argc = 1;
		break;
	}
	argv++;
	argc--;
    }

    if (argc < 3) {
	cout << "Usage: " << argv0
	     << " [-u] [-q] [-v] "
	     << " <path to xapian database> <indexer script> [<filename>]..."
	     << endl
	     << "Creates a new database containing the data given by the list "
	     << "of files."
	     << endl
	     << "The -q (quiet) option suppresses log messages."
	     << endl
	     << "The -v (verbose) option generates messages about all actions."
	     << endl
	     << "The -u option causes the database to be updated rather than "
	     << "created anew."
	     << endl;
	exit(1);
    }
    
    parse_index_script(argv[2]);
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database
	// Sleep and retry if we get an OmDatabaseLockError - this just means
	// that another process is updating the database
	OmWritableDatabase database;
	while (true) {
	    try {
		if (update_db) {
		    database = OmAuto__open(argv[1], OM_DB_CREATE_OR_OPEN);
		} else {
		    database = OmAuto__open(argv[1], OM_DB_CREATE_OR_OVERWRITE);
		}
		break;
	    } catch (const OmDatabaseLockError &error) {
		cout << "Database locked ... retrying" << endl;
		sleep(1);
	    }
	}
	OmStem stemmer("english"); 

	addcount = 0;
	repcount = 0;
	delcount = 0;

	// Read file/s
	if (argc == 3) {
	    // Read from stdin
	    index_file(cin, database, stemmer);
	} else {
	    for (int i = 3; i < argc; ++i) {
		ifstream stream(argv[i]);
		if (stream) {
		    index_file(stream, database, stemmer);
		} else {
		    cout << "Can't open file " << argv[i] << endl;
		}
	    }
	}

	if (verbose) cout << "Flushing: " << endl;
	database.flush();

	cout << "records (added, replaced, deleted) = (" << addcount <<
		", " << repcount << ", " << delcount << ")" << endl;
    } catch (const OmError &error) {
	cout << "Exception: "  << error.get_msg() << endl;
	exit(1);
    } catch (...) {
	cout << "Unknown Exception" << endl;
	exit(1);
    }
}
