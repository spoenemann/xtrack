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

/**  WOscBundle header file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2006-05-16 21:21:41 $
 * $Revision: 1.2 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#ifndef	__WOSCBUNDLE_H__
#define __WOSCBUNDLE_H__

#include "WOscPriorityQueue.h"
#include "WOscLib.h"
#include "WOscMessage.h"
#include "WOscPacket.h"
#include "WOscException.h"

#include <string.h>		// strncmp
#include "WOscUtil.h"	// WOscUtil::BufferTo<uint32_t>

/** Every bundle begins with its tag to identify it as bundle.
 *
 * In string representation it is "#bundle".
 *
 * \remarks
 * Per definition the tag is an OSC-string, which means that
 * its length has to be a multiple of 4 (an if not rit has to be rounded
 * up an has to be padded with zeros) what applies for this tag too.
 * But if allocated as a string this tag automatically has the length 8.
 */
#define WOSC_BUNDLE_TAG				"#bundle"
#define WOSC_BUNDLE_HEADER_SIZE		(8 + sizeof(uint64_t))

/** Raw bundle parsing.
 * Allows analysis of bundles directly from a binary stream. If a data packet
 * has been received the buffer can be casted to this class and the resulting
 * object can be analyzed, e.g.
 *
 *     WOscBundleRaw* bundle = reinterpret_cast<WOscBundleRaw*>(data);
 *
 */
class WOSC_EXPORT WOscBundleParser
{
private:
	/** Private bundle representation. */
	class WOscBundleRaw
	{
	public:
		/** Raw bundle tag. */
		char	m_tag[8];
		/** Raw time tag (network endianess). */
		char	m_timeTag[sizeof(uint64_t)];
		/** The bundle data consists of one or more chunks which again are
		 * assembled from a 32 bit size followed by an amount of data of this
		 * size. Therefore a chunk has the size sizeof(size)+size. */
		char	m_bundleData[];
	};
	/** Pointer to the bundle data. Gets initialized when parsing was
	 * successful. */
	const WOscBundleRaw*	m_bundle;
	/** Real binary length of the bundle including header and data. */
	unsigned int			m_bundleLen;
	/** Number of items (bundles or messages) in the bundle. */
	unsigned int			m_nItems;

	// state machine
	/** Next item index to fetch. */
	unsigned int			m_nextItem;
	/** Offset of next item from WOscBundleRaw::m_bundleData. */
	unsigned int			m_offset;
public:

	WOscBundleParser()
	{
		memset(this, 0, sizeof(WOscBundleParser));
	}
	void ResetStateMachine()
	{
		m_nextItem = 0;
		m_offset = 0;
	}
	void Clone(char** buffer, unsigned int* bufferLen) const
	{
		if ( m_bundle == NULL )
			throw WOscException(ERR_CAN_NOT_CLONE_UNINITIALIZED_BUNDLE, __FILE__, __LINE__);

		// Copy binary data.
		*buffer = new char[m_bundleLen];
		*bufferLen = m_bundleLen;
		memcpy(*buffer, m_bundle->m_bundleData, m_bundleLen);
	}
	/** Check if bundle layout correct and extract information required to
	 * iterate through all bundle items.
	 */
	void Parse(const char* data, unsigned int len)
	{
		// reset all members to zero (0)
		memset(this, 0, sizeof(WOscBundleParser));

		const WOscBundleRaw* bundle = reinterpret_cast<const WOscBundleRaw*>(data);

		unsigned int dataLen = len - WOSC_BUNDLE_HEADER_SIZE;
		if ( dataLen > len )	// if wrapped
			throw WOscException(ERR_TOO_SHORT, __FILE__, __LINE__);
		if ( dataLen == 0 )
			throw WOscException(ERR_EMPTY_BUNDLE, __FILE__, __LINE__);
		if ( strncmp(bundle->m_tag, WOSC_BUNDLE_TAG, sizeof(bundle->m_tag)) != 0 )
			throw WOscException(ERR_INVALID_TAG, __FILE__, __LINE__);

		// check if layout valid by stepping through bundle
		unsigned int offset = 0;
		for ( ; offset + sizeof(int32_t) < dataLen; m_nItems++ ) {
			int itemLen = WOscUtil::BufferTo<int32_t>(bundle->m_bundleData+offset);
			offset += itemLen + sizeof(uint32_t);
		}
		if ( offset != dataLen ) {
			throw WOscException(ERR_CURRUPT_BUNDLE_LAYOUT, __FILE__, __LINE__);
		}
		if ( m_nItems == 0 )
			throw WOscException(ERR_NO_ITEMS_IN_BUNDLE, __FILE__, __LINE__);
		m_bundle = bundle;
		m_bundleLen = len;
	}
	/** Get the time tag. Bundle must have been parsed successfully before. */
	WOscTimeTag GetTimeTag() const
	{
		if ( m_bundle == NULL )
			return WOscTimeTag::GetImmediateTime();
		return WOscTimeTag(reinterpret_cast<const char*>(&m_bundle->m_timeTag));
	}
	/** Return the number of items in the bundle. */
	unsigned int GetItemCount() const { return m_nItems; }
	/** Get the next item in the bundle.
	 * @param buffer
	 * Pointer which receives a pointer to the internal data of the next item.
	 * @param length
	 * Pointer which receives the length of the next item in bytes.
	 * @return
	 * True if there was an item left, false if no items left.
	 */
	bool GetNextItem(const char** buffer, unsigned int* length)
	{
		unsigned int dataLen = m_bundleLen - WOSC_BUNDLE_HEADER_SIZE;
		if ( m_nextItem >= m_nItems || m_offset >= dataLen )
			return false;

		*length = WOscUtil::BufferTo<int32_t>(m_bundle->m_bundleData + m_offset);
		*buffer = m_bundle->m_bundleData + m_offset + sizeof(uint32_t);
		m_offset += *length + sizeof(uint32_t);
		m_nextItem++;

		return true;
	}
};

