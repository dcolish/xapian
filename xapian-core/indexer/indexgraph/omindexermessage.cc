/* omindexermessage.cc: Code for the indexer message
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

#include "om/omindexermessage.h"
#include "om/omerror.h"
#include "omstringstream.h"
#include <algorithm>

OmIndexerData::OmIndexerData() : type(rt_empty)
{
}

OmIndexerData::OmIndexerData(int value_)
	: type(rt_int)
{
    u.int_val = value_;
}

OmIndexerData::OmIndexerData(double value_)
	: type(rt_double)
{
    u.double_val = value_;
}

OmIndexerData::OmIndexerData(const std::string &value_)
	: type(rt_string)
{
    u.string_val = new std::string(value_);
}

OmIndexerData::OmIndexerData(const std::vector<OmIndexerData> &value)
	: type(rt_vector)
{
    u.vector_val = new std::vector<OmIndexerData>(value.size());
    try {
	std::copy(value.begin(), value.end(), u.vector_val->begin());
    } catch (...) {
	delete u.vector_val;
	throw;
    }
}

OmIndexerData::OmIndexerData(const OmIndexerData &other)
	: type(other.type)
{
    switch (type) {
	case rt_empty:
	    break;
	case rt_int:
	    u.int_val = other.u.int_val;
	    break;
	case rt_double:
	    u.double_val = other.u.double_val;
	    break;
	case rt_string:
	    u.string_val = new std::string(*other.u.string_val);
	    break;
	case rt_vector:
	    u.vector_val = new std::vector<OmIndexerData>(*other.u.vector_val);
	    break;
    }
}

void
OmIndexerData::operator=(const OmIndexerData &other)
{
    OmIndexerData temp(other);
    swap(temp);
}

void
OmIndexerData::swap(OmIndexerData &other) {
    union {
	int int_val;
	double double_val;
	std::string *string_val;
	std::vector<OmIndexerData> *vector_val;
    } tempu;
    switch (other.type) {
	case rt_empty:
	    break;
	case rt_int:
	    tempu.int_val = other.u.int_val;
	    break;
	case rt_double:
	    tempu.double_val = other.u.double_val;
	    break;
	case rt_string:
	    tempu.string_val = other.u.string_val;
	    break;
	case rt_vector:
	    tempu.vector_val = other.u.vector_val;
	    break;
    }
    /* now copy this union across */
    switch (type) {
	case rt_empty:
	    break;
	case rt_int:
	    other.u.int_val = u.int_val;
	    break;
	case rt_double:
	    other.u.double_val = u.double_val;
	    break;
	case rt_string:
	    other.u.string_val = u.string_val;
	    break;
	case rt_vector:
	    other.u.vector_val = u.vector_val;
	    break;
    }
    /* And now copy the temp union over ours */
    switch (other.type) {
	case rt_empty:
	    break;
	case rt_int:
	    u.int_val = tempu.int_val;
	    break;
	case rt_double:
	    u.double_val = tempu.double_val;
	    break;
	case rt_string:
	    u.string_val = tempu.string_val;
	    break;
	case rt_vector:
	    u.vector_val = tempu.vector_val;
	    break;
    }
    /* finally swap type */
    std::swap(type, other.type);
}

void
OmIndexerData::destroy_val()
{
    switch (type) {
	case rt_empty:
	case rt_int:
	case rt_double:
	    // nothing to be done
	    break;
	case rt_string:
	    delete u.string_val;
	    break;
	case rt_vector:
	    delete u.vector_val;
	    break;
    }
    type = rt_empty;
}

OmIndexerData::~OmIndexerData()
{
    destroy_val();
}

OmIndexerData::record_type
OmIndexerData::get_type() const
{
    return type;
}

bool
OmIndexerData::is_empty() const
{
    return (type == rt_empty);
}

int
OmIndexerData::get_int() const
{
    if (type != rt_int) {
	throw OmTypeError("OmIndexerData::get_int() called for non-int value");
    }
    return u.int_val;
}

double
OmIndexerData::get_double() const
{
    if (type != rt_double) {
	throw OmTypeError("OmIndexerData::get_double() called for non-double value");
    }
    return u.double_val;
}

