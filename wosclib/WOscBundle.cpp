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

/** WOscBundle source file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2006-03-20 19:41:37 $
 * $Revision: 1.2 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */

#include "WOscBundle.h"
#include "WOscException.h"
#include "WOscUtil.h"
#include <string.h>

/** Construct an empty bundle and tag it with a time-tag.
 * The network return address of its base-class is initialized with NULL.
 * 
 * \param timeTag
 * Tagging time-tag.
 * 
 */
WOscBundle::WOscBundle(WOscTimeTag timeTag) : WOscTimeTag(timeTag)
{ 
	InitEmpty();
}

/** Construct an empty bundle.
 * The network return address of its base-class is initialized with NULL.
 * The WOscTimeTag of the queue item will be 'immediate'.
 */
WOscBundle::WOscBundle() : WOscTimeTag(WOscTimeTag::GetImmediateTime())
{
	InitEmpty();
}

/** Destructor.
 * Cleans up all messages and bundles which have not been removed
 * by GetBundle or GetMessage .
 *
 * Cleans up the buffer when it was used (invokation of one of 
 * the getBuffer(...)-members).
 * 
 */
WOscBundle::~WOscBundle(){
	
	/* clean up buffer */
	if ( m_buffer )
		delete [] m_buffer;

	/* clean up leftover elements */
	if ( m_messages ){
		for( int i = 0; i < m_numMessages; i++ )
			delete m_messages[i];
		delete [] m_messages;
	}
	if ( m_bundles ){
		for( int i = 0; i < m_numBundles; i++ )
			delete m_bundles[i];
		delete [] m_bundles;
	}
}

void
WOscBundle::Reset()
{
	if ( m_buffer )
		delete [] m_buffer;
	
	/* clean up leftover elements */
	if ( m_messages ){
		for( int i = 0; i < m_numMessages; i++ )
			delete m_messages[i];
		delete [] m_messages;
	}
	if ( m_bundles ){
		for( int i = 0; i < m_numBundles; i++ )
			delete m_bundles[i];
		delete [] m_bundles;
	}
	InitEmpty();
}

/** Initializes an empty bundle.
 * Internal function for resetting all pointers
 * and element counters. Used by most constructors.
 * 
 */
void WOscBundle::InitEmpty(){

	/* no messages */
	m_messages = NULL;
	m_numMessages = 0;	

	/* no bundles */
	m_bundles = NULL;
	m_numBundles = 0;

	/* buffer empty */
	m_buffer = NULL;
	m_bufferSize = 0;

}

/** Generates a binary representation of this bundle.
 * The binary representation is saved to an internal buffer
 * (m_buffer). First a bundle-header is written the the size
 * and binary data of each bundle-element is added.
 * 
 */
void WOscBundle::GenerateBufferFromElements(){

	m_bufferSize = GetBufferLen();
	m_buffer = new char[m_bufferSize];
	char* wrPtr = m_buffer;

	// write header
	memcpy(wrPtr, WOSC_BUNDLE_TAG, 8);
	wrPtr += 8;

	// write timeTag
	WriteToCharArray(wrPtr);
	wrPtr += WOscTimeTag::TIME_TAG_SIZE;

	// write all messages
	for ( int i = 0; i < m_numMessages; i++ ){
		// write size
		int msgSize = m_messages[i]->GetBufferLen();
		WOscUtil::FillBufferWith(wrPtr, msgSize);
		wrPtr += SIZE_SIZE;
		// write message
		const char* temp = m_messages[i]->GetBuffer();
		memcpy(wrPtr, temp, msgSize);
		wrPtr += msgSize;
	}

	// write all bundles
	for ( int i = 0; i < m_numBundles; i++ ){
		// write size
		int bndlSize = m_bundles[i]->GetBufferLen();
		WOscUtil::FillBufferWith(wrPtr, bndlSize);
		wrPtr += SIZE_SIZE;
		// write message
		const char* temp = m_bundles[i]->GetBuffer();
		memcpy(wrPtr, temp, bndlSize);
		wrPtr += bndlSize;
	}
}

/** Adds an OSC-bundle to the bundle.
 * The internal element counter is increased and
 * the bundle is added.
 * 
 * \param bundle
 * Reference of the bundle to be added.
 * 
 */
