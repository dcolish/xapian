/* btree_base.h: Btree base file implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_BTREE_BASE_H
#define OM_HGUARD_BTREE_BASE_H

#include "btree_types.h"
#include "btree_util.h"

class Btree_base {
    public:
	/** Initialise a Btree_Base object with all zero fields.
	 */
	Btree_base();

	/** Read a base file from disk into a structure in memory.
	 *
	 *  @param name			The base filename name
	 *  @param ch			The suffix
	 */
	Btree_base(const string &name_, char ch);

	/** Copy constructor */
	Btree_base(const Btree_base &other);

	/** Destructor - frees resources. */
	~Btree_base();

	/** Read values from a base file.
	 *
	 *  @param name		The base of the filename
	 *  @param ch		The suffix
	 *  @param err_msg	An error string which will be appended
	 *  			to for some errors instead of throwing
	 *  			an exception.
	 *
	 *  @return	true if the read succeeded, or false otherwise.
	 */
	bool read(const string &name, char ch, string &err_msg);

	uint4 get_revision();
	uint4 get_block_size();
	int4 get_root();
	int4 get_level();
	int4 get_bit_map_size();
	int4 get_item_count();
	int4 get_last_block();
	bool get_have_fakeroot();
	bool get_sequential();

	void set_revision(uint4 revision_);
	void set_block_size(uint4 block_size_);
	void set_root(int4 root_);
	void set_level(int4 level_);
	void set_item_count(int4 item_count_);
	void set_have_fakeroot(bool have_fakeroot_);
	void set_sequential(bool sequential_);

	/** Write the btree base file to disk. */
	void write_to_file(const string &filename);

	/* Methods dealing with the bitmap */
	/** true iff block n was free at the start of the transaction on
	 *  the B-tree.
	 */
	bool block_free_at_start(int4 n) const;

	void free_block(int4 n);

	int next_free_block();

	bool block_free_now(int4 n);

	void calculate_last_block();

	/* Only used with fake root blocks */
	void clear_bit_map();

	/* Used by Btree::check() */
	bool is_empty() const;

	void swap(Btree_base &other);

    private:
	/** private assignment operator - you probably want swap() instead */
	void operator=(const Btree_base &other);

	void extend_bit_map();

	/** Do most of the error handling from unpack_uint() */
	bool do_unpack_uint(const char **start, const char *end,
			    uint4 *dest, string &err_msg,
			    const string &basename,
			    const char *varname);

	/** Do most of the error handling from unpack_uint(),
	 *  with conversion to signed int.
	 */
	bool do_unpack_int(const char **start, const char *end,
			   int4 *dest, string &err_msg,
			   const string &basename,
			   const char *varname);

	/* Decoded values from the base file follow */
	uint4 revision;
	uint4 block_size;
	int4 root;
	int4 level;
	int4 bit_map_size;
	int4 item_count;
	int4 last_block;
	bool have_fakeroot;
	bool sequential;

	/* Data related to the bitmap */
	/** byte offset into the bit map below which there
	   are no free blocks */
	int bit_map_low;

	/** the initial state of the bit map of blocks: 1 means in
	   use, 0 means free */
	byte *bit_map0;

	/** the current state of the bit map of blocks */
	byte *bit_map;
};

#endif /* OM_HGUARD_BTREE_BASE_H */
