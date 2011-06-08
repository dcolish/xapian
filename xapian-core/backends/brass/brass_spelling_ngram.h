/** @file brass_spelling_ngram.h
 * @brief N-Gram spelling correction data for a brass database.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_SPELLING_NGRAM_H
#define XAPIAN_INCLUDED_BRASS_SPELLING_NGRAM_H

#include <xapian/types.h>

#include "brass_spelling.h"
#include "termlist.h"

#include <map>
#include <set>
#include <vector>

#include <string>

namespace Brass
{
	struct fragment
	{
			string data;

			// Default constructor.
			fragment() :
				data(4, 0)
			{
			}

			// Allow implicit conversion.
			fragment(char data_[4]) :
				data(data_, 4)
			{
			}

			fragment(const string& data_) :
				data(data_)
			{
			}

			char & operator[](unsigned i)
			{
				return data[i];
			}

			const char & operator[](unsigned i) const
			{
				return data[i];
			}

			operator std::string() const
			{
				return data;
			}

			bool operator<(const fragment &b) const
			{
				return data.compare(b.data) < 0;
			}
	};

}

class BrassSpellingTableNGram : public BrassSpellingTable
{
		std::map<Brass::fragment, std::set<std::string> > termlist_deltas;

	protected:
		virtual void merge_fragment_changes();

		void toggle_fragment(Brass::fragment frag, const std::string & word);

		virtual void toggle_word(const string& word);

		virtual void populate_word(const string& word, unsigned max_distance, std::vector<TermList*>& result);

	public:

		BrassSpellingTableNGram(const std::string & dbdir, bool readonly) :
			BrassSpellingTable(dbdir, readonly)
		{
		}

		void cancel()
		{
			termlist_deltas.clear();
			BrassSpellingTable::cancel();
		}
};

/** The list of words containing a particular trigram. */
class BrassSpellingTermListNGram : public TermList
{
		/// The encoded data.
		std::string data;

		/// Position in the data.
		unsigned p;

		/// The current term.
		std::string current_term;

		/// Copying is not allowed.
		BrassSpellingTermListNGram(const BrassSpellingTermListNGram &);

		/// Assignment is not allowed.
		void operator=(const BrassSpellingTermListNGram &);

	public:
		/// Constructor.
		BrassSpellingTermListNGram(const std::string & data_) :
			data(data_), p(0)
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

#endif // XAPIAN_INCLUDED_BRASS_SPELLING_NGRAM_H
