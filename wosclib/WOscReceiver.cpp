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

/** WOscReceiver source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2007-11-30 00:27:08 $
 * $Revision: 1.6 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#include "WOscReceiver.h"

 #if WOSC_USE_PRIORITY_QUEUE == 1
/** Constructor.
 * Constructs a receiver and initilizes its priority-
 * queue.
 * 
 * \param initialQueueCapacity
 * Optional size of priority queue. Grows automatically quadratically if
 * too small.
 * 
 * \param systemTime
 * Optional system-time object (inherited class from WOscSystemTime) which
 * supplies the receiver with system dependent time information. Passed
 * objects will be deleted by the receiver object destructor.
 */
WOscReceiver::WOscReceiver(int initialQueueCapacity,
		WOscSystemTime* systemTime) : WOscPriorityQueue(initialQueueCapacity)
#else
/** Constructor.
 * Constructs a receiver and initilizes its priority-
 * queue.
 * 
 * \param systemTime
 * Optional system-time object (inherited class from WOscSystemTime) which
 * supplies the receiver with system dependent time information. Passed
 * objects will be deleted by the receiver object destructor.
 */
WOscReceiver::WOscReceiver(WOscSystemTime* systemTime)
#endif	//  #if WOSC_USE_PRIORITY_QUEUE == 1
/* init inherited class */
{
	/* reset address space */
	m_addrSpaceRootContainer = NULL;

	/* set system time generator to NULL when not specified*/
	m_systemTime = systemTime;
}

/** Cleans up the receiver.
 * All elements which are not removed from the priority-queue
 * get deleted automatically.
 */
WOscReceiver::~WOscReceiver()
{
	if ( m_systemTime ) 
		delete m_systemTime;
}

/** Sets the address-space of the WOscReceiver.
 * The address-space can be modified dynamically and externally, 
 * desired, if the root-container remains valid.
 * 
 * \param addrSpaceRootContainer
 * Root-container. Can not be a child.
 * 
 * \throws WOscException
 * If root-container is not root.
 * 
 * \remarks
 * If currently no address-space should be available, pass NULL. 
 * This disables the invokation of messages.
 */
void
WOscReceiver::SetAddressSpace(WOscContainer* addrSpaceRootContainer)
{
	if ( ! addrSpaceRootContainer->IsRoot() )
		throw WOscException(ERR_NOT_ROOT_CONTAINER, __FILE__, __LINE__);
	m_addrSpaceRootContainer = addrSpaceRootContainer;
}

/** Returns the internal address-space reference.
 * 
 * \returns
 * Internal address-space, if there is none NULL.
 */
WOscContainer*
WOscReceiver::GetAddressSpace()
{
	return m_addrSpaceRootContainer;
}

/** Parses a received packet, constructs messages and/or bundles of it and
 * passes it to the invocation-engine, which scans the address-space for
 * matching methods, takes care of time-tags and queues bundles, which have to
 * be delayed, in the priority-queue.
 * 
 * \param data
 * Binary OSC-packet data.
 * 
 * \param dataLen
 * Length of binary OSC-packet data.
 * 
 * \param networkReturnAddress
 * Network packet origin.
 * 
 * \throws
 * None, but if an internal exception occurs, the user-defined
 * exception-handler is called, see below.
 * 
 * \see
 * handleOffendingPackets(const char* const data, int dataLen, WOscNetReturn networkReturnAddress, WOscException* exception)
 */
void
WOscReceiver::NetworkReceive(const char* data, int dataLen,
		WOscNetReturn* networkReturnAddress)
{
	/* check data length to not read wrong data 
	 * when guessing OSC-object
	 */
	if ( dataLen < 1 )
		return;

	// Guess if bundle or message. If bundle, the return address gets managed
	// by the bundle and priority-queue. If not, the return-address has to be
	// deleted manually after the message has been processed.
	if ( data[0] == '#' ){
		// probably a bundle. We try to parse it here.
		try {
			ProcessBundle(data, dataLen, networkReturnAddress);
		} catch(const WOscException& exception) {
			// call user defined exception handler
			HandleOffendingPackets(data, dataLen, exception);
			// clean up (no message constructed -> netreturn has to be deleted manually
			networkReturnAddress->RemoveParent();
			return;
		}
	} else {
		// probably a message -> invoke
		try{
			ProcessMessage(data, dataLen, networkReturnAddress);
		} catch(const WOscException& exception) {
			// call user defined exception handler
			HandleOffendingPackets(data, dataLen, exception);
			// clean up (no message constructed -> netreturn has to be deleted manually
			networkReturnAddress->RemoveParent();
			return;
		}

		/* clean up return address.
		 * if received OSC-object is not a bundle the network-return address 
		 * is not managed by it and has to be deleted manually.
		 */
		networkReturnAddress->RemoveParent();
	}
}

