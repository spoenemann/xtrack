/*
 * WOscTcpServer.cpp
 *
 *  Created on: Oct 22, 2010
 *      Author: uli
 */

#include "WOscTcpServer.h"

#if OS_IS_LINUX == 1 || OS_IS_MACOSX == 1 || OS_IS_CYGWIN == 1

#include <fcntl.h>
#include <sys/socket.h>

#define WOSC_TCP_SERVER_NUM_PENDING_CONNECTIONS	5

void
WOscTcpServerHandler::ReceiveThread()
{
	WOSC_DBG_PRT("Client thread entered.");
	WOscTcpHandler::ReceiveThread();
	// If we were still registered at the server, we cleanup and delete
	// ourselves. If the server terminated us, we just exit as he will
	// wait for us.
	if ( GetServer()->UnRegister(this) ) {
		close(GetSocketID());
		int ret = pthread_detach(GetRxThreadID());
		if ( ret ) {
			WOSC_ERR_PRT("Client detach failed.");
		} else {
			WOSC_DBG_PRT("Client thread detached.");
		}
		delete this;
	}
	WOSC_DBG_PRT("Client thread terminated.");
}

/* ------------------------------------------------------------------------- */

/** Bootstrapper for receive thread member function. */
void*
wosc_tcp_server_thread(void* argument)
{
	WOSC_DBG_PRT("Server thread entered.");
	WOscTcpServer* server = static_cast<WOscTcpServer*>(argument);
	server->Accept();
	WOSC_DBG_PRT("Server thread terminated.");
	pthread_exit(NULL);
}

WOscTcpServer::WOscTcpServer(bool threading)
{
	m_socketServer = -1;
	m_threading = threading;
	m_serverThreadID = 0;
	pthread_mutex_init(&m_connectionsMutex, NULL);
}

WOscTcpServer::~WOscTcpServer()
{
	Stop();
	pthread_mutex_destroy(&m_connectionsMutex);
}

WOscTcpServer::Error
WOscTcpServer::Bind(int port, in_addr_t addr)
{
	if ( m_socketServer >= 0 )
		return WOS_TCPS_ERR_ALREADY_BOUND;
	if ( m_threading && m_serverThreadID )
		return WOS_TCPS_ERR_SERVER_RUNNING;

	/* open a tcp socket to listen on */
	int socket_server = socket(AF_INET, SOCK_STREAM, 0);
	if ( socket_server < 0)
		return WOS_TCPS_ERR_OPEN_SOCKET;

	/* What exactly does SO_REUSEADDR do?
	 *
	 * This socket option tells the kernel that even if this port is busy (in
	 * the TIME_WAIT state), go ahead and reuse it anyway.  If it is busy,
	 * but with another state, you will still get an address already in use
	 * error.  It is useful if your server has been shut down, and then
	 * restarted right away while sockets are still active on its port.  You
	 * should be aware that if any unexpected data comes in, it may confuse
	 * your server, but while this is possible, it is not likely.
	 *
	 */
	int on = 1;
	if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		//perror("setsockopt(SO_REUSEADDR) failed");
	}

	// bind to our local address
	struct sockaddr_in address_server;
	bzero(&address_server, sizeof(address_server));
	address_server.sin_family = AF_INET;
	address_server.sin_addr.s_addr = htonl(addr);
	address_server.sin_port = htons(port);

	if (bind(socket_server, (struct sockaddr *) &address_server,
			sizeof(address_server)) < 0) {
		close(socket_server);
		return WOS_TCPS_ERR_BIND;
	}

	if ( listen(socket_server, WOSC_TCP_SERVER_NUM_PENDING_CONNECTIONS) < 0) {
		close(socket_server);
		return WOS_TCPS_ERR_LISTEN;
	}
	m_socketServer = socket_server;

	// start server thread here
	if ( m_threading ) {
		// Start receiving thread
		int ret = pthread_create(&m_serverThreadID, NULL, wosc_tcp_server_thread, this);
		if ( ret ) {
			close(m_socketServer);
			m_socketServer = -1;
			m_serverThreadID = 0;
			return WOS_TCPS_ERR_CREATE_THREAD;
		}
	} else {
		// set socket non blocking for select operation when threading is
		// disabled
		if (fcntl(m_socketServer, F_SETFL, fcntl(m_socketServer, F_GETFL) | O_NONBLOCK) < 0) {
			close(m_socketServer);
			m_socketServer = -1;
			return WOS_TCPS_ERR_SET_NONBLOCKING;
		}
	}

	return WOS_TCPS_ERR_NO_ERR;
}

