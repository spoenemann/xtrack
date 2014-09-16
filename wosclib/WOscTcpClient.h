#include "WOscConfig.h"

#ifndef __WOSC_TCP_CLIENT_H__
#define __WOSC_TCP_CLIENT_H__

#if OS_IS_LINUX == 1 || OS_IS_MACOSX == 1 || OS_IS_CYGWIN == 1

#include "WOscTcpHandler.h"

/**
 *
 */
class WOscTcpClient: public WOscTcpHandler
{
public:
	WOscTcpClient(bool threading = true);
	virtual ~WOscTcpClient();
	/** Try to open a connection to a remote server.
	 * Establishes a connection to a remote server and creates a separate
	 * thread for reception of server replies.
	 */
	Error Connect(const TheNetReturnAddress& serverAddress);
	Error Close();
protected:
	virtual void HandleOffendingPackets(const char* const data, int dataLen,
			const WOscException& exception) { }
	virtual void HandleNonmatchedMessages(const WOscMessage* msg,
			const WOscNetReturn* networkReturnAddress) { }
};

#else
#pragma message("WOscTcpClient not supported on this platform")
#endif

#endif	// __WOSC_TCP_CLIENT_H__