/** Internal bundle processing.
 * Checks time-tag of bundle, if not ready it is queued in
 * the priority queue. Else it is unbundled, the contained
 * messages are invoked and internal bundles are passed to
 * to processBundle(WOscBundle* bundle) again (recursion).
 *
 * \param bundle
 * The bundle which has to be processed.
 * 
 * \throws WOscException
 * When parsing bundles or messages.
 * 
 */
void
WOscReceiver::ProcessBundle(const char* data, unsigned int len,
		WOscNetReturn* networkReturnAddress)
{
	WOscBundleParser bundle;
	bundle.Parse(data, len);

#if WOSC_USE_PRIORITY_QUEUE == 1
	// check time-tag
	if ( bundle.GetTimeTag() <= WOscTimeTag::GetCurrentTime(m_systemTime) ) {
#endif
		const char* buffer;
		unsigned int bufferLength;
		while ( bundle.GetNextItem(&buffer, &bufferLength) ) {
			if ( bufferLength > 0 ) {
				// check if bundle in bundle
				if ( buffer[1] == '#' ) {
					ProcessBundle(buffer, bufferLength, networkReturnAddress);
				} else {
					ProcessMessage(buffer, bufferLength, networkReturnAddress);
				}
			} else {
				// buffer length zero throw exeption
			}
		}
		// we processed this bundle so we remove the return address.
		networkReturnAddress->RemoveParent();

#if WOSC_USE_PRIORITY_QUEUE == 1
	} else {
		// the bundle is not ready, queue it in the priority-queue.
		InsertItem( new WOscQueueBundle(bundle, networkReturnAddress) );
	}
#endif
}
/** Internal message processing.
 * The message is passed to the address-space and a
 * call-back-list is created and invoked.
 * 
 * \param msg
 * Message to be processed.
 * 
 * \param networkReturnAddress
 * The network-origin of this message.
 * 
 * \throws WOscException
 * Message internal exceptions, there should be none.
 * Internal exceptions point to library bugs.
 * 
 * \remarks
 * The message passed to this function will be deleted when
 * processed.
 */
void
WOscReceiver::ProcessMessage(const char* data, unsigned int len, const WOscNetReturn* networkReturnAddress)
{
	WOscMessage* msg = new WOscMessage(data, len);

	try {
		WOscTimeTag currentTime = WOscTimeTag::GetCurrentTime(m_systemTime);
	
		if ( m_addrSpaceRootContainer ){
			/* get matching methods */
			m_callbackList.Reset();
			m_addrSpaceRootContainer->DispatchMessage(m_callbackList, msg);
			// check if matching methods found
			if ( m_callbackList.GetCount() == 0 ) {
				// if not, handle message in special handler
				HandleNonmatchedMessages(msg, networkReturnAddress);
			} else {
				m_callbackList.Invoke(msg, currentTime, networkReturnAddress);
			}
		}
	} catch (const WOscException& exception) {
		// message processing failed. delete message and throw exception to
		// next handler.
		delete msg;
		throw exception;
	}
	// message done, clean up message
	delete msg;
}

#if WOSC_USE_PRIORITY_QUEUE == 1
/** Keep the internal priority-queue running.
 * If there are any ready-to-process-elements, 
 * they are removed from the priority-queue and
 * processed.
 * 
 * \throws WOscException
 * Bundle or message internal exceptions, there should be none.
 * Internal exceptions point to library bugs.
 * 
 * \remarks
 * Call this method perodically to keep the priority-queue running.
 */
void
WOscReceiver::ManagePriorityQueue()
{
	// run as long there is something ready
	while (	(GetEarliestTimeTag() < WOscTimeTag::GetCurrentTime(m_systemTime) ) && (GetNumItems() > 0) ) {
		WOscQueueBundle* bundle = NULL;
		WOscNetReturn* netReturn = NULL;
		try {
			// get element from queue
			WOscQueueBundle* bundle = static_cast<WOscQueueBundle*>(RemoveEarliestItem());
			// remove network return address from it
			netReturn = bundle->RemoveNetworkReturnAddress();
			// process it (if it runs through properly also the reference
			// counter of netReturn gets decremented
			ProcessBundle(bundle->GetData(), bundle->GetDataLen(), netReturn);

		} catch(const WOscException& exception) {
			// call user defined exception handler
			HandleOffendingPackets(NULL, 0, exception);
			if ( netReturn )
				netReturn->RemoveParent();
		}
		delete bundle;
	}
}

#endif	// #if WOSC_USE_PRIORITY_QUEUE == 1
