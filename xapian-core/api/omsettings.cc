/* omsettings.cc: "global" settings object
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

#include "config.h"
#include "omlocks.h"
#include "refcnt.h"
#include "omdebug.h"
#include "om/omsettings.h"
#include "utils.h"
#include <map>
#include <string>
#include <stdio.h>

//////////////////////////////////////////////////////////////////
// OmSettings ref-counted data
// ===========================

class SettingsData : public RefCntBase {
    public:
	typedef std::string key_type;
	typedef std::string value_type;

	typedef map<key_type, value_type> map_type;

	map_type values;
};

//////////////////////////////////////////////////////////////////
// OmSettings::Internal class
// ================
class OmSettings::Internal {
    private:
	/// Mutex used for controlling access to data.
	OmLock mutex;

	/// The actual data (or ref-counted pointer to it)
	RefCntPtr<SettingsData> data;
    public:
	Internal();

	/** Copy constructor.
	 *  The copies are reference-counted, so copies are relatively
	 *  cheap.  Modifications to a copy don't affect other existing
	 *  copies (the copy is copy-on-write). */
	Internal(const Internal &other);
	/** Assignment as with copy.  */
	void operator=(const Internal &other);

	/** Destructor. */
	~Internal();

	/** Set an option value.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, const std::string &value);

	/** Get a setting value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	std::string get(const std::string &key) const;

	/** Return stored settings as a string for use by
	 *  OmSettings::get_description()
	 */
	std::string get_description() const;
};

////////////////////////////////////////////////////////////////
// OmSettings methods

OmSettings::OmSettings()
	: internal(new OmSettings::Internal())
{
    DEBUGAPICALL(void, "OmSettings::OmSettings", "");
}

OmSettings::OmSettings(const OmSettings &other)
	: internal(new OmSettings::Internal(*other.internal))
{
    DEBUGAPICALL(void, "OmSettings::OmSettings", other);
}

void
OmSettings::operator=(const OmSettings &other)
{
    DEBUGAPICALL(void, "OmSettings::operator=", other);
    OmSettings temp(other);

    swap(internal, temp.internal);

    // temp object deleted.
}

OmSettings::~OmSettings()
{
    DEBUGAPICALL(void, "OmSettings::~OmSettings", "");
    delete internal;
}

void
OmSettings::set(const std::string &key, const std::string &value)
{
    DEBUGAPICALL(void, "OmSettings::set", key << ", " << value);
    internal->set(key, value);
}

void
OmSettings::set(const std::string &key, const char *value)
{
    DEBUGAPICALL(void, "OmSettings::set", key << ", " << value);
    internal->set(key, value);
}

void
OmSettings::set(const std::string &key, int value)
{
    DEBUGAPICALL(void, "OmSettings::set", key << ", " << value);
    internal->set(key, om_tostring(value));
}

void
OmSettings::set(const std::string &key, double value)
{
    DEBUGAPICALL(void, "OmSettings::set", key << ", " << value);
    internal->set(key, om_tostring(value));
}

void
OmSettings::set(const std::string &key, bool value)
{
    DEBUGAPICALL(void, "OmSettings::set", key << ", " << value);
    internal->set(key, value ? "1" : "");
}

void
OmSettings::set(const std::string &key, vector<string>::const_iterator begin,
		      vector<string>::const_iterator end)
{
    DEBUGAPICALL(void, "OmSettings::set", key << ", " << begin << ", " << end);
    std::string s;
    while (true) {
	s += *begin;
	begin++;
	if (begin == end) break;
	s += '\0';
    }
    internal->set(key, s);
}

std::string
OmSettings::get(const std::string &key) const
{
    DEBUGAPICALL(std::string, "OmSettings::get", key);
    RETURN(internal->get(key));
}

bool
OmSettings::get_bool(const std::string &key) const
{
    DEBUGAPICALL(bool, "OmSettings::get_bool", key);
    std::string s = internal->get(key);
    RETURN(!(s.empty() || s == "0"));
}

