/** @file brass_spelling_fastss.h
 * @brief FastSS spelling correction algorithm for a brass database.
 */
/* Copyright (C) 2011 Nikita Smetanin
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

#ifndef XAPIAN_INCLUDED_BRASS_SPELLING_FASTSS_H
#define XAPIAN_INCLUDED_BRASS_SPELLING_FASTSS_H

#include <xapian/types.h>

#include "brass_spelling.h"
#include "termlist.h"

#include <vector>
#include <xapian/unordered_set.h>

#include <string>

class BrassSpellingTableFastSS : public BrassSpellingTable {

    typedef unsigned termindex;

    static const unsigned MAX_DISTANCE = 2;
    static const unsigned LIMIT = 8; //There is strange behavior when LIMIT > 8
    static const unsigned PREFIX_LENGTH = 4;
    static const char PREFIX_SIGNATURE = 'P';
    static const char* WORD_INDEX_SIGNATURE;
    static const char* WORD_VALUE_SIGNATURE;
    static const char* INDEXMAX_SIGNATURE;
    static const char* INDEXSTACK_SIGNATURE;

    class TermIndexCompare {
	std::vector<std::vector<unsigned> >& wordlist_map;

	unsigned first_word_index;
	unsigned first_error_mask;

	unsigned second_word_index;
	unsigned second_error_mask;

    public:
	TermIndexCompare(std::vector<std::vector<unsigned> >& wordlist_map_) :
	    wordlist_map(wordlist_map_),
	    first_word_index(0), first_error_mask(0),
	    second_word_index(0), second_error_mask(0)
	{
	}

	bool operator()(termindex first_term, termindex second_term);
    };

    //Check if index doesn't exceed the bound 2^(32 - LIMIT).
    static unsigned check_index(unsigned index);

    //Creates key for to access word with given index
    static void get_word_key(unsigned index, std::string& key);

    //Pack word index and error mask into a value
    static termindex pack_term_index(unsigned wordindex, unsigned error_mask);

    //Unpack word index and error mask from a value
    static void unpack_term_index(termindex termindex, unsigned& wordindex,
				  unsigned& error_mask);

    //Get integer value from a string data at given index
    static unsigned get_data_int(const std::string& data, unsigned index);

    //Append integer value to a string data at the end
    static void append_data_int(std::string& data, unsigned value);

    //Binary search in a data for a given word and error mask
    unsigned term_binary_search(const std::string& data,
                                const std::vector<unsigned>& word,
                                unsigned error_mask,
                                unsigned start, unsigned end,
				bool lower) const;

    //Toggle term in database
    void toggle_term(const std::vector<unsigned>& word, std::string& prefix,
                     unsigned prefix_group, unsigned index, unsigned error_mask,
                     bool update_prefix);

    //Recursively call toggle_term with 0 .. max_distance errors.
    void toggle_recursive_term(const std::vector<unsigned>& word,
			       std::string& prefix, unsigned prefix_group,
			       unsigned index, unsigned error_mask, unsigned start,
			       unsigned distance, unsigned max_distance);

    //Search for a word and fill result set
    void populate_term(const std::vector<unsigned>& word, std::string& data,
		       std::string& prefix, unsigned prefix_group,
		       unsigned error_mask, bool update_prefix,
		       std::unordered_set<unsigned>& result) const;

    //Recursively search for a word with 0, 1, ..., max_distance errors.
    void populate_recursive_term(const std::vector<unsigned>& word,
				 std::string& data, std::string& prefix,
				 unsigned prefix_group,
				 unsigned error_mask, unsigned start,
				 unsigned distance, unsigned max_distance,
				 std::unordered_set<unsigned>& result) const;

    //Generate prefix of a word using given error mask.
    void get_term_prefix(const std::vector<unsigned>& word,
			 std::string& prefix, unsigned error_mask,
			 unsigned prefix_length) const;

    //Compare two strings using error mask (error mask contains one-bits at positions with errors)
    template<typename FirstIt, typename SecondIt>
    static int compare_string(FirstIt first_it, FirstIt first_end,
			      SecondIt second_it, SecondIt second_end,
			      unsigned first_error_mask,
			      unsigned second_error_mask, unsigned limit)
    {
	unsigned first_i = 0;
	unsigned second_i = 0;

	for (;; ++first_it, ++second_it, ++first_i, ++second_i, first_error_mask
		>>= 1, second_error_mask >>= 1) {
	    bool first_at_end = false;
	    while (!(first_at_end = (first_i == limit || first_it == first_end))
		    && (first_error_mask & 1)) {
		first_error_mask >>= 1;
		++first_i;
		++first_it;
	    }

	    bool second_at_end = false;
	    while (!(second_at_end = (second_i == limit || second_it
		    == second_end)) && (second_error_mask & 1)) {
		second_error_mask >>= 1;
		++second_i;
		++second_it;
	    }

	    if (first_at_end && second_at_end)
		return 0;

	    if (first_at_end && !second_at_end)
		return -1;

	    if (!first_at_end && second_at_end)
		return 1;

	    if (*first_it < *second_it)
		return -1;

	    if (*first_it > *second_it)
		return 1;
	}
    }

    std::vector<std::vector<unsigned> > wordlist_deltas;
    std::vector<unsigned> wordlist_deltas_prefixes;
    std::map<std::string, std::vector<termindex> > termlist_deltas;

protected:
    void merge_fragment_changes();

    void toggle_word(const std::string& word, const std::string& prefix);

    void populate_word(const std::string& word, const std::string& prefix,
                       unsigned max_distance, std::vector<TermList*>& result) const;

public:
    BrassSpellingTableFastSS(const std::string & dbdir, bool readonly) :
	BrassSpellingTable(dbdir, readonly), wordlist_deltas(),
	wordlist_deltas_prefixes(), termlist_deltas()
    {
    }

    bool get_word(unsigned index, std::string& key, std::string& word) const;

    /** Override methods of BrassSpellingTable.key
     *
     *  NB: these aren't virtual, but we always call them on the subclass in
     *  cases where it matters.
     *  @{
     */
    void cancel()
    {
	wordlist_deltas.clear();
	wordlist_deltas_prefixes.clear();
	termlist_deltas.clear();
	BrassSpellingTable::cancel();
    }
    // @}
};

class BrassSpellingFastSSTermList : public TermList {
    const BrassSpellingTableFastSS& table;
    std::vector<unsigned> words;
    std::string word;
    std::string key_buffer;
    unsigned index;

public:
    BrassSpellingFastSSTermList(const std::vector<unsigned>& words_,
				const BrassSpellingTableFastSS& table_) :
	table(table_), words(words_), index(0)
    {
    }

    Xapian::termcount get_approx_size() const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    Xapian::termcount get_collection_freq() const;

    TermList * next();

    TermList * skip_to(const std::string & term);

    bool at_end() const;

    Xapian::termcount positionlist_count() const;

    Xapian::PositionIterator positionlist_begin() const;
};

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_FASTSS_H
