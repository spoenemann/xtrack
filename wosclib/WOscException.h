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

/** WOscException header file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2009-01-13 17:23:19 $
 * $Revision: 1.4 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#ifndef __WOSCEXCEPTION_H__
#define __WOSCEXCEPTION_H__

#include "WOscLib.h"
#include <string>

/** The error codes of all exceptions which are thrown inside the Weiss OSC
 * Library. Refer to these defines when handling exceptions.
 */
enum ErrorCodes {
	ERR_NO_ERROR = 0,						/**< No exception occurred (should not happen
											 * when throwing exceptions).
											 */
	ERR_REMOVE_FROM_EMPTY_QUEUE,			/**< Trying to remove items from an empty
											 * priority queue. \see WOscPriorityQueue.
											 */
	ERR_INVALID_ADDRESS_NO_SLASH,			/**< Trying to initialize a WOscMessage with
											 * a corrupt address with missing slash at the
											 * beginning.
											 */
	ERR_NOT_DISPATCHING_FROM_ROOT,			/**< Messages can only be dispatched beginning
											 * at a root container.
											 * \see WOscContainer
											 */
	ERR_PARENT_POINTER_NULL,				/**< During construction of a WOscContainer or WOscMethod a NULL pointer was passed
											 * as parent or someone tried to add an OSC container alias (or an
											 * OSC method/method-alias) with NULL pointer as parent.
											 * \see WOscContainer.
											 */
	ERR_ROOT_ALIAS,							/** The root container can not be added as alias to any container.
											 *
											 */
	ERR_NULL_ADDRESS,						/**< NULL-pointer argument passed to constructor
											 * of WOscMessage.
											 * \see WOscMessage::WOscMessage(const char *address)
											 */
	ERR_OSC_STR_BUFF_TOO_SMALL,				/**< When using the getBuffer(char* buffer, int bufferLen)
											 * functions (included in several classes) and the
											 * passed buffer of length "bufferLen" is smaller than
											 * the object-internal data-length, this exception will be
											 * raised.
											 * \see WOscMessage | WOscBundle | WOscString
											 */
	ERR_NULL_BUFFER,						/**< When passed a null pointer to the constructor
											 * of an OSC-message when constructing from a byte-stream.
											 * \see WOscMessage::WOscMessage(char* buffer, int bufferLen)
											 */
	ERR_INVALID_INDEX,						/**< When extracting bundle elements (messages or bundles) from
											 * a WOscBundle object or arguments from a WOscMessage object,
											 * make sure the "idx" parameter remains in valid boundaries by
											 * getting the number of elements contained in the bundle.
											 * \see WOscMessage WOscBundle::GetMessage(int idx) |
											 * WOscBundle WOscBundle::GetBundle(int idx). |
											 * WOscBundle::GetNumMessages() | WOscBundle::GetNumBundles()
											 * WOscMessage::getFloat(int idx) | WOscMessage::getInt(int idx) |
											 * WOscMessage::getString(int idx) | WOscMessage::getBlob(int idx)
											 */
	ERR_BUFFER_TO_SMALL,					/**< When using the getBuffer(char* buffer, int bufferLen)
											 * functions (included in several classes) and the
											 * passed buffer of length "bufferLen" is smaller than
											 * the object-internal data-length, this exception will be
											 * raised.
											 * \see WOscBundle | WOscBundle::getBuffer(char* buffer, int bufferLen)
											 */
	ERR_NOT_ROOT_CONTAINER,					/**< Trying to set a not-root-conatiner as address-space.
											 * \see WOscReceiver::setAddressSpace(WOscContainer* addrSpaceRootContainer)
											 */
	ERR_RECURSIVE_ALIAS,					/**<\todo */

	ERR_TOO_SHORT,
	ERR_EMPTY_BUNDLE,
	ERR_INVALID_TAG,
	ERR_CURRUPT_BUNDLE_LAYOUT,
	ERR_NO_ITEMS_IN_BUNDLE,
	ERR_CAN_NOT_CLONE_UNINITIALIZED_BUNDLE,
};

/** Exceptions which occur inside the WOsc-Library. You should catch by const
 * reference.
 */
class WOSC_EXPORT WOscException{
	
public:
	WOscException(ErrorCodes err, std::string file, int line) :
		m_err(err), m_file(file), m_line(line) { }
	virtual ~WOscException() { }
	ErrorCodes GetErr() const { return m_err; }
	static std::string GetErrStr(ErrorCodes err);
	std::string GetErrStr() const { return GetErrStr(m_err); }
	const std::string& GetFile() const { return m_file; }
	int GetLine() const { return m_line; }

private:
	WOscException();
	/** Error number. Defines the reason why an exception was thrown.*/
	ErrorCodes		m_err;
	/** Source file. */
	std::string		m_file;
	/** Line in source file. */
	int				m_line;
};

#endif	// #ifndef __WOSCEXCEPTION_H__
