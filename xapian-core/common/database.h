/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <map>
#include <list>
#include <string>
#include <vector>

#include "omassert.h"

#include "omtypes.h"
#include "error.h"

class PostList {
    private:
    public:
	virtual doccount get_termfreq() const = 0;// Gets number of docs indexed by this term

	virtual docid  get_docid() const = 0;     // Gets current docid
	virtual weight get_weight() const = 0;    // Gets current weight

	virtual void   next() = 0;          // Moves to next docid
	virtual void   skip_to(docid);  // Moves to next docid >= specified docid
	virtual bool   at_end() const = 0;        // True if we're off the end of the list

        virtual ~PostList() { return; }
};

// naive implementation for database that can't do any better
inline void
PostList::skip_to(docid id)
{
    Assert(!at_end());
    while (!at_end() && get_docid() < id) {
	next();
    }
}

class TermList {
    private:
    public:
	virtual termid get_termid() = 0;    // Gets current termid
	virtual termcount get_wdf() = 0;    // Get wdf of current term
	virtual void   next() = 0;          // Moves to next termid
	virtual bool   at_end() = 0;        // True if we're off the end of the list

        virtual ~TermList() { return; }
};

class IRDatabase {
    private:
    public:
	virtual termid term_name_to_id(const termname &) = 0;
	virtual termname term_id_to_name(termid) = 0;
    
        virtual void open(const string &pathname, bool readonly) = 0;
	virtual void close() = 0;

	// Throws RangeError if termid invalid
	virtual PostList * open_post_list(termid) = 0;

	// Throws RangeError if docid invalid
	virtual TermList * open_term_list(docid) = 0;

        virtual ~IRDatabase() { return; }
};

#endif /* _database_h_ */
