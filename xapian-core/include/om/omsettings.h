/* omsettings.h: "global" settings object
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_OMSETTINGS_H
#define OM_HGUARD_OMSETTINGS_H

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////
// OmSettings class
// ================

/** This class is used to pass various settings to other OM classes.
 *
 *  The settings available are:
 *
 *  - general
 *    - backend : database backend type - current backends are: auto, da,
 *      db, inmemory, remote, quartz, and sleepycat
 *
 *  - auto pseudo-backend
 *    - auto_dir : directory for auto pseudo-backend to look for database in
 *
 *  - sleepycat backend
 *    - sleepycat_dir : directory containing sleepycat database
 *
 *  - muscat36 backends
 *    - m36_key_file : keys for DA or DB file
 *    - m36_record_file : DA record file
 *    - m36_term_file : DA term file
 *    - m36_db_file : DB file
 *    - m36_db_cache_size : size of DB file cache in blocks (default 30)
 *    - m36_heavyduty : true for 3 byte offset form, false for older 2 byte
 *      form.  Only used by da backend - db backend autodetects. (default true)
 *
 *  - remote backend
 *    - remote_type : "prog" or "tcp"
 *    - remote_program : the name of the program to run to act as a server
 *   			 (when remote_type="prog")
 *    - remote_args : the arguments to the program named in remote_program
 *   			 (when remote_type="prog")
 *    - remote_server : the name of the host running a tcp server
 *   			 (when remote_type="tcp")
 *    - remote_port : the port on which the tcp server is running
 *   			 (when remote_type="tcp")
 *    - remote_timeout : The timeout in milliseconds used before assuming that
 *                       the remote server has failed.  If this timeout is
 *                       reached for any operation, then an OmNetworkTimeout
 *                       exception will be thrown.  The default if not
 *                       specified is 10000ms (ie 10 seconds)
 *    - remote_connect_timeout : The timeout in milliseconds used when
 *                       attempting to connect to a remote server.  If this
 *                       timeout is reached when attempting to connect, then
 *                       an OmNetworkTimeout exception wil be thrown.  The
 *                       default if not specified is to use the value of
 *                       remote_timeout.
 *
 *  - quartz backend
 *    - quartz_dir : directory containing quartz database
 *
 *  - match options
 *    - match_collapse_key : key number to collapse on - duplicates mset
 *      entries will be removed based on a key (default -1 => no collapsing)
 *    - match_sort_forward : If true, documents with the same weight will
 *      be returned in ascending document order; if false, they will be
 *      returned in descending order.flag to sort forward (default true)
 *    - match_percent_cutoff : Minimum percentage score for returned
 *      documents. If a document has a lower percentage score than this, it
 *      will not appear in the mset.  If your intention is to return only
 *      matches which contain all the terms in the query, then consider using
 *      OmQuery::OP_AND instead of OmQuery::OP_OR in the query).
 *      (default -1 => no cut-off)
 *    - match_cutoff :  Minimum weight for a document to be returned.  If a
 *      document has a lower score that this, it will not appear in the mset.
 *      It is usually only possible to choose an appropriate weight for cutoff
 *      based on the results of a previous run of the same query; this is thus
 *      mainly useful for alerting operations.
 *    - match_max_or_terms : Maximum number of terms which will be used if
 *      the query contains a large number of terms which are ORed together.
 *      Only the terms with the match_max_or_terms highest termweights will be
 *      used.  Parts of the query which do not involve terms ORed together will
 *      be unaffected by this option.  An example use of this setting is to
 *      set a query which represents a document - only the elite set of terms
 *      which best distinguish that document to be used to find similar
 *      documents, resulting in a performance improvement.
 *      (default 0 => no limit)
 *    - match_weighting_scheme : Weighting scheme to use; this is either
 *      'bm25' or 'trad'.  The default is bm25.
 *
 *  - BM25 weighting options : The BM25 formula is \f[
 *      \frac{C.s_{q}}{1+L_{d}}+\sum_{t}\frac{(A+1)q_{t}}{A+q_{t}}.\frac{(B+1)f_{t,d}}{B((1-D)+DL_{d})+f_{t,d}}.w_{t}
 *    \f] where
 *    \f$w_{t}\f$ is the termweight of term t,
 *    \f$f_{t,d}\f$ is the within document frequency of term t in document d,
 *    \f$q_{t}\f$ is the within query frequency of term t,
 *    \f$L_{d}\f$ is the normalised length of document d,
 *    \f$s_{q}\f$ is the size of the query,
 *    and \f$A\f$, \f$B\f$, \f$C\f$ and \f$D\f$ are user specified parameters:
 *    - bm25weight_A : A parameter governing the importance of within query
 *                     frequency - any positive number, 0 being wqf not
 *                     used.  Default is 1.
 *    - bm25weight_B : A parameter governing the importance of within
 *                     document frequency - any positive number, 0 being wdf
 *                     not used.  Default is 1.
 *    - bm25weight_C : Size of compensation factor for the high wdf values
 *                     in large documents - any positive number, 0 being no
 *                     compensation.  Default is 0.
 *    - bm25weight_D : Relative importance of within document frequency and
 *                     document length - any number in range 0 to 1.  Default
 *                     is 0.5.
 *
 *  - traditional weighting scheme options : The traditional weighting formula
 *    is \f[
 *      \sum_{t}\frac{f_{t,d}}{k.L_{d}+f_{t,d}}.w_{t}
 *    \f] where
 *    \f$w_{t}\f$ is the termweight of term t,
 *    \f$f_{t,d}\f$ is the within document frequency of term t in document d,
 *    \f$L_{d}\f$ is the normalised length of document d,
 *    and \f$k\f$ is a user specifyable parameter:
 *    - tradweight_k : A parameter governing the importance of within
 *                     document frequency and document length - any positive
 *                     number, 0 being wdf and doc length not used.  Default
 *                     is 1.
 *
 *  - expand options:
 *    - expand_use_query_terms : If false, terms already in the query will be
 *      not be returned in the ESet; if true, they can be. (default false)
 *    - expand_use_exact_termfreq : If true then term frequencies will be
 *      calculated exactly; if true, an approximation may be used which can
 *      greatly improve efficiency. The approximation only applies when
 *      multiple databases are searched together. (default false)
 */
