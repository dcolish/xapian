// simplesearch.cc

#include <om/om.h>

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require two or more
    // parameters.
    if(argc < 3) {
	cout << "usage: " << argv[0] <<
		" <path to database> <search terms>" << endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database group
	OmDatabaseGroup databases;
	vector<string> parameters;
	parameters.push_back(argv[1]);
	databases.add_database("sleepycat", parameters);

	// Start an enquire session
	OmEnquire enquire(databases);

	// Prepare the query terms
	vector<om_termname> queryterms;
	for (int optpos = 2; optpos < argc; optpos++) {
	    queryterms.push_back(argv[optpos]);
	}

	// Build the query object
	OmQuery query(OM_MOP_OR, queryterms.begin(), queryterms.end());
	cout << "Performing query `" << query.get_description() << "'" << endl;

	// Give the query object to the enquire session
	enquire.set_query(query);

	// Get the top 10 results of the query
	OmMSet matches = enquire.get_mset(0, 10);

	// Display the results
	cout << matches.items.size() << " results found" << endl;

	for (vector<OmMSetItem>::const_iterator i = matches.items.begin();
	     i != matches.items.end();
	     i++) {
	    OmDocument doc = enquire.get_doc(*i);
	    cout << "Document ID " << i->did << "\t" <<
		    matches.convert_to_percent(*i) << "% [" <<
		    doc.get_data().value << "]" << endl;
	}
    }
    catch(OmError &error) {
	cout << "Exception: "  << error.get_msg() << endl;
    }
}
