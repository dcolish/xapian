/* omtime.h: Various useful network-related utilities
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

#ifndef OM_HGUARD_OMTIME_H
#define OM_HGUARD_OMTIME_H

#include "config.h"

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#include <unistd.h>
#endif
#ifdef HAVE_FTIME
#include <sys/timeb.h>   /* time */
#endif
#include <time.h>

/// A class representing a time
class OmTime {
    public:
	/// Initialise to 0
	OmTime() : sec(0), usec(0) {}

	/// Return the current time.
	static OmTime now();

	/// Initialised
	OmTime(long int msec) : sec(msec / 1000), usec((msec % 1000) * 1000) {}
	OmTime(long int sec, long int usec) : sec(sec), usec(usec) {}

	void operator+= (const OmTime &add) {
	    usec += add.usec;
	    sec += add.sec + usec / 1000000;
	    usec %= 1000000;
	}

	OmTime operator+ (const OmTime &add) const {
	    OmTime result;
	    result.usec = usec + add.usec;
	    result.sec = sec + add.sec + result.usec / 1000000;
	    result.usec %= 1000000;
	    return result;
	}

	OmTime operator- (const OmTime &sub) const {
	    OmTime result;
	    result.usec = usec - sub.usec;
	    result.sec = sec - sub.sec;
	    if (result.usec < 0) {
		result.usec += 1000000;
		result.sec -= 1;
	    }
	    return result;
	}

	bool operator> (const OmTime &rhs) const {
	    if (sec > rhs.sec) return true;
	    return (usec > rhs.usec);
	}

	long int sec;
	long int usec;
};

inline OmTime
OmTime::now()
{
    OmTime result;
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tv;
    if (gettimeofday(&tv, 0) == 0) {
	result.sec = tv.tv_sec;
	result.usec = tv.tv_usec;
	return result;
    }
#endif
#ifdef HAVE_FTIME
    struct timeb tp;
    if (ftime(&tp) == 0) {
	result.sec = tp.time;
	result.usec = tp.millitm * 1000;
	return result;
    }
#endif
    result.sec = time(NULL);
    result.usec = 0;
    return result;
}

#endif /* OM_HGUARD_OMTIME_H */
