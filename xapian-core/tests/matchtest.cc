#include <stdio.h>

#include "multi_database.h"
#include "irdocument.h"

#include "da_database.h"
#include "match.h"
#include "stem.h"

int
main(int argc, char *argv[])
{
    int msize = 10;
    const char *progname = argv[0];
    list<string> dbnames;
    bool multidb = false;

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
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[0], "--multidb") == 0) {
	    multidb = true;
	    argc--;
	    argv++;
	} else {
	    syntax_error = true;
	    break;
	}
    }
	
    if (syntax_error || argc < 1) {
	cout << "Syntax: " << progname << " TERM ..." << endl;
	cout << "\t--msize MSIZE\n";
	cout << "\t--db DBDIRECTORY\n";
	cout << "\t--multidb\n";
	exit(1);
    }

    if(!dbnames.size()) dbnames.push_back("testdir");
    
    try {
	IRDatabase *database;

	if (multidb || dbnames.size() > 1) {
	    MultiDatabase *multidb = new MultiDatabase;
	    list<string>::const_iterator p;
	    for(p = dbnames.begin(); p != dbnames.end(); p++) {
		multidb->open_subdatabase(new DADatabase, *p, 0);
	    }
	    database = multidb;
	} else {
	    database = new DADatabase;
	    database->open(*(dbnames.begin()), 0);
	}
       
        Match match(database);
       
        StemEn stemmer;

	if (msize) match.set_max_msize(msize);

	bool boolean = false;
        for (char **p = argv; *p; p++) {
	    string term = *p;
	    if(term == "B") { boolean = true;}
	    else if(term == "P") { boolean = false;}
	    else {
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
		    }
		}

		term = stemmer.stem_word(term);
		
		if (match.add_term(term)) {
		    printf("Added term \"%s\" ok\n", term.c_str());
		} else {
		    printf("Failed to add term \"%s\"\n", term.c_str());
		}
	    }
        }

        match.match();
	
	for (docid i = 0; i < match.msize; i++) {
	    docid q0 = match.mset[i].id;
	    IRDocument *doc = database->open_document(q0);
	    IRData data = doc->get_data();
	    string p = data.value;
	    cout << q0 << ":[" << p << "]\n\n";
	}
	cout << endl;
	database->close();
	delete database;
    }
    catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}