void WOscBundle::Add(WOscBundle* bundle){
	if ( !m_bundles ){
		m_bundles = new WOscBundle*[1];
		m_bundles[0] = bundle;
		m_numBundles++;
	}else{
		m_numBundles++;
		WOscBundle** newBndlArray = new WOscBundle*[m_numBundles];
		for( int i = 0; i < m_numBundles-1; i++ )
			newBndlArray[i] = m_bundles[i];
		newBndlArray[m_numBundles-1] = bundle;

		delete [] m_bundles;
		m_bundles = newBndlArray;
	}
}

/** Adds an OSC-message to the bundle.
 * The internal element counter is increased and
 * the message is added.
 * 
 * \param message
 * Pointer of the message to be added. The bundle deallocates it on deletion.
 * 
 */
void WOscBundle::Add(WOscMessage* message){
	if ( !m_messages ){
		m_messages = new WOscMessage*[1];
		m_messages[0] = message;
		m_numMessages++;
	}else{
		m_numMessages++;
		WOscMessage** newMsgArray = new WOscMessage*[m_numMessages];
		for( int i = 0; i < m_numMessages-1; i++ )
			newMsgArray[i] = m_messages[i];
		newMsgArray[m_numMessages-1] = message;
		delete [] m_messages;
		m_messages = newMsgArray;
	}
}

/** Returns the number of messages contained in this bundle.
 * Before extracting messages, get the number of bundle-internal
 * messages to avoid access-exceptions.
 * 
 * \returns
 * The number of messages contained in this bundle.
 * 
 * \see
 * WOscBundle::GetMessage(int idx)
 */
int WOscBundle::GetNumMessages(){
	return m_numMessages;
}

/** Returns the number of bundles contained in this bundle.
 * Before extracting bundles, get the number of bundle-internal
 * bundles to avoid access-exceptions.
 * 
 * \returns
 * The number of bundles contained in this bundle.
 * 
 * \see
 * WOscBundle::GetBundle(int idx)
 */
int WOscBundle::GetNumBundles(){
	return m_numBundles;
}

/** Returns a reference of a specific message contained in the bundle.
 * Removes the reference from the internal reference array.
 * 
 * \param idx
 * Index of the message in the message array. The number of messages
 * contained in this array can be queried by GetNumMessages() .
 * 
 * \returns
 * Reference of the element with index 'idx'
 * 
 * \throws WOscException
 * If index lies out of the array bounds (i.e. < 0 and >= GetNumMessages())
 * 
 * \remarks
 * Messages which were removed with this interface will not deleted
 * by the bundles destructor. The caller must keep track of them.
 * 
 * \see
 * GetNumMessages()
 */
WOscMessage* WOscBundle::GetMessage(int idx){
	if ( idx < 0 || idx >= m_numMessages )
		throw WOscException(ERR_INVALID_INDEX, __FILE__, __LINE__);
	else{
		WOscMessage* retMsg = m_messages[idx]; // copy message reference
		RemoveMessage(idx); // remove message from reference list
		return retMsg; // return reference
	}
}

/** Returns a reference of a specific sub-bundle contained in the bundle.
 * Removes the reference from the internal reference array (the desctructor
 * does not remove it when the bundle is destoyed).
 * 
 * \param idx
 * Index of the subbundle in the subbundle array. The number of subbundles
 * contained in this array can be queried by WOscBundle::GetNumBundles() .
 * 
 * \returns
 * Reference of the element (a subbundle) with index 'idx'.
 * 
 * \throws WOscException
 * If index lies out of the array bounds (i.e. < 0 and >= GetNumBundles())
 * 
 * \remarks
 * Subbundles which were removed with this interface will not deleted
 * by the bundle-destructor. The caller must keep track of them.
 * 
 * \see
 * WOscBundle::GetNumBundles() | WOscMessage* WOscBundle::GetMessage(int idx)
 */
WOscBundle* WOscBundle::GetBundle(int idx){
	if ( idx < 0 || idx >= m_numBundles )
		throw WOscException(ERR_INVALID_INDEX, __FILE__, __LINE__);
	else{
		WOscBundle* retBundle =  m_bundles[idx]; // copy bundle reference
		RemoveBundle(idx); // remove bundle from reference list

		/* return reference */
		return retBundle;
	}
}


