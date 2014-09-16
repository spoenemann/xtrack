/** WOscTCPClient.cpp - A streaming OSC client using posix threads and sockets
 *
 *  Created on: Apr 21, 2010
 *      Author: uli
 */

#include "WOscTcpClient.h"

#if OS_IS_LINUX == 1 || OS_IS_MACOSX == 1 || OS_IS_CYGWIN == 1

#include <fcntl.h>
#include <sys/socket.h>


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
WOscTcpClient::WOscTcpClient(bool threading) : WOscTcpHandler(threading)
{}

WOscTcpClient::~WOscTcpClient()
{
	Close();
}

/** Connect to a OSC server at a given address.
 * Starts a receiver thread which handles any replies from the server when this
 * class was constructed with threading enabled. If not, the socket is set to
 * non blocking and can be used in a select statement.
 */
WOscTcpClient::Error
WOscTcpClient::Connect(const TheNetReturnAddress& serverAddress)
{
	// Check if reception thread is already running
	if ( GetRxThreadID() > 0 )
		return WOS_ERR_ALREADY_RUNNING;
	
	// Check if someone established a connection already
	if ( GetSocketID() >= 0 )
		return WOS_ERR_ALREADY_CONNECTED;

	// Create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return WOS_ERR_SOCKET_CREATE;
	}

	// Connect
	int ret = connect(sock, (struct sockaddr*) &serverAddress.m_addr, sizeof(serverAddress.m_addr));
	if ( ret < 0 ) {
		close(sock);
		return WOS_ERR_SOCKET_CONNECT;
	}
	
	// starts thread if threading is enabled
	WOscTcpHandler::Error err = Start(sock, serverAddress);
	if ( err != WOscTcpHandler::WOS_ERR_NO_ERROR )
		close(sock);

	return err;
}

/** Close the receiving socket and shutdown the receiving thread if in
 * threading mode.
 */
WOscTcpClient::Error
WOscTcpClient::Close()
{
	// Check if reception thread has already been shut down
	if ( GetThreading() && GetRxThreadID() )
		return WOS_ERR_ALREADY_CLOSED;
	// Check if socket still
	if ( GetSocketID() < 0 )
		return WOS_ERR_DISCONNECTED;

	WOscTcpHandler::Error err = Stop();

	// close socket
	int ret = close(GetSocketID());
	if ( ret ) {
//		std::cout<<"Close failed."<<std::endl;
	}
	return err;
}

#elif OS_IS_WIN32 == 1
/* currently this example does not support windows - volunteers?
#	include "windows.h"
#	include "winsock2.h"
#	define socklen_t	int
*/
#pragma message("WOscTcpClient not supported on this platform")
#endif
