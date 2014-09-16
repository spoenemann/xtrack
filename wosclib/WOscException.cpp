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

/** WOscException source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2009-01-13 17:23:19 $
 * $Revision: 1.3 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#include "WOscException.h"
#include <stdio.h>	// sprintf

/**
 * 
 */
std::string
WOscException::GetErrStr(ErrorCodes err)
{
	switch ( err ) {
	case ERR_NO_ERROR: return
	"No error.";
	case ERR_NOT_ROOT_CONTAINER: return
	"Trying to set a child-container as address-space. Address-space must be a root-conatiner.";
	case ERR_REMOVE_FROM_EMPTY_QUEUE: return
	"Trying to remove items from an empty Queue.";
	case ERR_OSC_STR_BUFF_TOO_SMALL: return
	"Buffer too small when trying to copy string to buffer.";
	case ERR_BUFFER_TO_SMALL: return
	"Caller provided buffer to small when trying to copy the binary bundle content into it.";
	case ERR_INVALID_INDEX: return
	"Invalid index (out of bounds) when trying to remove an OSC-bundle from the enclosing bundle.";
	case ERR_NULL_ADDRESS: return
	"No address (NULL) supplied during message construction.";
	case ERR_NULL_BUFFER: return
	"Message initialization buffer is empty.";
	case ERR_NOT_DISPATCHING_FROM_ROOT: return
	"Can dispatch messages only when called from root container.";
	case ERR_INVALID_ADDRESS_NO_SLASH: return
	"Invalid address. Does not begin with slash (/).";
	case ERR_ROOT_ALIAS: return
	"This is the root container and the root container can not be aliased.";
	case ERR_RECURSIVE_ALIAS: return
	"Recursive aliasing not allowed.";
	case ERR_PARENT_POINTER_NULL: return
	"Invalid (NULL) parent container passed.";
	case ERR_TOO_SHORT: return
	"Bundle too short (parsing).";
	case ERR_EMPTY_BUNDLE: return
	"Bundle has no data (parsing).";
	case ERR_INVALID_TAG: return
	"Bundle has invalid tag. Should be \"#bundle\" (parsing).";
	case ERR_CURRUPT_BUNDLE_LAYOUT: return
	"Bundle layout invalid. Bundle size does not match the sum of the size of the individual bundle items (parsing).";
	case ERR_NO_ITEMS_IN_BUNDLE: return
	"No items in bundle (parsing).";
	case ERR_CAN_NOT_CLONE_UNINITIALIZED_BUNDLE: return
	"Can not clone an uninitialized bundle for priority queue insertion.";
	}
	char buffer[32];
	sprintf(buffer, "Unknown error (%i).", err);
	return buffer;
}
