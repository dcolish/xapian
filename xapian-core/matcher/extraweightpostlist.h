/* extraweightpostlist.h: add on extra weight contribution
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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

#ifndef OM_HGUARD_EXTRAWEIGHTPOSTLIST_H
#define OM_HGUARD_EXTRAWEIGHTPOSTLIST_H

/// A postlist which adds on an extra weight contribution
class ExtraWeightPostList : public PostList {
    private:
	PostList * pl;
	Xapian::Weight * wt;
	MultiMatch * matcher;
	om_weight max_weight;

    public:
	om_doccount get_termfreq_max() const { return pl->get_termfreq_max(); }
	om_doccount get_termfreq_min() const { return pl->get_termfreq_min(); }
	om_doccount get_termfreq_est() const { return pl->get_termfreq_est(); }

	om_docid  get_docid() const { return pl->get_docid(); }

	om_weight get_weight() const {
	    return pl->get_weight() + wt->get_sumextra(pl->get_doclength());
	}

	om_weight get_maxweight() const {
	    return pl->get_maxweight() + max_weight;
	}

        om_weight recalc_maxweight() {
	    return pl->recalc_maxweight() + max_weight;
	}

	PostList *next(om_weight w_min) {
	    PostList *p = pl->next(w_min - max_weight);
	    if (p) {
		delete pl;
		pl = p;
		if (matcher) matcher->recalc_maxweight();
	    }
	    return NULL;
	}
	    
	PostList *skip_to(om_docid did, om_weight w_min) {
	    PostList *p = pl->skip_to(did, w_min - max_weight);
	    if (p) {
		delete pl;
		pl = p;
		if (matcher) matcher->recalc_maxweight();
	    }
	    return NULL;
	}

	bool at_end() const { return pl->at_end(); }

	std::string get_description() const {
	    return "( ExtraWeight " + pl->get_description() + " )";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const {
	    return pl->get_doclength();
	}

	virtual PositionList * read_position_list() {
	    return pl->read_position_list();
	}

	virtual PositionList * open_position_list() const {
	    return pl->open_position_list();
	}

        ExtraWeightPostList(PostList * pl_, Xapian::Weight *wt_, MultiMatch *matcher_)
	    : pl(pl_), wt(wt_), matcher(matcher_), max_weight(wt->get_maxextra())
	{ }

	~ExtraWeightPostList() {
	    delete pl;
	    delete wt;
	}
};

#endif /* OM_HGUARD_EXTRAWEIGHTPOSTLIST_H */
