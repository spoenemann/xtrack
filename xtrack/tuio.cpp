/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Sending TUIO messages using WOscLib (http://wosclib.sourceforge.net/)

#include <winsock2.h>
#include "tuio.h"
#include "WOscBundle.h"

#pragma comment(lib, "Ws2_32.lib")

#define OSC_PATH "/tuio/2Dobj"

TuioServer::TuioServer(std::unordered_map<std::string, std::string> parameters) {
	this->ipaddr = stringParam(parameters, PARAM_ADDRESS, DEFAULT_ADDRESS);
	this->port = intParam(parameters, PARAM_PORT, DEFAULT_PORT);

	WSADATA wsaData;
	int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0) {
        std::cerr << "Winsock startup failed (error " << startupResult << ")\n";
        throw 1;
    }

	this->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->sock == INVALID_SOCKET) {
		std::cerr << "Socket cannot be opened (error " << WSAGetLastError() << ")\n";
		throw 1;
	}
}

TuioServer::~TuioServer() {
	if (this->sock >= 0) {
		closesocket(this->sock);
	}
	WSACleanup();
}

void TuioServer::sendMessage(FiducialX fiducials[], int numFiducials, int width, int height) {
	WOscBundle *bundle = new WOscBundle();;

	// Alive message
	WOscMessage *msg = new WOscMessage(OSC_PATH);
	msg->Add("alive");
	for (int i = 0; i < numFiducials; i++) {
		FiducialX *fidx = &fiducials[i];
		if (fidx->id >= 0) {
			msg->Add(fidx->id);
		}
	}
	bundle->Add(msg);

	// Set message
	for (int i = 0; i < numFiducials; i++) {
		FiducialX *fidx = &fiducials[i];
		msg = new WOscMessage(OSC_PATH);
		msg->Add("set");
		// /tuio/2Dobj set s i x y a X Y A m r
		msg->Add(fidx->id); // blob id
		msg->Add(fidx->id); // fiducial id
		msg->Add(fidx->x / width); // x
		msg->Add(fidx->y / height); // y
		msg->Add(fidx->angle); // a
		msg->Add(0.0f); // X
		msg->Add(0.0f); // Y
		msg->Add(0.0f); // A
		msg->Add(0.0f); // m
		msg->Add(0.0f); // r
		bundle->Add(msg);
	}

	// Frame sequence message
	msg = new WOscMessage(OSC_PATH);
	msg->Add("fseq");
	msg->Add(this->fseq++);
	bundle->Add(msg);

	// Send the bundle via UDP
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(this->ipaddr.c_str());
	servaddr.sin_port = htons(this->port);

	sendto(this->sock, bundle->GetBuffer(), bundle->GetBufferLen(), 0,
		 (struct sockaddr *) &servaddr, sizeof(servaddr));

	delete bundle;
}
