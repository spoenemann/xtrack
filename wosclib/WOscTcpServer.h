/*
 * WOscTcpServer.h
 *
 *  Created on: Oct 22, 2010
 *      Author: uli
 */

#ifndef WOSCTCPSERVER_H_
#define WOSCTCPSERVER_H_

#include "WOscTcpHandler.h"

#if OS_IS_LINUX == 1 || OS_IS_MACOSX == 1 || OS_IS_CYGWIN == 1

#include <vector>

#ifdef DEBUG
#	include <iostream>
#	define WOSC_ERR_PRT(MSG) std::cerr<<__FILE__<<" L"<<__LINE__<<" "<<__FUNCTION__<<": "<<MSG<<std::endl;
#	define WOSC_DBG_PRT(MSG) std::cout<<__FILE__<<" L"<<__LINE__<<" "<<__FUNCTION__<<": "<<MSG<<std::endl;
#else
#	define WOSC_ERR_PRT(MSG)
#	define WOSC_DBG_PRT(MSG)
#endif


class WOscTcpServer;

/** Threading, self deleting connection handler.
 * For each client connection a handler gets spawned an is run in a separate
 * thread.
 *
 */
class WOscTcpServerHandler: public WOscTcpHandler
{
public:
	WOscTcpServerHandler(WOscTcpServer* server): m_server(server) { }
protected:
	virtual void ReceiveThread();
	WOscTcpServer* GetServer() { return m_server; }
	virtual void HandleOffendingPackets(const char* const data, int dataLen,
			const WOscException& exception) { }
	virtual void HandleNonmatchedMessages(const WOscMessage* msg,
			const WOscNetReturn* networkReturnAddress) { }
private:
	WOscTcpServer* m_server;
};

class WOscTcpServer {
public:
	WOscTcpServer(bool threading = true);
	virtual ~WOscTcpServer();

	typedef enum {
		WOS_TCPS_ERR_NO_ERR = 0,
		WOS_TCPS_ERR_OPEN_SOCKET,
		WOS_TCPS_ERR_BIND,
		WOS_TCPS_ERR_CREATE_THREAD,
		WOS_TCPS_ERR_NO_SERVER_SOCKET,
		WOS_TCPS_ERR_ACCEPT,
		WOS_TCPS_ERR_SET_NONBLOCKING,
		WOS_TCPS_ERR_ALREADY_BOUND,
		WOS_TCPS_ERR_SERVER_RUNNING,
		WOS_TCPS_ERR_LISTEN,
	} Error;
	static const char* GetErrStr(Error err);
	Error Bind(int port, in_addr_t addr = INADDR_ANY);
	Error Start();
	Error Stop();
	/** This member is called from the server thread if a new client has
	 * connected to the server. The final server implementation must override
	 * this abstract member to launch its own connection handler. Please see
	 * the streaming server example how to do this. In short:\code
	{
		WOscTcpServerHandler* h = new WOscStreamingHandler(this);
		WOscTcpServerHandler::Error err = h->Start(socket, peer);
		if (err != WOscTcpServerHandler::WOS_ERR_NO_ERROR ) {
			delete h;
		} else {
			*handler = h;
		}
		return err;
	}
	\endcode
	 */
	virtual WOscTcpServerHandler::Error SetupHandlerForNewConnection(WOscTcpServerHandler** handler, int socket, const TheNetReturnAddress& peer) = 0;
	int GetServerSocket() const { return m_socketServer; }
protected:
	friend void* wosc_tcp_server_thread(void* argument);
	Error Accept();
	friend class WOscTcpServerHandler;
	void Register(WOscTcpServerHandler* handler)
	{
		pthread_mutex_lock(&m_connectionsMutex);
		m_connections.push_back(handler);
		pthread_mutex_unlock(&m_connectionsMutex);
	}
	bool UnRegister(WOscTcpServerHandler* handler)
	{
		bool ret = false;
		pthread_mutex_lock(&m_connectionsMutex);
		for ( int i = m_connections.size(); i >= 0; i-- )
			if ( m_connections[i] == handler ){
				m_connections.erase(m_connections.begin()+i);
				ret = true;
				break;
			}
		pthread_mutex_unlock(&m_connectionsMutex);
		return ret;
	}

private:
	int m_socketServer;
	bool m_threading;
	pthread_t m_serverThreadID;
	std::vector<WOscTcpServerHandler*> m_connections;
	pthread_mutex_t m_connectionsMutex;
};

#else
#pragma message("WOscTcpServer not supported on this platform")
#endif

#endif /* WOSCTCPSERVER_H_ */
