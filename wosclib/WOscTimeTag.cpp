/*
 * WOscLib, an object oriented OSC library.
 * Copyright (C) 2005 Uli Clemens Franke, Weiss Engineering LTD, Switzerland.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * For details see lgpl.txt
 *
 * Weiss Engineering LTD.
 * Florastrass 42
 * 8610 Uster
 * Switzerland
 *
 * uli.franke@weiss.ch
 */

/** WOscTimeTag source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2006-08-13 16:38:01 $
 * $Revision: 1.4 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#include "WOscTimeTag.h"
#include "WOscUtil.h"
#include "limits.h"

/*  -----------------------------------------------------------------------  */
/* 	WOscSystemTime                                                           */
/*  -----------------------------------------------------------------------  */

#if WOSC_HAS_STD_TIME_LIB
#	include <time.h>
	/* 17 leap years, stolen from OSC-kit, no guarantee for correctness...	 */
#	if defined( _MSC_VER )
#		define SECONDS_FROM_1900_TO_1970 2208988800
#	elif defined(__GNUC__)
#		define SECONDS_FROM_1900_TO_1970 2208988800ULL
#	endif
#endif

/** Returns the system time.
 * When not overridden in a derived class, it returns largest time tag possible.
 * 
 * \returns
 * Time representing the current system time.
 *
 * \remarks
 * Time tags are represented by a 64 bit fixed point number. The first 32 bits specify
 * the number of seconds since midnight on January 1, 1900, and the last 32 bits specify
 * fractional parts of a second to a precision of about 200 picoseconds. This is the rep-
 * resentation used by Internet NTP timestamps. The time tag value consisting of 63 zero
 * bits followed by a one in the least signifigant bit is a special case meaning "immediately."
 */
WOscTimeTag
WOscSystemTime::GetSystemTime() const
{
#if WOSC_HAS_STD_TIME_LIB
	/* The time.h lib states:
	 * "" time_t  time ( time_t * timer ); ""
	 * Get the number of seconds elapsed since 00:00 hours,
	 * Jan 1, 1970 UTC from the system clock.
	 */
	WOscTimeTag systemTime;
	systemTime.m_timeTag = (uint64_t)time(NULL) + SECONDS_FROM_1900_TO_1970;
	return systemTime;
#else
	return WOscTimeTag::GetLargestTimeTag();
#endif
}


/*  -----------------------------------------------------------------------  */
/* 	WOscTimeTag                                                              */
/*  -----------------------------------------------------------------------  */

/** Constructor
 * Initializes the time-tag to immediate time.
 * 
 */
WOscTimeTag::WOscTimeTag()
{
	m_timeTag = 1;
}

/** Construct an OSC-time tag from a raw OSC (network) byte-
 * stream (buffer).
 * The OSC (network) buffer has to be big-endian
 * 
 * \param rawTimeTag
 * Buffer containing the raw time-tag
 * 
 * \remarks
 * The argument buffer has to have a minimum length of 
 * WOscTimeTag::Constants::TIME_TAG_SIZE. Smaller lengths
 * may produce runtime errors.
 */
WOscTimeTag::WOscTimeTag(const char* rawTimeTag)
{
	InitFromCharArray(rawTimeTag);
}

/** Copy constructor.
 * Initializes the current time-tag with the data from the referenced.
 * 
 * \param rhs
 * Reference of time-tag to be copied.
 */
WOscTimeTag::WOscTimeTag(const WOscTimeTag& rhs)
{
	m_timeTag = rhs.m_timeTag;
}

/** Returns the current system time.
 * A optional system-time object can be passed, which implements
 * the system specific system-time-query.
 * 
 * \param systemTime
 * Pointer to system time object (optional). If not specified,
 * The system time gets initialized with the library default 
 * system time.
 *
 * \returns
 * Time-tag representing the current system time.
 */
WOscTimeTag WOscTimeTag::GetCurrentTime(const WOscSystemTime* systemTime /* = NULL*/){
	if ( systemTime == NULL ){
		/* get library default */
		WOscSystemTime time;
		return time.GetSystemTime();
	}else
		return systemTime->GetSystemTime();
}

/** Returns the "immediate" time-tag.
 * Immediate implies the LSB set to one. 
 * 
 * \returns
 * Immediate time-tag.
 */