class WOSC_EXPORT WOscQueueBundle : public WOscQueueItem
{
public:
	WOscQueueBundle(const WOscBundleParser& bundle, WOscNetReturn* ra) :
		WOscQueueItem(bundle.GetTimeTag(), ra)
	{
		m_bundleData = NULL;
		m_bundleDataLen = 0;
		bundle.Clone(&m_bundleData, &m_bundleDataLen);
	}
	virtual ~WOscQueueBundle()
	{
		delete [] m_bundleData;
	}
	const char* GetData() const { return m_bundleData; }
	unsigned int GetDataLen() const { return m_bundleDataLen; }
private:
	WOscQueueBundle();
	/** Copy of the data. Without copy we can not assure that the data is
	 * not deallocated while this bundle sits in the queue.
	 */
	char*				m_bundleData;
	unsigned int		m_bundleDataLen;
};

/** An OSC bundle bundles messages and bundles together which
 * have to be invoked at the same time.
 * 
 *
 * \remarks
 * A bundle element (Message or Bundle) is saved internally
 * as a reference. If the element is not removed by GetBundle
 * or GetMessage, the destructor cleans them up.
 * This speeds up the passing around of elements because
 * only the references (pointers) have to be managed/copied.
 * 
 * \see
 * None.
 */
class WOSC_EXPORT WOscBundle: public WOscTimeTag, public WOscPacket
{

public:
	/* construct for transmission */
	WOscBundle();
	WOscBundle(WOscTimeTag timeTag);

	virtual ~WOscBundle();
	void Reset();
	void Add(WOscBundle* bundle);
	void Add(WOscMessage* message);

	int GetNumMessages();
	int GetNumBundles();

	WOscMessage* GetMessage(int idx);
	WOscBundle* GetBundle(int idx);

	virtual void GetBuffer(char* buffer, int bufferLen);
	virtual const char* GetBuffer();
	virtual int GetBufferLen();
	
	/** Bundle related constants.
	 */
	enum Constants{
		END_OF_BUFFER = -1,		/**< Return value for CheckSize(int relReadPos, int dataLen, const char* rawData) .*/
		BUNDLE_HEADER_SIZE = 8,	/**< Size of the "#bundle\0" header at the beginnning of every bundle.*/
		SIZE_SIZE = 4,			/**< Size of bundle the bundle "size"-field which comes before every bundle element.*/
	};

private:
	void RemoveBundle(int idx);
	void RemoveMessage(int idx);

	void InitEmpty();
	void GenerateBufferFromElements();
	int CheckSize(int relReadPos, int dataLen, const char* rawData);

	WOscMessage**	m_messages;		/**<Array of message elements in this bundle.*/
	int				m_numMessages;	/**<Number of message elements in this bundle. */

	WOscBundle**	m_bundles;		/**<Array of bundle elements in this bundle.*/
	int				m_numBundles;	/**<Number of bundle elements in this bundle.*/

	char*			m_buffer;		/**<The buffer which represents the binary version of this bundle.*/
	int				m_bufferSize;	/**<Size of the buffer.*/

	WOscBundle(WOscBundle& rhs); /**< Disabled.*/
	WOscBundle& operator=(const WOscBundle& rhs); /**< Disabled.*/
};

#endif	// #ifndef	__WOSCBUNDLE_H__
