#include <stdio.h>

#include "omerror.h"
#include "database.h"
#include "database_factory.h"
#include "irdocument.h"

#include "match.h"
#include "expand.h"
#include "stem.h"
#include "rset.h"

#include <list>

int
main(int argc, char *argv[])
{
    int msize = 10;
    const char *progname = argv[0];
    list<docid> reldocs;
    list<string> dbnames;
    list<enum database_type> dbtypes;
    bool multidb = false;
    bool showmset = false;
    matchop default_op = OR;

    bool syntax_error = false;
    argv++;
    argc--;
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--msize") == 0) {
	    msize = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--db") == 0) {
	    dbnames.push_back(argv[1]);
	    dbtypes.push_back(OM_DBTYPE_DA);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--tf") == 0) {
	    dbnames.push_back(argv[1]);
	    dbtypes.push_back(OM_DBTYPE_TEXTFILE);
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--multidb") == 0) {
	    multidb = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--showmset") == 0) {
	    showmset = true;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--matchall") == 0) {
	    default_op = AND;
	    argc--;
	    argv++;
	} else if (strcmp(argv[0], "--rel") == 0) {
	    reldocs.push_back(atoi(argv[1]));
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }
	
    if (syntax_error || argc < 1) {
	cout << "Syntax: " << progname << " TERM ..." << endl;
	cout << "\t--msize MSIZE\n";
	cout << "\t--db DBDIRECTORY\n";
	cout << "\t--tf TEXTFILE\n";
	cout << "\t--rel DOCID\n";
	cout << "\t--multidb\n";
	cout << "\t--showmset\n";
	cout << "\t--matchall\n";
	exit(1);
    }

    if(!dbnames.size()) {
	dbnames.push_back("testdir");
	dbtypes.push_back(OM_DBTYPE_DA);
    }
    
    try {
	DatabaseFactory dbfactory;
	IRDatabase *database;

	if (multidb || dbnames.size() > 1) {
	    IRDatabaseGroup *multidb = dbfactory.makegroup(OM_DBGRPTYPE_MULTI);
	    list<string>::const_iterator p;
	    list<string>::const_iterator q;
	    for(p = dbnames.begin(), q = dbtypes.begin();
		p != dbnames.end();
		p++, q++) {
		multidb->open_subdatabase(dbfactory.make(*q),
					  *p, true);
	    }
	    database = multidb;
	} else {
	    database = dbfactory.make(*(dbtypes.begin())),
	    database->open(*(dbnames.begin()), true);
	}
       
	RSet rset(database);
	list<docid>::const_iterator i;
	for(i = reldocs.begin(); i != reldocs.end(); i++) {
	    rset.add_document(*i);
	}

        Match match(database);
	match.set_rset(&rset);
       
        StemEn stemmer;

	if (msize) match.set_max_msize(msize);

	bool boolean = false;
	vector<termname> prob_terms;
        for (char **p = argv; *p; p++) {
	    string term = *p;
	    if (term == "B") {
		if (!prob_terms.empty()) {
		    match.add_oplist(default_op, prob_terms);
		    prob_terms.clear();
		}
		boolean = true;
		continue;
	    } else if (term == "P") {
		boolean = false;		
		continue;
	    } else {
		if (boolean) {
		    if (term == "OR") {
			if (match.add_op(OR)) {
			    printf("Added boolean OR\n");
			} else {
			    printf("Failed to add boolean OR\n");
			}
			continue;
		    } else if (term == "NOT") {
			if (match.add_op(AND_NOT)) {
			    printf("Added boolean ANDNOT\n");
			} else {
			    printf("Failed to add boolean ANDNOT\n");
			}
			continue;
		    } else if (term == "AND") {
			if (match.add_op(AND)) {
			    printf("Added boolean AND\n");
			} else {
			    printf("Failed to add boolean AND\n");
			}
			continue;
		    } else if (term == "XOR") {
			if (match.add_op(XOR)) {
			    printf("Added boolean XOR\n");
			} else {
			    printf("Failed to add boolean XOR\n");
			}
			continue;
		    } else if (term == "ANDMAYBE") {
			if (match.add_op(AND_MAYBE)) {
			    printf("Added boolean ANDMAYBE\n");
			} else {
			    printf("Failed to add boolean ANDMAYBE\n");
			}
			continue;
		    } else if (term == "ANDNOT") {
			if (match.add_op(AND_NOT)) {
			    printf("Added boolean ANDNOT\n");
			} else {
			    printf("Failed to add boolean ANDNOT\n");
			}
			continue;
		    }
		}

		term = stemmer.stem_word(term);

		if (boolean) {
		    match.add_term(term);
		} else {
		    prob_terms.push_back(term);
		}
	    }
        }

	if (!prob_terms.empty()) {
	    match.add_oplist(default_op, prob_terms);
	    prob_terms.clear();
	}

        match.match();
	
	if (showmset) {
	    for (docid i = 0; i < match.msize; i++) {
		docid q0 = match.mset[i].did;
		IRDocument *doc = database->open_document(q0);
		IRData data = doc->get_data();
		string p = data.value;
		cout << q0 << ":[" << p << "] " << match.mset[i].wt << "\n\n";
	    }
	    cout << endl;
	}
	database->close();
	delete database;
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
