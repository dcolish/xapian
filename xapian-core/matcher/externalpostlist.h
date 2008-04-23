/** @file externalpostlist.h
 * @brief Return document ids from an external source.
 */
/* Copyright 2008 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_EXTERNALPOSTLIST_H
#define XAPIAN_INCLUDED_EXTERNALPOSTLIST_H

#include "postlist.h"

namespace Xapian {
    class PostingSource;
}

class ExternalPostList : public PostList {
  protected:
    Xapian::PostingSource * source;

    Xapian::docid current;

    /// Disallow copying.
    ExternalPostList(const ExternalPostList &);

    /// Disallow assignment.
    void operator=(const ExternalPostList &);

    PostList * update_after_advance();

  public:
    ExternalPostList(Xapian::PostingSource *source_) : source(source_) { }

    ~ExternalPostList();

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_est() const;

    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;

    Xapian::docid get_docid() const;

    Xapian::weight get_weight() const;

    Xapian::doclength get_doclength() const;

    Xapian::weight recalc_maxweight();

    PositionList * read_position_list();

    PositionList * open_position_list() const;

    PostList * next(Xapian::weight w_min);

    PostList * skip_to(Xapian::docid, Xapian::weight w_min);

    PostList * check(Xapian::docid did, Xapian::weight w_min, bool &valid);

    bool at_end() const;

    string get_description() const;
};

#endif /* XAPIAN_INCLUDED_EXTERNALPOSTLIST_H */
