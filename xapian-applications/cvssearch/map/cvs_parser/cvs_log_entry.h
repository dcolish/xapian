/************************************************************
 *
 *  cvs_log_entry.h is a class that holds an entry from the cvs log result.
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

#ifndef __CVS_LOG_ENTRY_H__
#define __CVS_LOG_ENTRY_H__

#include "virtual_iostream.h"
#include "cvs_revision.h"

class cvs_log_entry : public virtual_iostream
{
private:
    cvs_revision _revision;
    string _comments;
    string _date;
    string _author;
    string _state;
    string _lines;
    bool           _init;
protected:
    virtual istream & read(istream &);
    virtual ostream & show(ostream &) const;
    
public:
    cvs_log_entry();
    virtual ~cvs_log_entry() {}
    const cvs_revision & revision() const { return _revision; }
    const string & date()           const { return _date; }
    const string & author()         const { return _author; }
    const string & state()          const { return _state; }
    const string & lines()          const { return _lines; }
    bool is_first_entry()           const { return _init; }
};

#endif
