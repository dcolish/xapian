/************************************************************
 *
 *  cvs_db_file.h the class to manipulate a set of databases.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __CVS_DB_FILE_H__
#define __CVS_DB_FILE_H__

#include "cvs_comment_db.h"
#include "cvs_comment_id_db.h"
#include "cvs_db.h"
#include "cvs_filename_db.h"
#include "cvs_line_db.h"
#include "cvs_revision_db.h"
#include "cvs_file_revision_db.h"
#include "cvs_revision_line_db.h"
#include "cvs_diff_db.h"

#include <set>
#include <string>
#include <vector>
using std::set;
using std::string;
using std::vector;

class cvs_db_file
{
protected:
    cvs_comment_db _comment_db;
    cvs_comment_id_db _comment_id_db;
    cvs_filename_db _filename_db;
    cvs_line_db _line_db;
    cvs_revision_db _revision_db;
    cvs_file_revision_db _file_revision_db;
    cvs_diff_db _diff_db;
    cvs_revision_line_db _revision_line_db;
    const string _database_name;
    bool _read_only;

public:
    cvs_db_file(const string & database_name, bool read_only) : _database_name(database_name), _read_only(read_only) {}
    int get_comment(unsigned int file_id, const string & revision,       string & comment);
    int get_revision         (unsigned int file_id, unsigned int line, set<string, cvs_revision_less> & revisions);
    int get_line             (unsigned int file_id, const string & revision, set<unsigned int> &lines);
    int get_revision_comment (unsigned int file_id, set<string, cvs_revision_less> & revisions, vector<string> & comments);
    int get_filename         (unsigned int file_id, string & filename);
    int get_line_mapping     (unsigned int file_id, const string & revision, unsigned int line_new, unsigned int & line_old);
    int get_diff             (unsigned int fileId, const string & revision, 
                              vector<unsigned int> & s1, vector<unsigned int> & s2, 
                              vector<unsigned int> & d1, vector<unsigned int> & d2, vector<char> & type);

    int put_revision         (unsigned int file_id, const string & revision);
    int put_comment          (unsigned int file_id, const string & revision, const string & comment);
    int put_mapping          (unsigned int file_id, const string & revision, unsigned int line);
    int put_filename         (unsigned int & file_id, const string & filename);
    int put_line_mapping     (unsigned int file_id, const string & revision, unsigned int line_new, unsigned int line_old);
    int put_diff             (unsigned int fileId, const string & revision, 
                              const vector<unsigned int> & s1, const vector<unsigned int> & s2, 
                              const vector<unsigned int> & d1, const vector<unsigned int> & d2, const vector<char> & type);

    int sync();
    virtual ~cvs_db_file();

    int clear();
};


#endif
