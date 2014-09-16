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

/** WOscContainer and WOscContainerInfo source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2009-01-13 17:23:19 $
 * $Revision: 1.5 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */

#include "WOscContainer.h"
#include "WOscException.h"
#include <string.h>
#include <iostream>

/*  -----------------------------------------------------------------------  */
/* 	WOscContainer                                                            */
/*  -----------------------------------------------------------------------  */

/** Constructor for a root container.
 * Root container do not have parents (what's implicated by the name).
 * The root-container is responsible for clean-up. Deletion
 * is not allowed for WOscContainer and WOscMethod objects. The 
 * address space can be removed only as whole or be manipulated
 * by dedicated functions.
 *
 * \param info
 * Container information.
 * 
 * \param defNumChildren
 * Default internal array-size for children-references. If not sufficient
 * during later use, the array-sizes double themselves automatically.
 *
 * \param defNumMethods
 * See parameter defNumChildren.
 * 
 */
WOscContainer::WOscContainer(const WOscContainerInfo& info, int defNumChildren, int defNumMethods)
{

	/* a root container does not have any parents */
	m_parent = NULL;

	m_containerEntries.reserve(defNumChildren);
	m_methodEntries.reserve(defNumMethods);

	/* container info */
	m_info = info;
}

/** Constructor for child containers.
 * Child containers must supply a parent. They get added to the parent-container list of children
 * and can be parents if constructed.
 * 
 * \param info
 * Container information.
 * 
 * \param parent
 * Pointer to the parent-container. Must not be NULL.
 * 
 * \param name
 * OSC-pattern with the name of the container. This name will be later
 * a part of the OSC-address (/the/"name"/of/mama/mia/is/method ...).
 * If WOSC_ADDR_WILDCARDS is defined 1, wildcard matching will be performed on
 * the address path segments.
 *
 * \param defNumChildren
 * (Optional) See WOscContainer::WOscContainer(WOscContainerInfo* info, int defNumChildren, int defNumMethods)
 * 
 * \param defNumMethods
 * (Optional) See WOscContainer::WOscContainer(WOscContainerInfo* info, int defNumChildren, int defNumMethods)
 * 
 * \throws WOscException
 * If parent pointer NULL. That's illegal. Children should have parents
 * 
 * \see
 * WOscContainer::WOscContainer(WOscContainerInfo* info, int defNumChildren , int defNumMethods )
 */
WOscContainer::WOscContainer(WOscContainer* parent, const char* name, const WOscContainerInfo& info, int defNumChildren, int defNumMethods){

	if ( parent == NULL )
		throw WOscException(ERR_PARENT_POINTER_NULL, __FILE__, __LINE__);

	m_parent	= parent;
	m_info		= info;

	m_containerEntries.reserve(defNumChildren);
	m_methodEntries.reserve(defNumMethods);

	m_parent->AddContainer(this, name, false);
}

/** Destructor.
 * Can be accessed only by a WOscContainer object.
 * To delete a whole container-tree, call WOscContainer::removeAll() .
 * Deletes - when starting from root - all children (containers and methods) recursivly.
 *
 * \see
 * WOscContainer::removeAll()
 */
WOscContainer::~WOscContainer()
{
	// we have to copy the container list because if we remove the aliases of a
	// container in a loop and we have an alias of the same container anywhere
	// in the methods list, the list becomes invalid.
	std::vector<WOscContainerEntry> containers = m_containerEntries;
	for ( std::vector<WOscContainerEntry>::iterator it = containers.begin(); it < containers.end(); it++ )
		if ( ! it->m_isAlias )
			((WOscContainer*)FindRoot())->RecursivelyRemoveAliases(it->m_container);

	// deallocate all children which are not an alias
	for ( int i = m_containerEntries.size()-1; i >= 0; i-- )
		if ( ! m_containerEntries[i].m_isAlias ) {
			delete m_containerEntries[i].m_container;
		}
	m_containerEntries.clear();

	// we have to copy the methods list because if we remove the aliases of a
	// method in a loop and we have an alias of the same method anywhere in the
	// methods list, the list becomes invalid.
	std::vector<WOscMethodEntry> methods = m_methodEntries;
	for ( std::vector<WOscMethodEntry>::iterator it = methods.begin(); it < methods.end(); it++ )
		if ( ! it->m_isAlias )
			((WOscContainer*)FindRoot())->RecursivelyRemoveAliases(it->m_method);

	// deallocate all methods which are not an alias
	for ( int i = m_methodEntries.size()-1; i >= 0; i-- )
		if ( ! m_methodEntries[i].m_isAlias )
			delete m_methodEntries[i].m_method;
	m_methodEntries.clear();	// not necessary...
}

