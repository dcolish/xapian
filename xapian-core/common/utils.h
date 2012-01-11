/* utils.h: Various useful utilities
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005,2006,2007,2009,2012 Olly Betts
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

#ifndef OM_HGUARD_UTILS_H
#define OM_HGUARD_UTILS_H

#include <string>

/** Remove a directory, and its contents.
 *
 *  If dirname doesn't refer to a file or directory, no error is generated.
 *
 *  Note - this doesn't currently cope with directories which contain
 *  subdirectories.
 */
void removedir(const std::string &dirname);

namespace Xapian {
    namespace Internal {
	bool within_DBL_EPSILON(double a, double b);
    }
}

#endif /* OM_HGUARD_UTILS_H */
