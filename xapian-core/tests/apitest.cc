/* apitest.cc: tests the Xapian API
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2006,2007,2008 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "apitest.h"

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

#include "api_all.h"
#include "backendmanager.h"
#include "testrunner.h"
#include "testsuite.h"
#include <xapian.h>

using namespace std;

const char * get_dbtype()
{
    return backendmanager->get_dbtype();
}

Xapian::Database
get_database(const string &dbname)
{
    return backendmanager->get_database(dbname);
}

Xapian::Database
get_database(const string &dbname, const string &dbname2)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    dbnames.push_back(dbname2);
    return backendmanager->get_database(dbnames);
}

Xapian::WritableDatabase
get_writable_database(const string &dbname)
{
    return backendmanager->get_writable_database("dbw", dbname);
}

Xapian::WritableDatabase
get_named_writable_database(const std::string &name, const std::string &source)
{
   return backendmanager->get_writable_database("dbw__" + name, source);
}

std::string
get_named_writable_database_path(const std::string &name)
{
   return backendmanager->get_writable_database_path("dbw__" + name);
}

Xapian::Database
get_remote_database(const string &dbname, unsigned int timeout)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return backendmanager->get_remote_database(dbnames, timeout);
}

Xapian::Database
get_writable_database_as_database()
{
    return backendmanager->get_writable_database_as_database();
}

Xapian::WritableDatabase
get_writable_database_again()
{
    return backendmanager->get_writable_database_again();
}

class ApiTestRunner : public TestRunner
{
  public:
    int run() const {
	int result = 0;
#include "api_collated.h"
	return result;
    }
};

int main(int argc, char **argv)
{
    ApiTestRunner runner;
    return runner.run_tests(argc, argv);
}