/** Public function for deletion of a whole container-/method-tree.
 * Searches the root and starts a recursive deletion.
 * 
 * \see
 * WOscContainer::~WOscContainer()
 */
void
WOscContainer::RemoveAll()
{
	delete FindRoot();
}

/** Adds an alias of this container to the parent list-of-children.
 * 
 * \param parent
 * Pointer to container, which becomes the parent of the alias.
 * 
 * \param alias
 * New name of container under which the parent saves the alias.
 * 
 * \throws WOscException
 * When this container is the root-container, the root-container can not be aliased.
 * When the parent pointer is this container itself. 
 * When invalid parent-container pointer (NULL) is passed.
 * 
 * \remarks
 * Aliases are not real children and won't be deleted during recursive deletion.
 */
void
WOscContainer::AddContainerAlias(WOscContainer* parent, const char* alias)
{
	if ( m_parent == NULL )
		throw WOscException(ERR_ROOT_ALIAS, __FILE__, __LINE__);

	if ( parent == this )
		throw WOscException(ERR_RECURSIVE_ALIAS, __FILE__, __LINE__);
	
	if ( parent == NULL ) 
		throw WOscException(ERR_PARENT_POINTER_NULL, __FILE__, __LINE__);

	parent->AddContainer(this, alias, true);
}

/** Function for container-internal adding of method-children.
 * When the WOscMethod constructor is called, it internally
 * calls this function by dereferencing the parent-pointer-argument.
 * 
 * \param method
 * Pointer to method to be added to the list of methods in this container.
 * 
 * \param name
 * String containing the OSC-name of the method.
 * 
 * \param isAlias
 * If the method should be added as alias, this argument is true.
 * 
 * \see
 * WOscContainer::addContainer(WOscContainer* container, char* name, bool isAlias)
 */
void
WOscContainer::AddMethod(WOscMethod* method, const char* name, bool isAlias)
{
	WOscMethodEntry entry;
	entry.m_name = name;
	entry.m_isAlias = isAlias;
	entry.m_method = method;
	m_methodEntries.push_back(entry);
}

/** Adds a container to the child-container array.
 * Privately accessable and used by children
 * to add itself to the parents array of children or aliases.
 * 
 * \param container
 * Pointer of container to be added to the list.
 * 
 * \param name
 * OSC-pattern with the name of the node/container.
 * 
 * \param isAlias
 * True if added pointer is only an alias (will not be deleted when cleaning up).
 * 
 * \see
 * WOscContainer::addMethod(WOscMethod* method, char* name, bool isAlias)
 */
void
WOscContainer::AddContainer(WOscContainer* container, const char* name, bool isAlias)
{
	WOscContainerEntry entry;
	entry.m_name = name;
	entry.m_isAlias = isAlias;
	entry.m_container = container;
	m_containerEntries.push_back(entry);
}

/** Tries to find a certain container by its OSC-address.
 * \b Not pattern but address (/hello/world/whats/up).
 * 
 * \param address
 * String containing OSC-address to be searched for.
 * 
 * \returns
 * Pointer to container if found else NULL.
 *
 * \todo
 * Verify/test this function.
 */
const WOscContainer*
WOscContainer::FindContainer(const char* address) const
{
	unsigned int addrlen = strlen(address);
	
	if ( addrlen == 0 )						// no address
		return NULL;
	if ( address[0] != '/' )				// invalid address
		return NULL;
	if ( addrlen == 1 && *address == '/' )	// root container "/"
		return this;
	
	// find next slash (if any)
	const char* next = NextSlashOrNull(address+1);
	
	if ( *next == 0 ) {
		// leaf
		for (int i = m_containerEntries.size()-1; i >= 0; i-- )
			if ( m_containerEntries[i].m_name == (address+1) )
				return m_containerEntries[i].m_container;
	} else {
		// not a leaf. just compare node name
		unsigned int thisNodeStrLen = next-address;
		for ( int i = m_containerEntries.size()-1; i >= 0; i-- ) {
			if ( strncmp(m_containerEntries[i].m_name.c_str(), address+1, thisNodeStrLen) == 0 ) {
				const WOscContainer* ctnr = m_containerEntries[i].m_container->FindContainer(next);
				if ( ctnr )
					return ctnr;
			}
		}
	}
		
	return NULL;
}

