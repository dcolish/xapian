/* btreecheck.h: Btree checking
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#ifndef OM_HGUARD_BTREECHECK_H
#define OM_HGUARD_BTREECHECK_H

#include "btree.h"

#include <iostream>

class BtreeCheck : public Btree {
    public:
	static void check(const string & name, int opts,
			  std::ostream &out = std::cout);
    private:
	BtreeCheck(std::ostream &out_) : out(out_) {}

	void block_check(Cursor * C_, int j, int opts);
	int block_usage(const byte * p) const;
	void report_block(int m, int n, const byte * p) const;
	void report_block_full(int m, int n, const byte * p) const;
	void report_cursor(int N, const Cursor *C_) const;

	void failure(int n) const;
	void print_key(const byte * p, int c, int j) const;
	void print_tag(const byte * p, int c, int j) const;
	void print_spaces(int n) const;
	void print_bytes(int n, const byte * p) const;

	mutable std::ostream &out;
};

#define OPT_SHORT_TREE  1
#define OPT_FULL_TREE   2
#define OPT_SHOW_BITMAP 4
#define OPT_SHOW_STATS  8

#endif /* OM_HGUARD_BTREECHECK_H */
