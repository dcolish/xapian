/* apitest.cc: tests the Xapian API
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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
#include <iostream>
#include <string>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <vector>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::max;
using std::ostream;

#include "om/om.h"
#include "testsuite.h"
#include "testutils.h"
#include "backendmanager.h"
#include "utils.h"

#include "apitest.h"

#include "api_nodb.h"
#include "api_posdb.h"
#include "api_db.h"

BackendManager backendmanager;

Xapian::Database
get_database(const string &dbname, const string &dbname2)
{
    return backendmanager.get_database(dbname, dbname2);
}

Xapian::Database
get_network_database(const string &dbname,
		     unsigned int timeout)
{
    vector<string> params;
    params.push_back("#TIMEOUT#");
    params.push_back(om_tostring(timeout));
    params.push_back(dbname);
    return backendmanager.get_database(params);
}

Xapian::WritableDatabase
get_writable_database(const string &dbname)
{
    return backendmanager.get_writable_database(dbname);
}

#define RUNTESTS(B, T) if (backend.empty() || backend == (B)) {\
    backendmanager.set_dbtype((B));\
    cout << "Running " << #T << " tests with " << (B) << " backend..." << endl;\
    result = max(result, test_driver::run(T##_tests));\
    } else (void)0

int main(int argc, char **argv)
{
    string backend;
    // allow setting from environmental variable for backward compatibility
    const char *p = getenv("OM_TEST_BACKEND");
    if (p) backend = p;
    test_driver::add_command_line_option("backend", 'b', &backend);

    test_driver::parse_command_line(argc, argv);
    
    string srcdir = test_driver::get_srcdir();

    int result = 0;

    backendmanager.set_datadir(srcdir + "/testdata/");

    RUNTESTS("void", nodb);

#ifdef MUS_BUILD_BACKEND_INMEMORY
    RUNTESTS("inmemory", db);
    RUNTESTS("inmemory", specchar);
    RUNTESTS("inmemory", writabledb);
    RUNTESTS("inmemory", localdb);
    RUNTESTS("inmemory", positionaldb);
    RUNTESTS("inmemory", localpositionaldb);
    RUNTESTS("inmemory", doclendb);
    RUNTESTS("inmemory", collfreq);
    RUNTESTS("inmemory", allterms);
    RUNTESTS("inmemory", multivalue);
#endif

#ifdef MUS_BUILD_BACKEND_QUARTZ
    RUNTESTS("quartz", db);
    RUNTESTS("quartz", specchar);
    RUNTESTS("quartz", writabledb);
    RUNTESTS("quartz", localdb);
    RUNTESTS("quartz", positionaldb);
    RUNTESTS("quartz", localpositionaldb);
    RUNTESTS("quartz", doclendb);
    RUNTESTS("quartz", collfreq);
    RUNTESTS("quartz", allterms);
    RUNTESTS("quartz", multivalue);
#endif

#ifdef MUS_BUILD_BACKEND_REMOTE
    RUNTESTS("remote", db);
    RUNTESTS("remote", specchar);
    RUNTESTS("remote", remotedb);
    RUNTESTS("remote", positionaldb);
    RUNTESTS("remote", doclendb);
    RUNTESTS("remote", multivalue);
#endif

#ifdef MUS_BUILD_BACKEND_MUSCAT36
    // need makeDA, etc tools to build da and db databases
    if (file_exists("../../makeda/makeDA")) {
	RUNTESTS("da", db);
	RUNTESTS("da", localdb);
	RUNTESTS("da", allterms);
	RUNTESTS("da", mus36);
    }
    if (file_exists("../../makeda/makeDAflimsy")) {
	RUNTESTS("daflimsy", db);
	RUNTESTS("daflimsy", localdb);
	RUNTESTS("daflimsy", allterms);
	RUNTESTS("daflimsy", mus36);
    }
    if (file_exists("../../makeda/makeDB")) {
	RUNTESTS("db", db);
	RUNTESTS("db", localdb);
	RUNTESTS("db", mus36);
    }
    if (file_exists("../../makeda/makeDBflimsy")) {
	RUNTESTS("dbflimsy", db);
	RUNTESTS("dbflimsy", localdb);
	RUNTESTS("dbflimsy", mus36);
    }
#endif

    return result;
}