int
OmSettings::get_int(const std::string &key) const
{
    DEBUGAPICALL(int, "OmSettings::get_int", key);
    std::string s = internal->get(key);
    int res;
    sscanf(s.c_str(), "%d", &res);
    RETURN(res);
}

double
OmSettings::get_real(const std::string &key) const
{
    DEBUGAPICALL(double, "OmSettings::get_real", key);
    std::string s = internal->get(key);
    double res;
    sscanf(s.c_str(), "%lf", &res);
    RETURN(res);
}

std::string
OmSettings::get(const std::string &key, std::string def) const
{
    DEBUGAPICALL(std::string, "OmSettings::get", key << ", " << def);
    try {
	RETURN(internal->get(key));
    }
    catch (const OmRangeError &e) {
	RETURN(def);
    }
}

bool
OmSettings::get_bool(const std::string &key, bool def) const
{
    DEBUGAPICALL(bool, "OmSettings::get_bool", key << ", " << def);
    try {
	RETURN(get_bool(key));
    }
    catch (const OmRangeError &e) {
	RETURN(def);
    }
}

int
OmSettings::get_int(const std::string &key, int def) const
{
    DEBUGAPICALL(int, "OmSettings::get_int", key << ", " << def);
    try {
	RETURN(get_int(key));
    }
    catch (const OmRangeError &e) {
	RETURN(def);
    }
}

double
OmSettings::get_real(const std::string &key, double def) const
{
    DEBUGAPICALL(double, "OmSettings::get_real", key << ", " << def);
    try {
	RETURN(get_real(key));
    }
    catch (const OmRangeError &e) {
	RETURN(def);
    }
}

vector<string>
OmSettings::get_vector(const std::string &key) const
{
    DEBUGAPICALL(vector<string>, "OmSettings::get_vector", key);
    std::string s = internal->get(key);
    std::string::size_type p = 0, q;
    vector<string> v;
    while (1) {	    
	q = s.find('\0', p);
	v.push_back(s.substr(p, q - p));
	if (q == std::string::npos) break;
	p = q + 1;
    }
    RETURN(v);
}

std::string
OmSettings::get_description() const
{
    DEBUGCALL(INTRO, std::string, "OmSettings::get_description", "");
    /// \todo display all the settings
    RETURN("OmSettings(" + internal->get_description() + ')');
}

////////////////////////////////////////////////////////////////
// OmSettings::Internal methods

OmSettings::Internal::Internal()
	: mutex()
{
    data = new SettingsData;
}

OmSettings::Internal::Internal(const OmSettings::Internal &other)
	: mutex(), data(other.data)
{
}

void
OmSettings::Internal::operator=(const OmSettings::Internal &other)
{
    OmLockSentry sentry(mutex);
    OmSettings::Internal temp(other);
    swap(data, temp.data);
}

OmSettings::Internal::~Internal()
{
}

void
OmSettings::Internal::set(const std::string &key, const std::string &value)
{
    OmLockSentry sentry(mutex);
    // copy on write...
    if (data->ref_count_get() > 1) {
	data = RefCntPtr<SettingsData>(new SettingsData(*data));
    }
    data->values[key] = value;
}

std::string
OmSettings::Internal::get(const std::string &key) const
{
    OmLockSentry sentry(mutex);

    SettingsData::map_type::const_iterator i;
    i = data->values.find(key);

    if (i == data->values.end()) {
	throw OmRangeError(std::string("Setting ") + key + " doesn't exist.");
    }
    return i->second;
}

std::string
OmSettings::Internal::get_description() const
{
    std::string description;
    SettingsData::map_type::const_iterator i;
    for (i = data->values.begin(); i != data->values.end(); i++) {
	description += "\"" + i->first + "\"->\"" + i->second + "\" ";	
    }
    return description;
}
