#include "match.h"
#include "andpostlist.h"
#include "orpostlist.h"
#include "andnotpostlist.h"
#include "filterpostlist.h"

#include <algorithm>

Match::Match(IRDatabase *database)
{
    DB = database;
    max_msize = 1000;
}

bool
Match::add_pterm(const string& termname)
{
    termid id = DB->term_name_to_id(termname);

    if (!id) return false;

    PostList *postlist = DB->open_post_list(id);

    pq.push(postlist);
   
    return true;
}

bool
Match::add_bterm(const string& termname)
{
    termid id = DB->term_name_to_id(termname);

    if (!id) return false;

    PostList *postlist = DB->open_post_list(id);
    bq.push(postlist);

    return true;
}

bool
Match::add_band()
{
    if(bq.size() < 2) return false;
    PostList *left, *right;

    left = bq.top();
    bq.pop();
    right = bq.top();
    bq.pop();
    bq.push(new AndPostList(left, right, this));

    return true;
}

bool
Match::add_bor()
{
    if(bq.size() < 2) return false;
    PostList *left, *right;

    left = bq.top();
    bq.pop();
    right = bq.top();
    bq.pop();
    bq.push(new OrPostList(left, right, this));

    return true;
}

bool
Match::add_bandnot()
{
    if(bq.size() < 2) return false;
    PostList *left, *right;

    right = bq.top();
    bq.pop();
    left = bq.top();
    bq.pop();
    bq.push(new AndNotPostList(left, right, this));

    return true;
}


class MSetItem {
    public:
        weight w;
        docid id;
        MSetItem(weight w_, docid id_) { w = w_; id = id_; }
};

class MSetCmp {
    public:
        bool operator()(const MSetItem &a, const MSetItem &b) {
            return a.w > b.w;
        }
};

void
Match::recalc_maxweight()
{
    // if we don't have a merger, who the hell is telling us to recalc?
    Assert(merger != NULL);
    recalculate_maxweight = true;
}

void
Match::match()
{    
    merger = NULL;
    PostList *boolmerger = NULL;

    if (bq.size() > 1) return; // Partially constructed boolean query

    if (bq.size() == 1) {
	boolmerger = bq.top();
	// bq.top() is a boolean query merged postlist
    }

    if (!pq.empty()) {
	// build a tree balanced by the term frequencies
	// (similar to building a huffman encoding tree)
	while (true) {
	    merger = pq.top();
	    pq.pop();
	    if (pq.empty()) break;
	    // NB right is always <= left - we can use this to optimise
	    merger = new OrPostList(pq.top(), merger, this);
	    pq.pop();
	    pq.push(merger);
	}
    }

    if (boolmerger) {
	if (merger) {
	    merger = new FilterPostList(merger, boolmerger, this);
	} else {
	    merger = boolmerger;
	}
    } else if (!merger)	{
    	return;
    }

    doccount msize = 0, mtotal = 0;
    weight w_min = 0;
    vector<MSetItem> mset;
    int sorted_to = 0;

    recalculate_maxweight = true;
    weight w_max;

    // FIXME: clean all this up
    // FIXME: partial_sort?
    // FIXME: quicker to just resort whole lot than sort and merge?
    while (1) {
	if (recalculate_maxweight) {
	    recalculate_maxweight = false;
	    w_max = merger->recalc_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
	}    

	PostList *ret = merger->next(w_min);
        if (ret) {
	    delete merger;
	    merger = ret;
	    cout << "*** REPLACING ROOT\n";
	    // no need for a full recalc - we're just switching to a subtree
	    w_max = merger->get_maxweight();
	    cout << "max possible doc weight = " << w_max << endl;
            // FIXME: actually we do need to recalc but only because (e.g.)
            // an OR maybe have become and ANDMAYBE which needs its weights
            // initialising
	    w_max = merger->recalc_maxweight();
	    recalculate_maxweight = false;
	    cout << "max possible doc weight = " << w_max << " (recalculated)" << endl;
	    if (w_max < w_min) {
		cout << "*** TERMINATING EARLY" << endl;
		break;
	    }
	}

	if (merger->at_end()) break;

        mtotal++;
	
        weight w = merger->get_weight();
        
        if (w > w_min) {
	    docid id = merger->get_docid();
	    mset.push_back(MSetItem(w, id));

	    if (mset.size() == max_msize * 2) {
	        // sort new elements
	        cout << "sorting\n";		
	        stable_sort(mset.begin() + sorted_to, mset.end(), MSetCmp());
		// merge with existing elements
	        cout << "merging\n";
                if (sorted_to) {
		    inplace_merge(mset.begin(), mset.begin() + sorted_to, mset.end(), MSetCmp());
		}
	        msize = max_msize;
	        sorted_to = msize;
		// erase elements which don't make the grade
	        mset.erase(mset.begin() + sorted_to, mset.end());
	        w_min = mset.back().w;
	        cout << "mset size = " << mset.size() << endl;
	    }
	}
    }

    cout << "sorting\n";
    stable_sort(mset.begin() + sorted_to, mset.end(), MSetCmp());
    cout << "merging\n";
    if (sorted_to) {
	inplace_merge(mset.begin(), mset.begin() + sorted_to, mset.end(), MSetCmp());
    }
    msize = mset.size();
    if (max_msize < msize) {
	sorted_to = msize = max_msize;
	mset.erase(mset.begin() + sorted_to, mset.end());
	w_min = mset.back().w;
    }
    cout << "mset size = " << mset.size() << endl;

    cout << "msize = " << msize << ", mtotal = " << mtotal << endl;
    if (msize) {
	cout << "max weight in mset = " << mset[0].w
	     << ", min weight in mset = " << mset[msize - 1].w << endl;
    }
#if 0
    for (docid i = 0; i < msize; i++) {
        cout << mset[i].id << "\t" << mset[i].w << endl;
    }
#endif
    delete merger;
    merger = NULL;
}