/** Removes child container or container alias by name (not address).
 *
 */
bool
WOscContainer::RemoveContainer(const char* name)
{
	for ( int i = m_containerEntries.size()-1; i >= 0; i-- )
		if ( m_containerEntries[i].m_name == name ) {
			// deallocated only if not an alias.
			if ( ! m_containerEntries[i].m_isAlias )
				delete m_containerEntries[i].m_container;
			m_containerEntries.erase(m_containerEntries.begin() + i);
			return true;
		}
	return false;
}

/** Removes child method or container alias by name (not address).
 *
 */
bool
WOscContainer::RemoveMethod(const char* name)
{
	for ( int i = m_methodEntries.size()-1; i >= 0; i-- )
		if ( m_methodEntries[i].m_name == name ) {
			// deallocated only if not an alias.
			if ( ! m_methodEntries[i].m_isAlias )
				delete m_methodEntries[i].m_method;
			m_methodEntries.erase(m_methodEntries.begin() + i);
			return true;
		}
	return false;
}

/** Creates a list of methods which match the address of the message passed
 * as argument. Those lists reflect the effect of an OSC-message.
 * The list can then be used to invoke all matching methods.
 * 
 * \param msg
 * Message to be scanned for.
 * 
 * \returns
 * A list containing all matching functions. Its the task of the list if none
 * are found and an invocation is issued. The caller must free the returned
 * list.
 * 
 * \throws WOscException
 * When dispatching is not executed from a root container.
 * (can be removed by first searching for root. discussion?).
 *
 * \remarks
 * Code stolen from Matt Wright's OSC-Kit and adapted to this architecture (\b Thank You!).
 *
 * \see
 * WOscContainer::dispatchSubMessage(char* pattern)
 */
void
WOscContainer::DispatchMessage(WOscCallbackList& methods, WOscMessage* msg)
{
	if ( m_parent != NULL )
		throw WOscException(ERR_NOT_DISPATCHING_FROM_ROOT, __FILE__, __LINE__);

	const char* pattern = msg->GetOscAddress().GetBuffer();

	if ( pattern == NULL || pattern[0] != '/')
		throw WOscException(ERR_INVALID_ADDRESS_NO_SLASH, __FILE__, __LINE__);

	// dispatch address pattern (note: leading slash removed!)
	DispatchSubMessage(methods, pattern+1);
}

/** Creates a list of methods which match the address passed as argument.
 * 
 * \param pattern
 * OSC-sub-address to be scanned for. If the first pattern matches a child the remaining
 * address will be passed to that child (recursion).
 * \returns
 * A list of methods matching the pattern. The returned list must be deleted by
 * the caller
 * \remarks
 * Code stolen from Matt Wright's OSC-Kit and adapted to this architecture (\b Thank You!).
 * \see
 * WOscContainer::DispatchMessage(WOscMessage* msg)
 */
void
WOscContainer::DispatchSubMessage(WOscCallbackList& methods, const char* pattern)
{
	const char *nextSlash = NextSlashOrNull(pattern);
	if (*nextSlash == '\0') {
		/* Leave case: the pattern names methods of this container.
		*/
		for (int i = m_methodEntries.size()-1; i >= 0 ; i--)
			if (PatternMatch(pattern, m_methodEntries[i].m_name.c_str()))
				// method found -> add to method list
				methods.Append(m_methodEntries[i].m_method);
	} else {
		/* Recursive case: in the middle of an address, so the job at this step
		** is to look for containers that match.  We temporarily turn the next
		** slash into a null so pattern will be a null-terminated string of the
		** stuff between the slashes.
		**
		** Note that we're temporary modifying a constant buffer. This is not
		** clean but since we're fixing it afterwards this buffer is const when
		** this function is regarded as a black box.
		*/
		*((char*)nextSlash) = '\0';	// temporary zero termination
		const char *restOfPattern = nextSlash + 1;
		for (int i = m_containerEntries.size()-1; i >= 0; i--)
			if (PatternMatch(pattern, m_containerEntries[i].m_name.c_str()))
				m_containerEntries[i].m_container->DispatchSubMessage(methods, restOfPattern);
		*((char*)nextSlash) = '/';	// reverting temporary zero termination
	}
}

/** Returns the not aliased name (OSC-pattern, \b not address) of this container.
 * 
 * \returns
 * WOscString containing the name-pattern of this container.
 */
