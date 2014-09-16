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

/** WOscQueueItem and WOscPriorityQueue header file.
 * \file
 *
 * $Author: cls-nebadje $ ( \ref _UcfWOscLib )
 * $Date: 2006-05-16 21:21:41 $
 * $Revision: 1.4 $
 *
 * Copyright (c) Weiss Engineering Ltd
 *
 */
#ifndef __WOSCPRIORITYQUEUE_H__
#define __WOSCPRIORITYQUEUE_H__

#include "WOscNetReturn.h"
#include "WOscLib.h"
#include "WOscTimeTag.h"

/*  -----------------------------------------------------------------------  */
/* 	WOscQueueItem                                                            */
/*  -----------------------------------------------------------------------  */

/** Items which can be scheduled in a priority queue.
 * Priority-queue items are time-tags with a network return address. They get 
 * queued after their time-tag values in the priority queue.
 * 
 * \remarks
 * Any class inherited from this base class can be queued in the
 * priority-queue.
 * 
 * \see
 * WOscPriorityQueue and WOscNetReturn
 */
class WOSC_EXPORT WOscQueueItem : public WOscTimeTag
{
public:
	WOscQueueItem(const WOscTimeTag& timeTag, WOscNetReturn* ra) :
		WOscTimeTag(timeTag)
	{
		/* copy reference */
		m_ra = ra;
		/* register myself, since i'm using it */
		m_ra->AddParent();
	}
	virtual ~WOscQueueItem()
	{
		/* if it's a received packet, it should have a
		 * return address. unregister it before death
		 * of this object.
		 */
		if ( m_ra )
			m_ra->RemoveParent();
	}
	/** When removing the network return address from the queue item, the
	 * caller must decrement the reference counter himself.
	 */
	WOscNetReturn* RemoveNetworkReturnAddress()
	{
		WOscNetReturn* tmp = m_ra;
		m_ra = NULL;
		return tmp;
	}

private:
	WOscQueueItem();
	WOscQueueItem(const WOscQueueItem&);
	/** As long we own the reference counted object we have to decrement
	 * the usage counter.
	 */
	WOscNetReturn*	m_ra;
};

/*  -----------------------------------------------------------------------  */
/* 	WOscPriorityQueue                                                        */
/*  -----------------------------------------------------------------------  */
#if WOSC_USE_PRIORITY_QUEUE == 1
/** Sorts queue-items by their time tag when inserted.
 * The earliest items can be queried and/or removed. 
 * 
 * \remarks
 * The queue items are stored internally only by reference,
 * the user must control the life (allocation and deletion)
 * of the queue items.
 * 
 * \see
 * WOscQueueItem
 */
class WOSC_EXPORT WOscPriorityQueue
{
public:
	WOscPriorityQueue(int initialCapacity);
	virtual ~WOscPriorityQueue();

	void			InsertItem(WOscQueueItem *item);
	WOscQueueItem*	RemoveEarliestItem();
	WOscTimeTag		GetEarliestTimeTag();
	int				GetNumItems();

private:
	void			DoubleCapacity();
	void			RemoveItem(int idx);

	/** Pointer to the priority queue. */
	WOscQueueItem**	m_list;
	/** Number of item is the queue. */
	int				m_itemsInList;
	/** Current list capacity. Can be doubled by calling doubleCapacity(). */
	int				m_listCapacity;
};

#endif // #if WOSC_USE_PRIORITY_QUEUE == 1

#endif	// #ifndef __WOSCPRIORITYQUEUE_H__
