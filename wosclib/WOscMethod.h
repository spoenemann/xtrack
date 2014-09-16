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

/** WOscMethod , WOscCallbackList and WOscCallbackListItem header file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2009-01-13 17:23:19 $
 * $Revision: 1.5 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#ifndef __WOSCMETHOD_H__
#define __WOSCMETHOD_H__

#include "WOscNetReturn.h"
#include "WOscLib.h"
#include "WOscMessage.h"
#include "WOscTimeTag.h"
#include "WOscContainer.h"
#include "WOscInfo.h"

/*  -----------------------------------------------------------------------  */
/* 	WOscMethodInfo                                                           */
/*  -----------------------------------------------------------------------  */

/** Information about an OSC method.
 * When queried for information about a method, this class defines it.
 *
 */
class WOSC_EXPORT WOscMethodInfo: public WOscInfo
{
public:
	WOscMethodInfo(const char* description);
	WOscMethodInfo();
};

/*  -----------------------------------------------------------------------  */
/* 	WOscMethod                                                               */
/*  -----------------------------------------------------------------------  */

class WOscContainer;

/** OSC callback methods are objects that are mapped to a specific OSC-address.
 * Very similar to OSC-containers they are children of any container.
 *
 * If during dispatching and subsequent invoking an OSC-method gets called, the
 * message, which produced the invokation, and a network-return-address is passed.
 *
 * Methods can be seen as remote functions. Multiple computers communicate remotely 
 * by calling each others functions and passing arguments.
 *
 * Inherit your own methods from this class or - when using WOscReceiver - from WOscReceiverMethod.
 * 
 * \see
 * WOscCallbackList | WOscCallbackListItem | WOscReceiverMethod
 */
class WOSC_EXPORT WOscMethod
{
public:

	WOscMethod(WOscContainer* parent, const char* name,
			WOscMethodInfo* info = NULL);

	/** The OSC method.
	 * This is the function that will be called, when a certain OSC-address in a message
	 * matches the address of this method-object. The message is then passed together
	 * with a network-return-address and a timeTag which contains the point in time of the
	 * invocation.
	 *
	 * Each message object can access the program resources without global variables
	 * by using the context-pointer.
	 *
	 * \param message
	 * Message causing this call (eventually containing arguments).
	 *
	 * \param when
	 * Time of invocation (should be current time, when a priority queue, system time and
	 * a fast update-rate is used.
	 *
	 * \param networkReturnAddress
	 * The network address where the message came from.
	 *
	 * \throws
	 * Not in this, but perhaps in inherited classes and in their function-calls.
	 */
	virtual void Method(const WOscMessage* message, const WOscTimeTag& when,
			const WOscNetReturn* networkReturnAddress) = 0;
	void AddMethodAlias(WOscContainer* parent, const char* alias);
	WOscContainer* GetParent();

	/* Address related functions */
	WOscString GetName();
	WOscString GetAddress();
	WOscString GetAddressSpace();
	/** Retrieve the proprietary info which can be set in the constructor.
	 * NULL if not set.
	 */
	const WOscMethodInfo* GetInfo() const { return m_info; }

protected:
	/** WOscContainer controls the deletion of registered methods. */
	friend class WOscContainer;
	/** Destructor. A WOscMethod can be deleted only by WOscContainers. Does
	 * nothing currently. */
	virtual ~WOscMethod() { delete m_info; }

private:
	WOscMethod();				/**< Disabled default constructor.*/
	WOscMethod(WOscMethod &rhs);/**< Disabled copy constructor.*/

	WOscContainer*	m_parent;	/**< Parent container.*/
	WOscMethodInfo*	m_info;		/**< Info object.*/
};


#include <string.h>	// memcpy

/** Callback list. Gets filled with matching methods from an OSC address space.
 * Adjusts its size as soon its capacity is too low but keeps its size to avoid
 * unnecessary memory allocations.
 */
class WOscCallbackList
{
private:
	WOscMethod**	m_methods;
	unsigned int	m_count;
	unsigned int	m_capacity;
	/** Doubles the internal capacity and copies the old elements over to the
	 * new (bigger) internal buffer.
	 */
	void Double()
	{
		WOscMethod** newMethods = new WOscMethod*[m_capacity*2];
		memcpy(newMethods, m_methods, sizeof(WOscMethod*)*m_capacity);
		m_capacity *= 2;
		delete [] m_methods;
		m_methods = newMethods;
	}
public:
	/** Constructor.
	 * @param initialCapacity
	 * Initial (default) capacity of the call-back list.
	 */
	WOscCallbackList(unsigned int initialCapacity = 32)
	{
		m_methods = new WOscMethod*[initialCapacity];
		m_capacity = initialCapacity;
		m_count = 0;
	}
	/** Destructor. */
	virtual ~WOscCallbackList()
	{
		delete [] m_methods;
	}
	/** Just sets the number of callback items to zero but keeps the allocated
	 * buffer which is reused the next time items are filled into the call-back
	 * list.
	 */
	void Reset() { m_count = 0; }
	/** Append a method to the call-back list. */
	void Append(WOscMethod* method)
	{
		if ( m_count >= m_capacity )
			Double();
		m_methods[m_count++] = method;
	}
	void Invoke(const WOscMessage* message, const WOscTimeTag& when,
			const WOscNetReturn* networkReturnAddress)
	{
		WOscMethod** method = m_methods;
		for ( int i = m_count-1; i >= 0; i-- )
			(*(method++))->Method(message, when, networkReturnAddress);
	}

	/** Returns the number of items in the call-back list. */
	unsigned int GetCount() const { return m_count; }
};


#endif	// #ifndef __WOSCMETHOD_H__

