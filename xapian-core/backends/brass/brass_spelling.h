/** @file brass_spelling.h
 * @brief Spelling correction data for a brass database.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2011 Nikita Smetanin
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

#ifndef XAPIAN_INCLUDED_BRASS_SPELLING_H
#define XAPIAN_INCLUDED_BRASS_SPELLING_H

#include <xapian/types.h>

#include "brass_lazytable.h"
#include "termlist.h"

#include <ext/hash_map>
#include <map>
#include <set>
#include <string>
#include <vector>



class BrassSpellingTable : public BrassLazyTable {
    static const char WORDS_SIGNATURE = 'M';
    static const char SPELLING_SIGNATURE = 'S';
    static const char* GROUPMAX_SIGNATURE;
    static const char* GROUPSTACK_SIGNATURE;

    std::string pack_words(const std::string& prefix,
                           const std::string& first_word,
                           const std::string& second_word) const;

    void set_entry_wordfreq(char prefix, const std::string& word,
                            Xapian::termcount freq,
			    const std::string& extra_value = string());

    Xapian::termcount get_entry_wordfreq(char prefix,
                                         const std::string& word) const;

    unsigned get_prefix_data(std::vector<unsigned>& index_stack);

    std::map<std::string, Xapian::termcount> wordfreq_changes;
    std::map<std::string, Xapian::termcount> wordsfreq_changes;
    std::map<std::string, unsigned> prefix_changes;
    std::vector<unsigned> prefix_index_stack;
    unsigned prefix_index_max;
    std::map<std::string, std::string> wordvalue_map;

protected:
    static const unsigned PREFIX_DISABLED;

    void set_word_value(const std::string& word, const std::string& value);

    std::string get_word_value(const std::string& word) const;

    virtual void merge_fragment_changes() = 0;

    virtual void toggle_word(const std::string& word,
                             const std::string& prefix) = 0;

    virtual void populate_word(const std::string& word,
                               const std::string& prefix, unsigned max_distance,
			       std::vector<TermList*>& result) const = 0;

public:
    static const char WORD_SIGNATURE = 'W';

    static void append_prefix_group(std::string& data, unsigned value);

    /** Create a new BrassSpellingTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the brass database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    BrassSpellingTable(const std::string & dbdir, bool readonly) :
	BrassLazyTable("spelling", dbdir + "/spelling.", readonly,
		       Z_DEFAULT_STRATEGY), prefix_index_max(0)
    {
    }

    // Merge in batched-up changes.
    void merge_changes();

    void add_word(const std::string & word, Xapian::termcount freqinc,
		  const std::string& prefix = string());

    void remove_word(const std::string & word, Xapian::termcount freqdec,
                     const std::string& prefix = string());

    void add_words(const std::string& first_word,
		   const std::string& second_word, Xapian::termcount freqinc,
		   const std::string& prefix = string());

    void remove_words(const std::string& first_word,
                      const std::string& second_word, Xapian::termcount freqdec,
                      const std::string& prefix = string());

    TermList * open_termlist(const std::string & word,
                             const std::string& prefix = string());

    TermList * open_termlist(const std::string & word, unsigned max_distance,
                             const std::string& prefix = string());

    Xapian::doccount get_word_frequency(const std::string& word,
                                        const std::string& prefix = string()) const;

    Xapian::doccount get_words_frequency(const std::string& first_word,
					 const std::string& second_word,
					 const std::string& prefix = string()) const;

    unsigned get_spelling_group(const std::string& prefix = string()) const;

    //Enable spelling for the given prefix and unite its spelling data with group_prefix.
    void enable_spelling(const std::string& prefix, const std::string& group_prefix);

    //Disable spelling for the given prefix.
    void disable_spelling(const std::string& prefix);

    //Check whether spelling is enabled for the given prefix
    bool is_spelling_enabled(const std::string& prefix) const;

    /** Override methods of BrassTable.
     *
     *  NB: these aren't virtual, but we always call them on the subclass in
     *  cases where it matters.
     *  @{
     */

    bool is_modified() const
    {
	return !wordfreq_changes.empty() || !wordsfreq_changes.empty()
		|| !prefix_changes.empty() || BrassTable::is_modified();
    }

    void flush_db()
    {
	merge_changes();
	BrassTable::flush_db();
    }

    void cancel()
    {
	// Discard batched-up changes.
	wordfreq_changes.clear();
	wordsfreq_changes.clear();
	prefix_changes.clear();
	prefix_index_stack.clear();
	prefix_index_max = 0;

	BrassTable::cancel();
    }
    // @}
};

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_H