WOscString
WOscContainer::GetName() const
{
	if ( m_parent != NULL ) {
		/* search all children of parent for not aliased name */
		const std::vector<WOscContainerEntry>& entries = m_parent->m_containerEntries;
		for ( int i = entries.size()-1; i >= 0; i-- )
			if ( (entries[i].m_container == this) && (!(entries[i].m_isAlias))){
				return WOscString(entries[i].m_name.c_str());
				break;
			}
	}
	return WOscString("/");
}

/** Traverses the container-tree upwards until root is reached and
 * assembles its unaliased address.
 * 
 * \returns
 * OSC-address of this container.
 *
 * \remarks
 * OSC-addresses of containers end with a slash ('\\'),
 * OSC-addresses of methods end with nothing.
 */
WOscString
WOscContainer::GetAddress() const
{

	if ( m_parent == NULL)
		return WOscString("/");

	WOscContainer* momentaryParent = m_parent;
	WOscString address = GetName();
	address += '/';

	/* traverse tree upwards until root is reached */
	while ( momentaryParent != NULL ) {
		address = momentaryParent->GetName() + address;
		momentaryParent = momentaryParent->m_parent;
	}
	
	return address;
}

/** Returns a line-break('\\n')-separated list of all OSC-addresses of 
 * methods as seen from root.
 * 
 * \returns
 * WOscString containing the list.
 *
 * \remarks
 * OSC-addresses of containers end with a slash ('\\'),
 * OSC-addresses of methods end with nothing.
 */
WOscString
WOscContainer::GetAddressSpace() const
{
	const WOscContainer* root = FindRoot();
	return root->GetMethodList();
}

/** Scans for all local methods and aliased methods.
 * 
 * \returns
 * A list as described in WOscContainer::getAddressSpace() containing
 * the addresses of all method-leaves reachable from this node/container.
 */
WOscString
WOscContainer::GetMethodList() const
{
	WOscString methodList;

	// scan for local methods
	for ( int m = m_methodEntries.size()-1; m >= 0; m-- ){
		if ( m_methodEntries[m].m_isAlias ) {
			methodList += GetAddress();
			methodList += m_methodEntries[m].m_name.c_str();
		} else {
			methodList += m_methodEntries[m].m_method->GetAddress();
		}
		methodList += '\n';
	}

	// scan children containers
	for ( int c = m_containerEntries.size()-1; c >= 0; c-- ) {
		if ( m_containerEntries[c].m_isAlias ) {
			/** \todo Does not handle aliased containers !!!! */
		} else {
			methodList += m_containerEntries[c].m_container->GetMethodList();
		}
	}
	
	return methodList;
}

/** Traverses the container-tree upwards until root is reached.
 * 
 * \returns
 * The pointer of the root-container.
 */
const WOscContainer*
WOscContainer::FindRoot() const
{

	const WOscContainer* momentaryParent = m_parent;
	const WOscContainer* lastParent = this;

	/* traverse tree upwards until root is reached */
	while ( momentaryParent != NULL ){
		lastParent = momentaryParent;
		momentaryParent = momentaryParent->m_parent;
	}
	return lastParent;
}

/** Checks if the current container is the root-container.
 * 
 * \returns
 * True if it is, false else.
 */
bool
WOscContainer::IsRoot() const
{
	return m_parent == NULL ? true : false;
}

void
WOscContainer::RecursivelyRemoveAliases(WOscContainer* container)
{
	// search children backwards since we're erasing
	for ( int i = m_containerEntries.size()-1 ; i >= 0; i-- ) {
		WOscContainerEntry& entry = m_containerEntries[i];
		// remove only when alias, else search
		if ( entry.m_isAlias ) {
			// remove alias only when it matches the container which alias'
			// we'd like to remove
			if ( entry.m_container == container )
				m_containerEntries.erase(m_containerEntries.begin()+i);
		} else {
				entry.m_container->RecursivelyRemoveAliases(container);
		}
	}
}

void
WOscContainer::RecursivelyRemoveAliases(WOscMethod* method)
{
	// search own methods
	for ( int i = m_methodEntries.size()-1 ; i >= 0; i-- ) {
		WOscMethodEntry& entry = m_methodEntries[i];
		// remove only when alias, else search
		if ( entry.m_isAlias ) {
			// remove alias only when it matches the container which alias'
			// we'd like to remove
			if ( entry.m_method == method )
				m_methodEntries.erase(m_methodEntries.begin()+i);
		}
	}
	// search methods of children
	for ( int i = m_containerEntries.size()-1 ; i >= 0; i-- )
		m_containerEntries[i].m_container->RecursivelyRemoveAliases(method);
}
