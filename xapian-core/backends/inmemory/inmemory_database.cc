/* inmemory_database.cc: interface to text file access */

#include <stdio.h>

#include "omassert.h"
#include "inmemory_database.h"
#include "inmemory_document.h"
#include "textfile_indexer.h"

#include <string>
#include <vector>
#include <map>
#include <list>

//////////////
// Postlist //
//////////////

weight
InMemoryPostList::get_weight() const
{
    Assert(started);
    Assert(!at_end());
    Assert(ir_wt != NULL);

    return ir_wt->get_weight((*pos).positions.size(),
			     this_db->get_doclength(get_docid()));
}

///////////////////////////
// Actual database class //
///////////////////////////

InMemoryDatabase::InMemoryDatabase()
	: totlen(0)
{
    Assert((opened = false) == false);
    Assert((indexing = false) == false);
}

InMemoryDatabase::~InMemoryDatabase()
{
}

void
InMemoryDatabase::open(const DatabaseBuilderParams &params)
{
    Assert(!opened); // Can only open once

    // Check validity of parameters
    Assert(params.readonly == true);
    Assert(params.paths.size() > 0);
    Assert(params.subdbs.size() == 0);
    
    // Index documents
    Assert((indexing = true) == true);

    TextfileIndexer indexer;
    indexer.set_destination(this);

    for(vector<string>::const_iterator p = params.paths.begin();
	p != params.paths.end(); p++) {
	TextfileIndexerSource source(*p);
	cout << *p << endl;
	indexer.add_source(source);
    }

    // Make sure that there's at least one document
    if(postlists.size() <= 0)
	throw OmError("Document was empty or nearly empty - nothing to search");

    Assert((opened = true) == true);
}

DBPostList *
InMemoryDatabase::open_post_list(const termname & tname, RSet *rset) const
{
    Assert(opened);
    Assert(term_exists(tname));

    map<termname, InMemoryTerm>::const_iterator i = postlists.find(tname);
    Assert(i != postlists.end());

    return new InMemoryPostList(this, i->second);
}

TermList *
InMemoryDatabase::open_term_list(docid did) const
{
    Assert(opened);
    Assert(did > 0 && did <= termlists.size());

    return new InMemoryTermList(this, termlists[did - 1]);
}

IRDocument *
InMemoryDatabase::open_document(docid did) const
{
    Assert(opened);
    Assert(did > 0 && did <= doclists.size());

    return new InMemoryDocument(doclists[did - 1]);
}

void
InMemoryDatabase::make_term(const termname &tname)
{
    Assert(indexing == true);
    Assert(opened == false);

    postlists[tname];  // Initialise, if not already there.
}

docid
InMemoryDatabase::make_doc(const docname &dname)
{
    Assert(indexing == true);
    Assert(opened == false);

    termlists.push_back(InMemoryDoc());
    doclengths.push_back(0);
    doclists.push_back(dname);

    AssertParanoid(termlists.size() == doclengths.size());

    return termlists.size();
}

void InMemoryDatabase::make_posting(const termname & tname,
				    docid did,
				    termcount position)
{
    Assert(indexing == true);
    Assert(opened == false);
    Assert(postlists.find(tname) != postlists.end());
    Assert(did > 0 && did <= termlists.size());
    Assert(did > 0 && did <= doclengths.size());

    // Make the posting
    InMemoryPosting posting;
    posting.tname = tname;
    posting.did = did;
    posting.positions.push_back(position);

    // Now record the posting
    postlists[tname].add_posting(posting);
    termlists[did - 1].add_posting(posting);
    doclengths[did - 1] += posting.positions.size();
    totlen += posting.positions.size();
}

bool
InMemoryDatabase::term_exists(const termname &tname) const
{
    Assert(opened);

#ifdef MUS_DEBUG_VERBOSE
    cout << "Looking up term `" << tname.c_str() << "'" << endl;
#endif
    map<termname, InMemoryTerm>::const_iterator p = postlists.find(tname);

    if (p == postlists.end()) {
	return false;
    }
    return true;
}
