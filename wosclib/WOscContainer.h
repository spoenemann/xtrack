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

/** WOscContainer and WOscContainerInfo header file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2009-01-13 17:23:19 $
 * $Revision: 1.6 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#ifndef __WOSCCONTAINER_H__
#define __WOSCCONTAINER_H__


#include "WOscMethod.h"
#include "WOscLib.h"
#include "WOscPatternMatch.h"
#include "WOscInfo.h"

#include <vector>
#include <string>

/*  -----------------------------------------------------------------------  */
/* 	WOscContainerInfo                                                        */
/*  -----------------------------------------------------------------------  */

/** Container/node related information.
 * 
 */
class WOSC_EXPORT WOscContainerInfo: public WOscInfo{
public:
	WOscContainerInfo(const char* description) : WOscInfo(description) { }
	WOscContainerInfo() { }
	virtual ~WOscContainerInfo() { }
};


/*  -----------------------------------------------------------------------  */
/* 	WOscContainer                                                            */
/*  -----------------------------------------------------------------------  */

class WOscMethod;	// Compatibility with GCC4 (credits: Cyril Comparon)
class WOscContainer;
class WOscCallbackList;

class WOscContainerEntry
{
public:
	WOscContainer*	m_container;
	std::string		m_name;
	bool			m_isAlias;
	WOscContainerEntry() : m_container(NULL), m_isAlias(false) { }
};

class WOscMethodEntry
{
public:
	WOscMethod*		m_method;
	std::string		m_name;
	bool			m_isAlias;
	WOscMethodEntry() : m_method(NULL), m_isAlias(false) { }
};

/** OSC Address-space node.
 * An OSC-address space consists of a tree of containers with methods as leaves.
 *
 * \todo More elaborate documentation of WOscContainer class.
 * 
 * \remarks
 * ALL CONTAINERS EXCEPT THE ROOT CONTAINER MUST BE ALLOCATED DYNAMICALLY,
 * since the root container deletes all its children during its destruction!!
 *
 * \see
 * See the examples for address-tree generation and deletion.
 */
class WOSC_EXPORT WOscContainer: public WOscPatternMatch {
public:

	/* Root */
	WOscContainer(const WOscContainerInfo& info = WOscContainerInfo("root container"), int defNumChildren = 32, int defNumMethods = 32);
	
	/* Children */
	WOscContainer(WOscContainer* parent, const char* name, const WOscContainerInfo& info = WOscContainerInfo(), int defNumChildren = 32, int defNumMethods = 32);
	
	void AddContainerAlias(WOscContainer* parent, const char* alias);

	void DispatchMessage(WOscCallbackList& methods, WOscMessage* msg);

	/* Address related functions */
	WOscString GetName() const;
	WOscString GetAddress() const;

	WOscString GetAddressSpace() const;
	WOscString GetMethodList() const;

	const WOscContainer* FindContainer(const char* address) const; // NOT pattern
	bool RemoveContainer(const char* name);
	bool RemoveMethod(const char* name);

	const WOscContainer* FindRoot() const;
	bool IsRoot() const;

	void RemoveAll();

private:
	friend class WOscMethod;
	virtual ~WOscContainer();
	void AddContainer(WOscContainer* container, const char* name, bool isAlias);
	void AddMethod(class WOscMethod* method, const char* name, bool isAlias);
	void DispatchSubMessage(WOscCallbackList& methods, const char* pattern);
	void RecursivelyRemoveAliases(WOscContainer* container);
	void RecursivelyRemoveAliases(WOscMethod* method);

    WOscContainer*		m_parent;			/**< Pointer to the parent container, NULL for root nodes.*/
    std::vector<WOscContainerEntry>	m_containerEntries;
    std::vector<WOscMethodEntry>	m_methodEntries;
	WOscContainerInfo	m_info;				/**< Information about this container.*/
};

#endif	// #ifndef __WOSCCONTAINER_H__
