/************************************************************
 *
 *  html_writer.cpp implementation.
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

#include "html_writer.h"

ostream & 
html_writer::show (ostream & os) const
{
    return last(write(init(os)));
}

ostream & html_writer::init (ostream & os) const
{
    os << "<HTML>" << endl;
    os << "<HEAD>" << endl;
    os << "<TITLE>" << _title << "</TITLE>" << endl;
    style(os);
    os << "</HEAD>" << endl;
    os << "<BODY>" << endl;
    return os;
}

ostream & html_writer::last (ostream & os) const
{
    os << "</BODY>" << endl;
    os << "</HTML>" << endl;
    return os;
}