WOscTimeTag
WOscTimeTag::GetImmediateTime()
{
	WOscTimeTag retTag;
	retTag.m_timeTag = 1;
	return retTag;
}

/** Returns the largest possible time-tag.
 * I.e. all bits set to one.
 * 
 * \returns
 * Largest possible time-tag.
 */
WOscTimeTag
WOscTimeTag::GetLargestTimeTag()
{
	WOscTimeTag retTag;
	retTag.m_timeTag = _UI64_MAX;
	return retTag;
}

/** Returns the "immediate" time-tag.
 * Immediate implies the LSB set to one. 
 * 
 * \returns
 * Immediate time-tag.
 */
WOscTimeTag
WOscTimeTag::GetSmallestTimeTag()
{
	return GetImmediateTime();
}

/** Sets this time-tag to the current system time.
 * A optional system-time object can be passed, which implements
 * the system specific system-time-query.
 * 
 * \param systemTime
 * Pointer to system time object (optional). If not specified,
 * The system time gets initialized with the library default 
 * system time.
 *
 * \see
 * WOscTimeTag::getCurrentTime(const WOscSystemTime* systemTime)
 */
void
WOscTimeTag::SetToCurrentTime(const WOscSystemTime* systemTime /*= NULL*/)
{
	*this = systemTime->GetSystemTime();
}

/** Sets this time tag to immediate time.
 * 
 * \see
 * WOscTimeTag::getImmediateTime()
 */
void
WOscTimeTag::SetToImmediateTime()
{
	*this = GetImmediateTime();
}

/** Sets this time tag to the biggest time-tag possible.
 * 
 * \see
 * WOscTimeTag::getLargestTimeTag()
 */
void
WOscTimeTag::SetToLargestTimeTag()
{
	*this = GetLargestTimeTag();
}

/** Sets this time tag to the smallest time-tag possible.
 * 
 * \see
 * WOscTimeTag::getSmallestTimeTag()
 */
void
WOscTimeTag::SetToSmallestTimeTag()
{
	*this = GetSmallestTimeTag();
}

/** Add operator.
 * Adds the right-hand time-tag to the left-hand time-tag
 * and returns the result without modifying the left-hand time-tag.
 *
 * \param rhs
 * Right-hand time-tag reference to be added to the left one.
 * 
 * \returns
 * Left-hand time-tag plus right-hand time-tag. 
 */
WOscTimeTag
WOscTimeTag::operator+ (const WOscTimeTag& rhs) const
{
	WOscTimeTag retTag;
	retTag.m_timeTag = m_timeTag + rhs.m_timeTag;
	return retTag;
}

/** Assignment operator.
 * Assigns the right-hand time-tag to the left-hand time-tag
 * and returns it.
 *
 * \param rhs
 * Right-hand time-tag reference to be assigned to the left one.
 * 
 * \returns
 * Left-hand time-tag value which equals now the right-hand time-tag. 
 */
WOscTimeTag
WOscTimeTag::operator= (const WOscTimeTag& rhs)
{
	m_timeTag = rhs.m_timeTag;
	return *this;
}

/** Add and assign operator.
 * Adds the right-hand time-tag to the left-hand time-tag,
 * stores the result in the left-hand time-tag and 
 * returns the result.
 *
 * \param rhs
 * Right-hand time-tag reference to be added to the left one.
 * 
 * \returns
 * Left-hand time-tag plus right-hand time-tag. 
 */
WOscTimeTag
WOscTimeTag::operator+= (const WOscTimeTag& rhs)
{
	m_timeTag =+ rhs.m_timeTag;
	return *this;
}

/** Subtract operator.
 * Subtracts the right-hand time-tag from the left-hand time-tag
 * and returns the result without modifying the left-hand time-tag.
 *
 * \param rhs
 * Right-hand time-tag reference to be subtracted from the left one.
 * 
 * \returns
 * Left-hand time-tag minus right-hand time-tag. 
 */
WOscTimeTag
WOscTimeTag::operator- (const WOscTimeTag& rhs) const
{
	WOscTimeTag retTag;
	retTag.m_timeTag = m_timeTag - rhs.m_timeTag;
	return retTag;
}

/** Subtract and assign operator.
 * Subtracts the right-hand time-tag from the left-hand time-tag,
 * stores the result in the left-hand time-tag and 
 * returns the result.
 *
 * \param rhs
 * Right-hand time-tag reference to be subtracted from the left one.
 * 
 * \returns
 * Left-hand time-tag minus right-hand time-tag. 
 */
