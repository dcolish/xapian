/* error.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_ERROR_H
#define OM_HGUARD_ERROR_H

#include <string>
#include <stdexcept>

#include "omtypes.h"

/// Base class for all errors reported
class OmError {
    private:
        string msg;

	/// assignment operator private and unimplemented
	void operator=(const OmError &copyme);
    protected:
    	/** constructors are protected, since they can only
	 * be used by derived classes anyway.
	 */
        OmError(const string &error_msg) : msg(error_msg) { }
	OmError(const OmError &copyme) : msg(copyme.msg) {}
    public:
	/** Return a message describing the error.  This is in a human
	 *  readable form.
	 */
        string get_msg()
        {
            return msg;
        }

        /// Instantiations of OmError (as opposed to subclasses) are forbidden
	virtual ~OmError() = 0;
};


inline OmError::~OmError() {}

/** An exception derived from OmLogicError is thrown when a misuse
 *  of the API is detected.
 *  @memo Base class for errors due to programming errors.
 */
class OmLogicError : public OmError {
    protected:
        OmLogicError(const string &msg) : OmError(msg) {};
};

/** An exception derived from OmRuntimeError is thrown when an
 *  error is caused by problems with the data or environment rather
 *  than a programming mistake.
 *  @memo Base class for errors due to run time problems.
 */
class OmRuntimeError : public OmError {
    protected:
        OmRuntimeError(const string &msg) : OmError(msg) {};
};

/** Thrown if an internal consistency check fails.
 *  This represents a bug in Muscat. */
class OmAssertionError : public OmLogicError {
    public:
	OmAssertionError(const string &msg)
		: OmLogicError(msg + " - assertion failed") {};
};

/** Thrown when an attempt to use an unimplemented feature is made. */
class OmUnimplementedError : public OmLogicError {
    public:
        OmUnimplementedError(const string &msg) : OmLogicError(msg) {};
};

/** Thrown when an invalid argument is supplied to the API. */
class OmInvalidArgumentError : public OmLogicError {
    public:
        OmInvalidArgumentError(const string &msg) : OmLogicError(msg) {};
};

/** Thrown when an attempt is made to access a document which is not in the
 *  database.  This could occur either due to a programming error, or
 *  because the database has changed since running the query. */
class OmDocNotFoundError : public OmRuntimeError {
    public:
	OmDocNotFoundError(const string &msg) : OmRuntimeError(msg) {};
};

/** Thrown when opening a database fails. */
class OmOpeningError : public OmRuntimeError {
    public:
        OmOpeningError(const string &msg) : OmRuntimeError(msg) {};
};

#endif /* OM_HGUARD_ERROR_H */