/** Fills the buffer with the raw bytestream of this packet.
 * 
 * \param buffer
 * Description of parameter buffer.
 * 
 * \param bufferLen
 * Description of parameter bufferLen.
 * 
 * \throws <exception class>
 * Description of criteria for throwing this exception.
 * 
 * Write detailed description for getBuffer here.
 * 
 * \remarks
 * Write remarks for getBuffer here.
 * 
 * \see
 * Separate items with the '|' character.
 */
void WOscBundle::GetBuffer(char* buffer, int bufferLen){
	if (m_buffer){
		delete [] m_buffer;
		m_buffer = NULL;
	}

	/* generate bundle buffer */
	GenerateBufferFromElements();

	/* copy buffer */
	if ( m_bufferSize > bufferLen ){
		/* clean up */
		delete m_buffer;
		m_buffer = NULL;
		m_bufferSize = 0;
		throw WOscException(ERR_BUFFER_TO_SMALL, __FILE__, __LINE__);
	}
	memcpy(buffer, m_buffer, m_bufferSize);

}

/** Generates internal buffer filled with the binary representation
 * of the bundle.
 * The buffer-size can be queried through "getBufferLen()".
 * 
 * \returns
 * A temporary pointer to the binary representation of the bundle.
 * 
 * \remarks
 * The pointer remains valid until the next time an element
 * will be added to the bundle, the bundle is destination of
 * an operation (check operators), the bundle will be deleted
 * or one of the getBuffer functions is called. If you are
 * not sure about that issue, please use the other getBuffer
 * function.
 * 
 * \see
 * WOscBundle::getBuffer(char* buffer, int bufferLen)
 */
const char* WOscBundle::GetBuffer(){

	if (m_buffer){
		delete [] m_buffer;
		m_buffer = NULL;
	}

	/* generate bundle buffer */
	GenerateBufferFromElements();

	/* user cleans up */
	return m_buffer;
}

/** Returns the required buffer size when calling WOscBundle::getBuffer(char* buffer, int bufferLen).
 * Use for determine the right buffer size when using WOscBundle::getBuffer(char* buffer, int bufferLen) 
 * or getting size of the the buffer when called char* WOscBundle::getBuffer().
 * 
 * \returns
 * Length of the required buffer size for calling 
 */
int WOscBundle::GetBufferLen(){
	int bufSize, i;

	bufSize =  BUNDLE_HEADER_SIZE + WOscTimeTag::TIME_TAG_SIZE;

	for ( i = 0; i < m_numMessages; i++ )
		bufSize += SIZE_SIZE + m_messages[i]->GetBufferLen();

	for ( i = 0; i < m_numBundles; i++ )
		bufSize += SIZE_SIZE + m_bundles[i]->GetBufferLen();

	return bufSize;
}

/** Removes the internal bundle pointer from the list (does not delete it) and
 * returns it.
 * 
 * \param idx
 * Index of the bundle to be removed.
 * 
 * \throws WOscException
 * Index negative, internal bundle-list empty or index
 * larger than number of internal bundles.
 */
void WOscBundle::RemoveBundle(int idx){

	if ( idx >= m_numBundles || m_numBundles == 0 || idx < 0 )
		throw WOscException(ERR_INVALID_INDEX, __FILE__, __LINE__);

	int i;
	
	m_numBundles--;
	
	WOscBundle **newBundles = new WOscBundle*[m_numBundles];

	for ( i = 0; i < idx; i++)
		newBundles[i] = m_bundles[i];

	for ( i = idx; i < m_numBundles; i++)
		newBundles[i] = m_bundles[i+1];

	delete [] m_bundles;
	m_bundles = newBundles;
}

/** Removes the internal message pointer from the list (does not delete it) and
 * returns it.
 * 
 * \param idx
 * Index of the message to be removed.
 * 
 * \throws WOscException
 * Index negative, internal message-list empty or index
 * larger than number of internal messages.
 */
void WOscBundle::RemoveMessage(int idx){

	if ( idx >= m_numMessages || m_numMessages == 0 || idx < 0)
		throw WOscException(ERR_INVALID_INDEX, __FILE__, __LINE__);

	int i;
	
	m_numMessages--;
	
	WOscMessage **newMessages = new WOscMessage*[m_numMessages];

	for ( i = 0; i < idx; i++)
		newMessages[i] = m_messages[i];

	for ( i = idx; i < m_numMessages; i++)
		newMessages[i] = m_messages[i+1];

	delete [] m_messages;
	m_messages = newMessages;
}


