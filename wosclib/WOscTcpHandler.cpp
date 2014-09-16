/*
 * WOscTcpHandler.cpp
 *
 *  Created on: Oct 22, 2010
 *      Author: uli
 */

#include "WOscTcpHandler.h"

/** WOscTCPClient.cpp - A streaming OSC client using posix threads and sockets
 *
 *  Created on: Apr 21, 2010
 *      Author: uli
 */

#if OS_IS_LINUX == 1 || OS_IS_MACOSX == 1 || OS_IS_CYGWIN == 1

#include <fcntl.h>
#include <sys/socket.h>

// On OSX MSG_NOSIGNAL is not defined
#if !defined(MSG_NOSIGNAL) && OS_IS_MACOSX == 1
#	define MSG_NOSIGNAL	SO_NOSIGPIPE
#endif

/** Bootstrapper for receive thread member function. */
void*
wosc_tcp_handler_rx_thread(void* argument)
{
	WOscTcpHandler* handler = static_cast<WOscTcpHandler*>(argument);
	handler->ReceiveThread();
	pthread_exit(NULL);
}

static int
wosc_send_over_socket(int socket, const char* buffer, int count)
{
	int nSent = 0;
	for ( int tmp; (count - nSent ) > 0; nSent += tmp) {
		tmp = send(socket, buffer + nSent, count - nSent, MSG_NOSIGNAL);
		if ( tmp <= 0 )
			return -1;
	}
	return nSent;
}

template<class TYPE> class WOscAutoArray
{
public:
	WOscAutoArray(TYPE* array) : m_array(array) { }
	virtual ~WOscAutoArray() { delete [] m_array;	}
	TYPE* operator &() { return m_array; }
	operator TYPE*() { return m_array; }
private:
	TYPE* m_array;
};

/** Implements a full featured OSC client with TCP transport.
 * @param threading
 * If true, all reception happens in a separate thread and does not require any
 * select logic in the main loop, but you have to take care when calling main
 * thread code from the OSC message handlers/methods.
 * If false, threading is disabled and the reception must be implemented by a
 * select statement. On input WOscTcpClient::NetworkReceive() must be called.
 * The socket is switched to non blocking automatically. See the streaming
 * server example for further details.
 */
WOscTcpHandler::WOscTcpHandler(bool threading)
{
	m_receptionThreadID = 0;
	m_socketID = -1;
	pthread_mutex_init(&m_mutexTx, NULL);
	m_threading = threading;
}

WOscTcpHandler::~WOscTcpHandler()
{
	pthread_mutex_destroy(&m_mutexTx);
	// we do not stop the thread here as it could be that a handler must
	// delete (detach) itself from within it's thread
}

const char*
WOscTcpHandler::GetErrStr(Error err)
{
	switch ( err ) {
	case WOS_ERR_NO_ERROR: return "No error.";
	case WOS_ERR_ALREADY_RUNNING: return "Seems that a reception thread is already running. You have to disconnect first.";
	case WOS_ERR_ALREADY_CONNECTED: return "Seems that we're already connected";
	case WOS_ERR_SOCKET_CREATE: return "Socket creation failed.";
	case WOS_ERR_SOCKET_CONNECT: return "Socket connect failed.";
	case WOS_ERR_CREATE_RX_THREAD: return "Creation of receive thread failed.";
	case WOS_ERR_NO_SOCKET: return "Socket not connected.";
	case WOS_ERR_CONNECTION_LOST: return "Socket connection lost.";
	case WOS_ERR_DISCONNECTED: return "Trying to close an already closed socket.";
	case WOS_ERR_CONNECTION_CLOSED: return "Tried to receive with "
			"WOscTcpClient::NetworkReceive() from a connection which has been "
			"closed.";
	case WOS_ERR_CONNECTION_FAILURE: return "Receiving from WOscTcpClient::NetworkReceive() failed.";
	case WOS_ERR_NOT_WHEN_THREADING: return "The select/polling receive "
			"function can only be used with threading disabled.";
	case WOS_ERR_CAN_NOT_SET_NONBLOCKING: return "Can not set socket into non blocking mode.";
	default: return "Unknown error.";
	}
}

/** Transmit data (preferrably OSC messages and bundles) to the server. */
WOscTcpHandler::Error
WOscTcpHandler::NetworkSend(const char* data, int dataLen)
{
	pthread_mutex_lock(&m_mutexTx);
	if ( m_socketID < 0 ) {
		pthread_mutex_unlock(&m_mutexTx);
		return WOS_ERR_NO_SOCKET;
	}

	int len = htonl(dataLen);
	len = wosc_send_over_socket(m_socketID, (const char*)&len, sizeof(len));
	if ( len <= 0 ) {
		pthread_mutex_unlock(&m_mutexTx);
		return WOS_ERR_CONNECTION_LOST;
	}
	len = wosc_send_over_socket(m_socketID, data, dataLen);
	if ( len <= 0 ) {
		pthread_mutex_unlock(&m_mutexTx);
		return WOS_ERR_CONNECTION_LOST;
	}
	pthread_mutex_unlock(&m_mutexTx);
	return WOS_ERR_NO_ERROR;
}

