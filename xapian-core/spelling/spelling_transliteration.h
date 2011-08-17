/** @file spelling_transliteration.h
 * @brief Spelling transliteration.
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

#ifndef XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_H
#define XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_H

#include <vector>
#include <string>
#include <map>

#include <xapian/intrusive_ptr.h>
#include <xapian/visibility.h>

namespace Xapian {

/**
 * Base class for word transliteration methods
 */
class XAPIAN_VISIBILITY_DEFAULT SpellingTransliterationImpl : public Xapian::Internal::intrusive_base {

    static const unsigned MAX_TRANSLITERATIONS = 1 << 7;

    std::map<std::string, std::vector<std::string> > translit_map;
    std::map<std::string, std::vector<std::string> > reverse_translit_map;

    std::string language_name;
    std::string language_code;

    bool is_default(unsigned ch) const;

protected:
    SpellingTransliterationImpl(const std::string& language_name_, const std::string& language_code_);

    void add_mapping(const char* source, const char* translit);

    void add_reverse_mapping(const char* source, const char* translit);

    void make_reverse_mapping();

    void get_transliterations(const std::string& word,
                              const std::map<std::string, std::vector<std::string> >& char_map,
                              bool keep_default, bool limit_variants,
                              std::vector<std::string>& transliterations) const;

public:
    std::string get_transliteration(const std::string& word) const;

    std::vector<std::string> get_transliterations(const std::string& word) const;

    const std::string& get_lang_name() const;

    const std::string& get_lang_code() const;
};

class XAPIAN_VISIBILITY_DEFAULT SpellingTransliteration {

    struct SpellingTransliterationStatic {
	Xapian::Internal::intrusive_ptr<SpellingTransliterationImpl> default_internal;
	std::vector< Xapian::Internal::intrusive_ptr<SpellingTransliterationImpl> > internals;

	SpellingTransliterationStatic();
    };

    static const SpellingTransliterationStatic static_instance;
    Xapian::Internal::intrusive_ptr<SpellingTransliterationImpl> internal;

public:
    SpellingTransliteration(const std::string& name);
    SpellingTransliteration(SpellingTransliterationImpl* impl = NULL);

    //Return one variant of the given word transliterated from the
    //selected to the latin alphabet.
    std::string get_transliteration(const std::string& word) const;

    //Return variants of the given word transliterated to and from the
    //selected alphabet. (Limited by the MAX_TRANSLITERATIONS const)
    std::vector<std::string> get_transliterations(const std::string& word) const;
};

}
#endif // XAPIAN_INCLUDED_SPELLING_TRANSLITERATION_H
