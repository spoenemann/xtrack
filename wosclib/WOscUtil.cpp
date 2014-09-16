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

/** WOscUtil source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2006-08-13 16:38:01 $
 * $Revision: 1.3 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#include "WOscUtil.h"
#include "WOscLib.h"
#include <string.h>

/**  Get length of string and round up to the next multiple of 4.
 * Gets the size of the string passed as parameter and rounds it
 * up to the next multiple of four.
 * 
 * \param string
 * String which size should be rounded.
 * 
 * \returns
 * Rounded size of the string.
 * 
 * \see
 * WOscUtil::getSizeFourByteAligned(const int length)
 */
int
WOscUtil::GetSizeFourByteAligned(const char* string)
{
	int length = (int)strlen(string) + 1;
	return length + ( 4 - length % 4 ) % 4;
}

// 
/** Rounds up the given integer to the next multiple of 4.
 * Similar to getSizeFourByteAligned(const char* string).
 * 
 * \param length
 * Integer which sould be rounded up to the next multiple of 4.
 * 
 * \returns
 * The rounded integer.
 * 
 * \see
 * getSizeFourByteAligned(const char* string)
 */
int
WOscUtil::GetSizeFourByteAligned(const int length)
{
	return length + ( 4 - length % 4 ) % 4;
}

/**	Copies the source string to the destination string
 * and pads it with zeros to the next multiple of four, 
 * the destination buffer must be large enough.
 *
 * \param destString
 * Destination string buffer (length must be the rounded-up
 * length of the source string), which receives the padded
 * string.
 * 
 * \param sourceString
 * The string which should be padded.
 * 
 * \remarks
 * Works only with strings!!!
 * 
 * \see
 * For determination of the destination string-buffer-length:
 * getSizeFourByteAligned(const char* string)
 */
void
WOscUtil::PadStringWithZeros(char* destString, const char* sourceString)
{
	int length = (int)strlen(sourceString) + 1;
	int osclength = GetSizeFourByteAligned(sourceString);

	strcpy(destString, sourceString);

	for ( int i = length; i < osclength; i++ )
		destString[i] = '\0';
}

/**	Copies the source buffer to the destination buffer
 * and pads the difference with zeros. 
 * The destination buffer must be large enough, i.e. destLen >= srcLen.
 *
 * Works for every buffer, assumed destLen >= srcLen. If destLen is smaller
 * than srcLen only srcLen data is copied and nothing padded.
 * 
 * \param destBuffer
 * Destination string buffer (length must be the rounded-up
 * length of the source string), which receives the padded
 * string.
 * 
 * \param sourceBuffer
 * Source buffer.
 * 
 * \param destLen
 * Size of the destination-buffer.
 * 
 * \param srcLen
 * Size of the source-buffer.
 * 
 * \see
 * padStringWithZeros(char* destString, const char* sourceString)
 */
void
WOscUtil::PadBufferWithZeros(char* destBuffer, const char* sourceBuffer,
		int destLen, int srcLen)
{
	if ( destLen <= srcLen )
		memcpy(destBuffer, sourceBuffer, destLen);
	else {
		memcpy(destBuffer, sourceBuffer, srcLen);
		memset(destBuffer+srcLen, 0, destLen-srcLen);
	}
}