WOscTcpServer::Error
WOscTcpServer::Accept()
{
	if ( m_socketServer < 0 )
		return WOS_TCPS_ERR_NO_SERVER_SOCKET;
	for (;;) {

		// Wait for client connection
		int socket_client;
		struct sockaddr_in address_client;
		socklen_t client_address_size = sizeof(address_client);

		// this is a thread cancellation point
		socket_client = accept(m_socketServer, (struct sockaddr *) &address_client, &client_address_size);

		if (socket_client < 0) {
			// server socket closed or error
			WOSC_ERR_PRT("socket_client < 0");
			return WOS_TCPS_ERR_ACCEPT;
		}

		// Setup handler for client connection
		WOscTcpServerHandler* handler;
		WOscTcpHandler::Error err = SetupHandlerForNewConnection(&handler, socket_client, TheNetReturnAddress(address_client));
		if ( err == WOscTcpHandler::WOS_ERR_NO_ERROR ) {
			// Register connection in order we can kill all active connections
			// when we terminate
			Register(handler);
		} else {
			// if handler failed to start, we have to close the connection
			close(socket_client);
		}

	}
	return WOS_TCPS_ERR_NO_ERR;
}


WOscTcpServer::Error
WOscTcpServer::Stop()
{
	// stop server
	if ( m_socketServer < 0 )
		return WOS_TCPS_ERR_NO_SERVER_SOCKET;

	WOSC_DBG_PRT("Stopping server thread.");

	if ( m_threading && m_serverThreadID ) {
		WOSC_DBG_PRT("Cancelling server thread.");
		int ret = pthread_cancel(m_serverThreadID);
		if ( ret ) {
			WOSC_ERR_PRT("Server thread cancel failed.");
		}
		void* threadReturn = NULL;
		ret = pthread_join(m_serverThreadID, &threadReturn);
		if ( ret ) {
			WOSC_ERR_PRT("Server thread join failed.");
		}
		m_serverThreadID = 0;
	}

	// Stop open connections: Unregister clients. Unregistered clients do not
	// delete themselves

	pthread_mutex_lock(&m_connectionsMutex);
	// backup running client connections
	std::vector<WOscTcpServerHandler*> openClients = m_connections;
	m_connections.clear();
	pthread_mutex_unlock(&m_connectionsMutex);

	// Terminate and cleanup all client threads.

	for ( int i = openClients.size()-1; i >= 0; i-- ) {
		WOSC_DBG_PRT("Terminating client connection "<<i<<" and joining thread.");
		// by closing the client connection sockets the threads will exit and
		// the client connection handlers will deallocate themselves.
		openClients[i]->Stop();
		close(openClients[i]->GetSocketID());
		delete openClients[i];
	}

	WOSC_DBG_PRT("Closing server socket.");

	close(m_socketServer);
	m_socketServer = -1;


	return WOS_TCPS_ERR_NO_ERR;
}

const char*
WOscTcpServer::GetErrStr(Error err)
{
	switch (err) {
	case WOS_TCPS_ERR_NO_ERR:
		return "No error.";
	case WOS_TCPS_ERR_OPEN_SOCKET:
		return "WOS_TCPS_ERR_OPEN_SOCKET";
	case WOS_TCPS_ERR_BIND:
		return "WOS_TCPS_ERR_BIND";
	case WOS_TCPS_ERR_CREATE_THREAD:
		return "WOS_TCPS_ERR_CREATE_THREAD";
	case WOS_TCPS_ERR_NO_SERVER_SOCKET:
		return "WOS_TCPS_ERR_NO_SERVER_SOCKET";
	case WOS_TCPS_ERR_ACCEPT:
		return "WOS_TCPS_ERR_ACCEPT";
	case WOS_TCPS_ERR_SET_NONBLOCKING:
		return "WOS_TCPS_ERR_SET_NONBLOCKING";
	case WOS_TCPS_ERR_ALREADY_BOUND:
		return "WOS_TCPS_ERR_ALREADY_BOUND";
	case WOS_TCPS_ERR_SERVER_RUNNING:
		return "WOS_TCPS_ERR_SERVER_RUNNING";
	default:
		return "Unknown or undocumented error.";
	}
}

#elif OS_IS_WIN32 == 1
/* currently this xource does not support windows - volunteers? */
#pragma message("WOscTcpServer not supported on this platform")
#endif
