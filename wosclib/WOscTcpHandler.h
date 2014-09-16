/*
 * WOscTcpHandler.h
 *
 *  Created on: Oct 22, 2010
 *      Author: uli
 */

#ifndef WOSCTCPHANDLER_H_
#define WOSCTCPHANDLER_H_

#include "WOscConfig.h"

#if OS_IS_LINUX == 1 || OS_IS_MACOSX == 1 || OS_IS_CYGWIN == 1

#include <netinet/in.h>
#include <pthread.h>

#include "WOscNetReturn.h"
#include "WOscReceiver.h"
#include "WOscReceiverMethod.h"

/** The network return address. This object inherits the functionality required
 * by WOscLib to keep it alive until all messages ob a bundle are processed
 * (reference counting). The networking data structure which contains the
 * actual ip/port information is saved as a new member.
 */
class TheNetReturnAddress: public WOscNetReturn
{
public:
	TheNetReturnAddress()
	{
		memset(&m_addr, 0, sizeof(m_addr));
		m_addr.sin_family = AF_INET;
	}
	TheNetReturnAddress(const struct sockaddr_in& addr)
	{
		m_addr = addr;
		m_addr.sin_family = AF_INET;
	}
	TheNetReturnAddress(const TheNetReturnAddress& rhs)
	{
		this->m_addr = rhs.m_addr;
	}
	TheNetReturnAddress& operator=(const TheNetReturnAddress& rhs)
	{
		this->m_addr = rhs.m_addr;
		return *this;
	}
	struct sockaddr_in	m_addr;
};

class WOscTcpHandler: public WOscReceiver
{
public:
	WOscTcpHandler(bool threading = true);
	virtual ~WOscTcpHandler();
	/** Errors which can occur in the network layer. */
	typedef enum
	{
		/** No error. */
		WOS_ERR_NO_ERROR =  0,
		/** Seems that a reception thread is already running. You have to
		 * disconnect first.
		 */
		WOS_ERR_ALREADY_RUNNING,
		/** Seems that we're already connected. */
		WOS_ERR_ALREADY_CONNECTED,
		/** Socket creation failed. */
		WOS_ERR_SOCKET_CREATE,
		/** Socket connect failed. */
		WOS_ERR_SOCKET_CONNECT,
		/** Creation of receive thread failed. */
		WOS_ERR_CREATE_RX_THREAD,
		/** Socket not connected. */
		WOS_ERR_NO_SOCKET,
		/** Socket connection lost. */
		WOS_ERR_CONNECTION_LOST,
		/** Trying to close an already closed socket. */
		WOS_ERR_ALREADY_CLOSED,
		/** */
		WOS_ERR_DISCONNECTED = WOS_ERR_ALREADY_CLOSED,
		/** Tried to receive with WOscTcpClient::NetworkReceive() from a
		 * connection which has been closed. */
		WOS_ERR_CONNECTION_CLOSED,
		/** Receiving from WOscTcpClient::NetworkReceive() failed. */
		WOS_ERR_CONNECTION_FAILURE,
		/** The select/polling receive function can only be used with
		 * threading disabled.
		 */
		WOS_ERR_NOT_WHEN_THREADING,
		/** Unable to set socket into non blocking mode. */
		WOS_ERR_CAN_NOT_SET_NONBLOCKING,
		/** Invalid socket handler passed. */
		WOS_ERR_INVALID_SOCKET,
		/** User defined errors start here. */
		WOS_ERR_USER,
	} Error;
	static const char* GetErrStr(Error err);
	Error NetworkSend(const char* data, int dataLen);
	Error NetworkReceive();
	Error Start(int socketID, const TheNetReturnAddress& peer);
	Error Stop();
	/** Get Socket file descriptor for use in select or similar. */
	int GetSocketID() const { return m_socketID; }
	int GetRxThreadID() const { return m_receptionThreadID; }
	bool GetThreading() const { return m_threading; }
	const TheNetReturnAddress& GetPeer() const { return m_peer; }
protected:
	virtual void NetworkSend(const char* data, int dataLen,
			const WOscNetReturn* networkReturnAddress);
	/** The bootstrapper is our friend. */
	friend void* wosc_tcp_handler_rx_thread(void* argument);
	virtual void ReceiveThread();
	Error NetworkReceiveInternal();
private:
	/** If true, the receiver and message dispatcher runs is a separate thread.
	 * If not, the receiving must be done by either polling or running select
	 * on the socket and calling WOscTcpClient::NetworkReceive().
	 */
	bool				m_threading;
	pthread_t			m_receptionThreadID;
	int					m_socketID;
	TheNetReturnAddress m_peer;

	/** Mutex protecting the send part of the socket because the main thread
	 * code and OSC methods will use it to transmit packets to the server.
	 */
	pthread_mutex_t		m_mutexTx;

	/** Internal state machine for OSC packet reception from socket. */
	class SelectRxState
	{
	public:
		SelectRxState()
		{
			m_packetData = NULL;
			Init();
		}
		~SelectRxState()
		{
			delete m_packetData;
		}
		void Init()
		{
			delete [] m_packetData;
			m_packetData = NULL;
			m_isRxPacketLen = true;
			m_nReceived = 0;
		}
		/** Indicates whether we are currently receiving the packet length or
		 * the packet data itself. */
		bool	m_isRxPacketLen;
		/** Number of bytes currently received (packet length or data). */
		int		m_nReceived;
		/** Size of the OSC data payload. */
		int		m_packetLen;
		/** OSC data receive buffer. */
		char*	m_packetData;
	} m_selectState;
};

/** WOscTcpClient specific method (note that stream oriented OSC communication
 * has no return address. We have an open and known connection.
 */
class WOscTcpHandlerMethod: public WOscReceiverMethod {
public:
	WOscTcpHandlerMethod(WOscContainer* parent, WOscTcpHandler* receiverContext,
			const char* methodName, const char* methodDescription = "") :
		WOscReceiverMethod(parent,
				static_cast<WOscReceiver*> (receiverContext), methodName,
				methodDescription) {
	}
	virtual WOscTcpHandler* GetContext() {
		return static_cast<WOscTcpHandler*> (WOscReceiverMethod::GetContext());
	}
protected:
	virtual void Method(const WOscMessage *message) {
	}
private:
	virtual void Method(const WOscMessage *message, const WOscTimeTag& when,
			const WOscNetReturn* networkReturnAddress) {
		Method(message);
	}
};

#else
#pragma message("WOscTcpHandler not supported on this platform")
#endif

#endif /* WOSCTCPHANDLER_H_ */
