/* textfile_indexer.cc
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#include "omassert.h"
#include "omerror.h"
#include "textfile_indexer.h"
#include "stem.h"
#include "index_utils.h"
#include <fstream>
#include <cstdlib>
#include <string>

TextfileIndexerSource::TextfileIndexerSource(const string &fname)
	: filename(fname)
{ return; }

istream *
TextfileIndexerSource::get_stream() const
{
    std::ifstream * from = new std::ifstream(filename.c_str());
    if(!*from) throw OmError("Cannot open file " + filename + " for indexing");
    return from;
};


void
TextfileIndexer::add_source(const IndexerSource &source)
{
    Assert(dest != NULL);
    istream *from = source.get_stream();

    // Read lines, each line is a document, split lines into words,
    // each word is a term
    // FIXME - This is just a temporary hack - we want to make a toolkit
    // of indexing "bits" and allow the user to specify how to put them
    // together.

    StemEn stemmer;

    while(*from) {
	string para;
	get_paragraph(*from, para);
	//get_a_line(*from, para);
	
	docid did = dest->make_doc(para);
	termcount position = 1;

	string::size_type spacepos;
	termname word;
	while((spacepos = para.find_first_not_of(" \t\n")) != string::npos) {
	    if(spacepos) para = para.erase(0, spacepos);
	    spacepos = para.find_first_of(" \t\n");
	    word = para.substr(0, spacepos);
	    select_characters(word, "");
	    lowercase_term(word);
	    word = stemmer.stem_word(word);
	    dest->make_term(word);
	    dest->make_posting(word, did, position++);
	    para = para.erase(0, spacepos);
	}
    }

    delete from;
}