WOscTimeTag
WOscTimeTag::operator-= (const WOscTimeTag& rhs)
{
	m_timeTag -= rhs.m_timeTag;
	return *this;
}

/** Smaller or equal comparison operator.
 * Returns true if the left time-tag is smaller (older)
 * than the right.
 * 
 * \param rhs
 * Right-hand side.
 * 
 * \returns
 * True if the left time-tag is smaller (older)
 * than the right. False else.
 */
bool
WOscTimeTag::operator< (const WOscTimeTag& rhs) const
{
	return (m_timeTag < rhs.m_timeTag);
}

/** Smaller or equal comparison operator.
 * Returns true if the left time-tag is smaller (older)
 * or equal (same time) than the right.
 * 
 * \param rhs
 * Right-hand side.
 * 
 * \returns
 * True if the left time-tag is smaller (older)
 * or equal (same time) than the right. False else.
 */
bool
WOscTimeTag::operator<= (const WOscTimeTag& rhs) const
{
	return (m_timeTag <= rhs.m_timeTag);
}

/** Is equal operator.
 * Returns true if the right time-tag is equal to the left.
 * 
 * \param rhs
 * Right-hand side.
 * 
 * \returns
 * True if the right time-tag is equal to the left.
 * False else.
 */
bool
WOscTimeTag::operator== (const WOscTimeTag& rhs) const
{
	return (m_timeTag == rhs.m_timeTag);
}

/** Is not equal operator.
 * Returns true if the right time-tag is different from the left.
 * 
 * \param rhs
 * Right-hand side.
 * 
 * \returns
 * True if the right time-tag is different from the left.
 * False else.
 */
bool
WOscTimeTag::operator!= (const WOscTimeTag& rhs) const
{
	return (m_timeTag != rhs.m_timeTag);
}

/** Bigger comparison operator.
 * Returns true if the right time-tag is smaller (older)
 * than the left.
 * 
 * \param rhs
 * Right-hand side.
 * 
 * \returns
 * True if the right time-tag is smaller (older)
 * than the left. False else.
 */
bool
WOscTimeTag::operator> (const WOscTimeTag& rhs) const
{
	return (m_timeTag > rhs.m_timeTag);
}

/** Bigger or equal comparison operator.
 * Returns true if the right time-tag is smaller (older)
 * or equal (same time) than the left.
 * 
 * \param rhs
 * Right-hand side.
 * 
 * \returns
 * True if the right time-tag is smaller (older)
 * or equal (same time) than the left. False else.
 */
bool
WOscTimeTag::operator>= (const WOscTimeTag& rhs) const
{
	return (m_timeTag >= rhs.m_timeTag);
}

/** Returns a new char array of size TIME_TAG_SIZE filled with the
 * network aligned (big endian) timetag.
 * 
 * The caller has to free the allocated memory.
 *
 * \returns
 * The allocated char array filled with the
 * network aligned (big endian) timetag.
 * 
 * \remarks
 * The caller has to free the allocated memory.
 * 
 */
const char*
WOscTimeTag::ToCharArray() const
{
	char* buffer = new char[sizeof(uint64_t)];
	WOscUtil::FillBufferWith(buffer, m_timeTag);
	return buffer;
}

/** Fill the given buffer with the network char array
 * representation of an OSC time tag.
 *
 * \param buffer
 * Caller supplied buffer with a minimum size of TIME_TAG_SIZE.
 * 
 * \remarks
 * The byte stream buffer has to have a minimum length of
 * TIME_TAG_SIZE.
 */
void WOscTimeTag::WriteToCharArray(char* buffer) const
{
	WOscUtil::FillBufferWith(buffer, m_timeTag);
}

/** Initializes an OSC time tag from a byte stream (big endian
 * ordered).
 * The byte stream buffer has to have a minimum length of
 * TIME_TAG_SIZE.
 *
 * \param buffer
 * Pointer to buffer with initialization data.
 * 
 * \remarks
 * The byte stream buffer has to have a minimum length of
 * TIME_TAG_SIZE.
 */
void
WOscTimeTag::InitFromCharArray(const char* buffer)
{
	m_timeTag = WOscUtil::BufferTo<uint64_t>(buffer);
}
