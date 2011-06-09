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

#include <iostream>

using namespace std;

DEFINE_TESTCASE(spelling, spelling)
{
	Xapian::WritableDatabase db = backendmanager->get_writable_database("dbw", string());

	ifstream infile("../testdata/dict.txt");

	if (!infile)
	{
		logger.testcase_end();
		return false;
	}

	vector<string> lines;
	string line;

	while (getline(infile, line))
	{
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		lines.push_back(line);
		db.add_spelling(line);
	}

	infile.close();
	db.commit();
	db.flush();

	unsigned int runsize = 100;

	std::map<std::string, std::string> params;
	params["runsize"] = str(runsize);

	db.get_spelling_suggestion(lines[0], 2);

	logger.testcase_begin("spelling");

	logger.indexing_begin("dbw", params);

	for (size_t i = 0; i < lines.size(); i += (lines.size() / runsize))
	{
		db.get_spelling_suggestion(lines[i], 2);
	}

	logger.indexing_end();

	logger.testcase_end();

	return true;
}
