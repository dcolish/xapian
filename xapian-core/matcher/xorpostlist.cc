#include "xorpostlist.h"
#include "andnotpostlist.h"

// for XOR we just pass w_min through unchanged since both branches matching
// doesn't cause a match

inline PostList *
XorPostList::advance_to_next_match(weight w_min)
{
    while (rhead == lhead) {
	handle_prune(l, l->next(w_min));
	handle_prune(r, r->next(w_min));
	if (l->at_end()) {
	    if (r->at_end()) {
		lhead = 0;
		return NULL;
	    }
	    PostList *ret = r;
	    r = NULL;
	    return ret;
	}
	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	lhead = l->get_docid();
	rhead = r->get_docid();
    }
    return NULL;
}

XorPostList::XorPostList(PostList *left, PostList *right, Match *root_)
{
    root = root_;
    l = left;
    r = right;
    lhead = rhead = 0;
}

PostList *
XorPostList::next(weight w_min)
{
    if (w_min > minmax) {
	// we can replace the XOR with another operator (or run dry)
	PostList *ret;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("XOR drops below w_min" << endl);
		// neither side is weighty enough, so run dry
		lhead = 0;
		return NULL;
	    }
	    DebugMsg("XOR -> AND NOT (1)" << endl);
	    ret = new AndNotPostList(r, l, root);
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("XOR -> AND NOT (2)" << endl);
	    ret = new AndNotPostList(l, r, root);
	}
		
	PostList *ret2 = ret->next(w_min);
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry = false;
    bool rnext = false;

    if (lhead <= rhead) {
	// lhead == rhead should only happen on first next
        if (lhead == rhead) rnext = true;
        handle_prune(l, l->next(w_min));
	if (l->at_end()) ldry = true;
    } else {
	rnext = true;
    }
    
    if (rnext) {
        handle_prune(r, r->next(w_min));
        if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    if (ldry) {
	PostList *ret = r;
	r = NULL;
	return ret;
    }

    lhead = l->get_docid();
    return advance_to_next_match(w_min);
}

PostList *
XorPostList::skip_to(docid id, weight w_min)
{
    if (w_min > minmax) {
	// we can replace the XOR with another operator (or run dry)
	PostList *ret, *ret2;
	if (w_min > lmax) {
	    if (w_min > rmax) {
		DebugMsg("XOR drops below w_min" << endl);
		// neither side is weighty enough, so run dry
		lhead = 0;
		return NULL;
	    }
	    DebugMsg("XOR -> AND NOT (in skip_to) (1)" << endl);
	    AndNotPostList *ret3 = new AndNotPostList(r, l, root);
	    id = max(id, rhead);
	    ret2 = ret3->sync_and_skip_to(id, w_min, rhead, lhead);
	    ret = ret3;
	} else {
	    // w_min > rmax since w_min > minmax but not (w_min > lmax)
	    Assert(w_min > rmax);
	    DebugMsg("XOR -> AND NOT (in skip_to) (2)" << endl);
	    AndNotPostList *ret3 = new AndNotPostList(l, r, root);
	    id = max(id, lhead);
	    ret2 = ret3->sync_and_skip_to(id, w_min, lhead, rhead);
	    ret = ret3;
	}
		
	l = r = NULL;
	if (ret2) {
	    delete ret;
	    ret = ret2;
	}
	return ret;
    }

    bool ldry = false;
    if (lhead < id) {
	handle_prune(l, l->skip_to(id, w_min));
	ldry = l->at_end();
    }

    if (rhead < id) {
	handle_prune(r, r->skip_to(id, w_min));

	if (r->at_end()) {
	    PostList *ret = l;
	    l = NULL;
	    return ret;
	}
	rhead = r->get_docid();
    }

    if (ldry) {
	PostList *ret = r;
	r = NULL;
	return ret;
    }
    
    lhead = l->get_docid();
    return advance_to_next_match(w_min);
}
