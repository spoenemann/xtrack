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

/** WOscMethod and WOscMethodInfo source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2009-01-13 17:23:19 $
 * $Revision: 1.5 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */

#include "WOscMethod.h"
#include "WOscException.h"
#include "WOscContainer.h"

/*  -----------------------------------------------------------------------  */
/* 	WOscMethodInfo                                                           */
/*  -----------------------------------------------------------------------  */

/** Constructs a method-info from a string.
 * The string should contain some information about the method.
 * 
 * \param description
 * String with method description.
 * 
 * \throws None.
 */
WOscMethodInfo::WOscMethodInfo(const char* description) : WOscInfo(description)
{ }

/** Constructs an empty method-info object.
 * The internal buffer is initialized to zero.
 * 
 * \throws None.
 */
WOscMethodInfo::WOscMethodInfo() :WOscInfo()
{ }

/*  -----------------------------------------------------------------------  */
/* 	WOscMethod                                                               */
/*  -----------------------------------------------------------------------  */

/** Constructor.
 * Osc-methods can only be constructed in presence of a parent container, since
 * method-orphans are not allowed. Additionally they have to be instantiated
 * dynamically because the OSC-address-tree (formed by WOscContainer objects)
 * handles the clean-up of all tree-elements. Therefor the destructor is private.
 * 
 * \param info
 * Info about this method.
 * 
 * \param parent
 * Pointer of the container that's going to be the parent of this method.
 * 
 * \param context
 * A void pointer for later use from within the method. This can be used to pass
 * an object pointer on which the method should operate lateron (As implemented in
 * WOscReceiver).
 * 
 * \param name
 * The OSC-pattern that will be the name of this method in the OSC-address
 * space. If WOSC_ADDR_WILDCARDS is defined 1, wildcard matching will be
 * performed on the address path segments.
 * 
 * \throws WOscException
 * When illegal parent pointers are passed (i.e. NULL).
 */
WOscMethod::WOscMethod(WOscContainer* parent, const char* name,
		WOscMethodInfo* info) :
	m_parent(parent), m_info(info)
{
	if ( parent == NULL )
		throw WOscException(ERR_PARENT_POINTER_NULL, __FILE__, __LINE__);
	m_parent->AddMethod(this, name, false);
}

/** Adds this method as alias to the list of methods of the
 * container passed as argument.
 *
 * \param parent
 * Parent container to which this method should be added as alias.
 *
 * \param alias
 * New (aliased) name of the method.
 * 
 * \throws WOscException
 * When passing an illegal parent pointer (i.e. NULL).
 * 
 * \see
 * WOscContainer::addContainerAlias(WOscContainer* parent, char* alias)
 */
void WOscMethod::AddMethodAlias(WOscContainer* parent, const char* alias)
{
	if ( parent == NULL ) // should not happen
		throw WOscException(ERR_PARENT_POINTER_NULL, __FILE__, __LINE__);
	parent->AddMethod(this, alias, true);
}

// will be overridden
/** The OSC method.
 * This is the function that will be called, when a certain OSC-address in a message
 * matches the address of this method-object. The message is then passed together
 * with a network-return-address and a timeTag which contains the point in time of the 
 * invokation.
 *
 * Each message object can access the programm-ressources without global variables
 * by using the context-pointer.
 * 
 * \param message
 * Message causing this call (eventually containing arguments).
 * 
 * \param when
 * Time of invokation (should be current time, when a priority queue, system time and
 * a fast update-rate is used.
 * 
 * \param networkReturnAddress
 * The network address where the message came from.
 *
 * \throws
 * Not in this, but perhaps in inherited classes and in their function-calls.

void WOscMethod::method(WOscMessage *message, 
						WOscTimeTag& when, 
						WOscNetReturn* networkReturnAddress){

}
 */

/** Gets the name (the single pattern, \b not the OSC-address)
 * of this node.
 * If a container is named "xyz" and it's located in the
 * OSC-address-space at "/abcd/klm/xyz/lalala", only xyz is returned.
 * 
 * \returns
 * The name of this container as WOscString.
 * 
 */
WOscString WOscMethod::GetName()
{
	/* search all children of parent for not aliased name */
	const std::vector<WOscMethodEntry>& methEntries = m_parent->m_methodEntries;
	for ( int i = methEntries.size()-1; i >= 0; i-- )
		if ( methEntries[i].m_method == this && !methEntries[i].m_isAlias ) {
			return WOscString(methEntries[i].m_name.c_str());
		}

	return WOscString();
}

/** Returns the whole OSC-address-space as seen from root.
 * The address-space contains only addresses with valid
 * leaves (OSC-methods).
 * 
 * \returns
 * All OSC-addresses which form the OSC-address-space,
 * separated by '\n'-characters.
 * 
 */
WOscString
WOscMethod::GetAddressSpace()
{
	const WOscContainer* root = m_parent->FindRoot();
	return root->GetMethodList();
}


/** Returns the OSC-address of this container as seen from the root.
 * If this container should be aliased, it returns the main,
 * not aliased, address.
 * 
 * \returns
 * OSC-address as WOscString.
 * 
 * \remarks
 * Remember: Container-addresses end with '/', method-addresses
 * with no slash.
 *
 */
WOscString WOscMethod::GetAddress()
{
	WOscContainer* momentaryParent = m_parent;
	WOscString address = GetName();
	WOscString tmp;

	/* traverse tree upwards until root is reached */
	while ( momentaryParent != NULL ){
		if (momentaryParent->m_parent == NULL) break;
		WOscString tmp = momentaryParent->GetName();
		tmp += '/';
		address = tmp + address;

		momentaryParent = momentaryParent->m_parent;
	}
	tmp = '/';
	
	return tmp + address;
}


/** Returns the main parent-container (not aliased ones!).
 * For use in the method-member function to get the calling-
 * container-context.
 * 
 * \returns
 * The main parent container (not aliased).
 */
WOscContainer* WOscMethod::GetParent()
{
	return m_parent;
}