void
WOscTcpHandler::NetworkSend(const char* data, int dataLen,
			const WOscNetReturn* networkReturnAddress)
{
	NetworkSend(data, dataLen);
}
void
WOscTcpHandler::ReceiveThread()
{
	while ( true ) {
		Error err = NetworkReceiveInternal();
		if ( err == WOS_ERR_NO_ERROR ) {
			continue;
		} else if ( err == WOS_ERR_CONNECTION_CLOSED ) {
			break;
		} else if ( err == WOS_ERR_CONNECTION_FAILURE ) {
			break;
		}
	}
}

WOscTcpHandler::Error
WOscTcpHandler::NetworkReceive()
{
	if ( m_threading )
		return WOS_ERR_NOT_WHEN_THREADING;
	return NetworkReceiveInternal();
}

WOscTcpHandler::Error
WOscTcpHandler::Start(int socketID, const TheNetReturnAddress& peer)
{
	if (socketID < 0)
		return WOS_ERR_INVALID_SOCKET;
	if ( m_socketID >= 0 )
		return WOS_ERR_ALREADY_CONNECTED;
	// Check if reception thread is already running
	if ( m_receptionThreadID > 0 )
		return WOS_ERR_ALREADY_RUNNING;

	m_socketID = socketID;
	m_selectState.Init();
	m_peer = peer;

	if ( m_threading ) {
		// Start receiving thread
		int ret = pthread_create(&m_receptionThreadID, NULL, wosc_tcp_handler_rx_thread, this);
		if ( ret ) {
//			close(m_socketID);
			m_socketID = -1;
			m_receptionThreadID = 0;
			return WOS_ERR_CREATE_RX_THREAD;
		}
	} else {
		// set socket non blocking for select operation when threading is
		// disabled
		if (fcntl(m_socketID, F_SETFL, fcntl(m_socketID, F_GETFL) | O_NONBLOCK) < 0) {
//			close(m_socketID);
			m_socketID = -1;
			return WOS_ERR_CAN_NOT_SET_NONBLOCKING;
		}

	}
	return WOS_ERR_NO_ERROR;

}

/** Shutdown and join the receiving thread if in
 * threading mode. The socket must be closed beforehand.
 */
WOscTcpHandler::Error
WOscTcpHandler::Stop()
{
	// Check if reception thread is already running
	if ( m_threading && m_receptionThreadID == 0 )
		return WOS_ERR_ALREADY_CLOSED;

	// Check if someone stopped us already
	if ( m_socketID < 0 )
		return WOS_ERR_DISCONNECTED;

	// Check if reception thread is already running
	if ( m_threading ) {
		// join rx thread with main thread
		void* threadReturn = NULL;
		int ret = pthread_cancel(m_receptionThreadID);
		if ( ret ) {
	//		std::cerr<<"Receive thread join failed."<<std::endl;
		}
		ret = pthread_join(m_receptionThreadID, &threadReturn);
		if ( ret ) {
	//		std::cerr<<"Receive thread join failed."<<std::endl;
		}
		m_receptionThreadID = 0;
	}
	m_socketID = -1;

	// set
	return WOS_ERR_NO_ERROR;
}

WOscTcpHandler::Error
WOscTcpHandler::NetworkReceiveInternal()
{
	char* rxBuf;
	int rxCount;
	if ( m_selectState.m_isRxPacketLen ) {
		rxBuf = (char*)&m_selectState.m_packetLen + m_selectState.m_nReceived;
		rxCount = sizeof(m_selectState.m_packetLen) - m_selectState.m_nReceived;
	} else {
		rxBuf = m_selectState.m_packetData;
		rxCount = m_selectState.m_packetLen - m_selectState.m_nReceived;
	}

	int nRecv = recv(m_socketID, rxBuf, rxCount, 0);

	if ( nRecv == 0 ) {
		// connection closed
		return WOS_ERR_CONNECTION_CLOSED;
	} else if ( nRecv < 0 ) {
		// connection failure
		return WOS_ERR_CONNECTION_FAILURE;
	} else {
		m_selectState.m_nReceived += nRecv;
	}

	// check if packet length is complete
	if ( m_selectState.m_isRxPacketLen &&  m_selectState.m_nReceived == sizeof(m_selectState.m_packetLen) ) {
		m_selectState.m_packetLen = ntohl(m_selectState.m_packetLen);
		m_selectState.m_isRxPacketLen = false;
		delete [] m_selectState.m_packetData;
		m_selectState.m_packetData = new char[m_selectState.m_packetLen];
		m_selectState.m_nReceived = 0;
	}
	// check if packet data is complete
	else if ( !m_selectState.m_isRxPacketLen &&  m_selectState.m_nReceived == m_selectState.m_packetLen) {
		m_selectState.m_isRxPacketLen = true;
		WOscReceiver::NetworkReceive(m_selectState.m_packetData, m_selectState.m_packetLen, new TheNetReturnAddress(m_peer));
		m_selectState.m_nReceived = 0;
	}
	return WOS_ERR_NO_ERROR;
}

#elif OS_IS_WIN32 == 1
/* currently this example does not support windows - volunteers?
#	include "windows.h"
#	include "winsock2.h"
#	define socklen_t	int
*/
#pragma message("WOscTcpHandler not supported on this platform")
#endif
