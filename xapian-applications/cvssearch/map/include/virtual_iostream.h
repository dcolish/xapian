/************************************************************
 *
 *  virtual_iostream.h allow derived classes to be read from/
 *  write to streams.
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
 *
 *  Usage:
 *
 *  any derived class must override the virtual functions
 *  read(istream &) and show(ostream &).
 * 
 *  the following is possible if class someobject derives
 *  virtual_iostream:
 *
 *  some_object a;
 *  cin >> a;
 *  ...
 *  cout << a;
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __VIRTUAL_IOSTREAM_H__
#define __VIRTUAL_IOSTREAM_H__

#include "virtual_istream.h"
#include "virtual_ostream.h"

class virtual_iostream : public virtual_istream, public virtual_ostream
{
};

#endif
