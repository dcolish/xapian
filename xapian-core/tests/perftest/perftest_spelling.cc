#include <config.h>

#include <xapian.h>

#include <string>

#include "perftest/perftest_spelling.h"

#include "backendmanager.h"
#include "perftest.h"
#include "testrunner.h"
#include "testsuite.h"
#include "testutils.h"
#include "str.h"

using namespace std;

DEFINE_TESTCASE(spelling, spelling)
{
    Xapian::WritableDatabase db = backendmanager->get_writable_database("dbw", string());

    ifstream infile("../testdata/dict.txt");

    if (!infile) {
	logger.testcase_end();
	return false;
    }

    vector<string> lines;
    string line;

    unsigned skip = 2;
    unsigned pairs = 25;
    unsigned runsize = 100;
    unsigned sequence = 5;

    while (getline(infile, line)) {
	transform(line.begin(), line.end(), line.begin(), ::tolower);

	lines.push_back(line);

	//Skip some words to allow misspelled words in queries.
	if (lines.size() % skip == 0) {
	    db.add_spelling(line);
	    for (unsigned i = 1; i < min<size_t>(lines.size() / skip, pairs); ++i)
		db.add_spelling(lines[lines.size() - i * skip - 1], line);
	}
    }

    infile.close();
    db.commit();
    db.flush();

    map<string, string> params;
    params["runsize"] = str(runsize);

    vector<string> words;
    db.get_spelling_suggestion(lines[0]);

    logger.testcase_begin("spelling");
    logger.indexing_begin("dbw", params);

    for (size_t i = 0; i < lines.size(); i += (lines.size() / runsize)) {
	while (words.size() >= sequence)
	    words.erase(words.begin());
	words.push_back(lines[i]);

	db.get_spelling_suggestion(words);
    }

    logger.indexing_end();
    logger.testcase_end();

    return true;
}