class OmSettings {
    private:
	class Internal;

	/// Internal implementation
	Internal *internal;

    public:
	/** Create a settings object.
	 */
	OmSettings();

	// Maybe add method/constructor to read settings from file?

	/** Copy constructor.
	 *  The copies are reference-counted, so copies are relatively
	 *  cheap.  Modifications to a copy don't affect other existing
	 *  copies (the copy is copy-on-write). */
	OmSettings(const OmSettings &other);
	/** Assignment operator.  This should be cheap. */
	void operator=(const OmSettings &other);

	/** Destructor. */
	~OmSettings();

	/** Set an option value.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, const std::string &value);

	/** Set an option value.
	 *
	 *  @param key   The name of the option as a char *.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, const char *value);
    
	/** Set an option value to an integer.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, int value);

	/** Set an option value to a real number.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, double value);

	/** Set an option value to a boolean.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, bool value);

	/** Set an option value to a vector of strings.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param begin Iterator pointing to start of vector.
	 *
	 *  @param end   Iterator pointing to end of vector.
	 */
	void set(const std::string &key,
		 std::vector<std::string>::const_iterator begin,
		 std::vector<std::string>::const_iterator end);

	/** Get a setting value as a string.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	std::string get(const std::string &key) const;

	/** Get a setting value as a string, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	std::string get(const std::string &key, std::string def) const;

	/** Get a setting value as an integer.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	int get_int(const std::string &key) const;

	/** Get a setting value as an integer, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	int get_int(const std::string &key, int def) const;

	/** Get a setting value as a boolean.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	bool get_bool(const std::string &key) const;

	/** Get a setting value as a boolean, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	bool get_bool(const std::string &key, bool def) const;

	/** Get a setting value as an real number.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	double get_real(const std::string &key) const;

	/** Get a setting value as an real number, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	double get_real(const std::string &key, double def) const;

	/** Get a setting value as a vector of strings.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	std::vector<std::string> get_vector(const std::string &key) const;

	/** Returns a string representing the database group object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif // OM_HGUARD_OMSETTINGS_H