std::string
OmIndexerData::get_string() const
{
    if (type != rt_string) {
	std::string message = "OmIndexerData::get_string() called for non-string value";
	/*cerr << *this << endl;
	abort(); */
	throw OmTypeError(message);
    }
    return *u.string_val;
}

OmIndexerData::size_type
OmIndexerData::get_vector_length() const
{
    if (type != rt_vector) {
	throw OmTypeError("OmIndexerData::get_vector_length() called for non-vector value");
    }
    return u.vector_val->size();
}

const OmIndexerData &
OmIndexerData::operator[](unsigned int offset) const
{
    return get_element(offset);
}

const OmIndexerData &
OmIndexerData::get_element(size_type offset) const
{
    if (type != rt_vector) {
	throw OmTypeError("OmIndexerData::get_vector_length() called for non-vector value");
    }
    if (offset > u.vector_val->size()) {
	throw OmRangeError("Access to non-existant element of vector record");
    }
    return (*u.vector_val)[offset];
}

void
OmIndexerData::append_element(const OmIndexerData &element)
{
    if (type != rt_vector) {
	throw OmTypeError("OmIndexerData::append_element() called for non-vector value");
    }
    u.vector_val->push_back(element);
}

void
OmIndexerData::eat_element(OmIndexerData &element)
{
    if (type != rt_vector) {
	throw OmTypeError("OmIndexerData::append_element() called for non-vector value");
    }
    size_t offset = u.vector_val->size();
    u.vector_val->resize(offset + 1);
    (*u.vector_val)[offset].swap(element);
}

void
OmIndexerData::eat_list(OmIndexerData &list)
{
    if (type != rt_vector) {
	throw OmTypeError("OmIndexerData::eat_list() called for non-vector value");
    }
    if (list.type != rt_vector) {
	throw OmTypeError("OmIndexerData::eat_list() called with non-vector argument");
    }
    size_t offset = u.vector_val->size();
    size_t othersize = list.u.vector_val->size();
    size_t newsize = offset + othersize;
    u.vector_val->resize(newsize);
    for (size_t i = 0; i<othersize; ++i) {
	(*u.vector_val)[offset + i].swap((*list.u.vector_val)[i]);
    }
    // now clear out the other list.
    list.u.vector_val->clear();
}

void OmIndexerData::set_empty()
{
    destroy_val();
}

void OmIndexerData::set_int(int value)
{
    destroy_val();
    type = rt_int;
    u.int_val = value;
}

void OmIndexerData::set_double(double value)
{
    destroy_val();
    type = rt_double;
    u.double_val = value;
}

void OmIndexerData::set_string(const std::string &value)
{
    destroy_val();

    // set the string first, since it may throw an exception,
    // which would be bad if we tried to delete the value later.
    u.string_val = new std::string(value);
    type = rt_string;
}

void OmIndexerData::set_vector(std::vector<OmIndexerData>::const_iterator begin,
			       std::vector<OmIndexerData>::const_iterator end)
{
    destroy_val();

    // set the string first, since it may throw an exception,
    // which would be bad if we tried to delete the value later.
    u.vector_val = new std::vector<OmIndexerData>(begin, end);
    type = rt_vector;
}

template <class Stream>
static void write_record(Stream &os,
			 const OmIndexerData &record)
{
    switch (record.get_type()) {
	case OmIndexerData::rt_empty:
	    os << "{empty}";
	    break;
	case OmIndexerData::rt_int:
	    os << record.get_int();
	    break;
	case OmIndexerData::rt_double:
	    os << record.get_double();
	    break;
	case OmIndexerData::rt_string:
	    os << "`" << record.get_string() << "\'";
	    break;
	case OmIndexerData::rt_vector:
	    os << "[ ";
	    {
		for (int i=0; i<record.get_vector_length(); ++i) {
		    if (i > 0) {
			os << ", ";
		    }
		    write_record(os, record[i]);
		}
	    }
	    os << " ]";
	    break;
    }
}

std::string
OmIndexerData::get_description() const
{
    om_ostringstream os;
    write_record(os, *this);
    return os.str();
}

#if 0
std::ostream &operator<<(std::ostream &os, const OmIndexerData &record)
{
    os << "OmIndexerData(";
    write_record(os, record);
    os << ")";
    return os;
}
#endif

std::ostream &operator<<(std::ostream &os, const OmIndexerMessage &message)
{
    os << *message;
    return os;
}
