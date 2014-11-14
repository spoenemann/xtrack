/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Sending TUIO messages using WOscLib (http://wosclib.sourceforge.net/)

#include "tuio.h"
#include "WOscBundle.h"

#define OSC_PATH "/tuio/2Dobj"

TuioServer::TuioServer(std::unordered_map<std::string, std::string> &parameters) {
	this->ipaddr = stringParam(parameters, PARAM_ADDRESS, DEFAULT_ADDRESS);
	this->port = intParam(parameters, PARAM_PORT, DEFAULT_PORT);
	this->fseq = 0;

	// TODO implement
//	WSADATA wsaData;
//	int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (startupResult != 0) {
//        std::cerr << "Winsock startup failed (error " << startupResult << ")\n";
//        throw 1;
//    }
//
//	this->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	if (this->sock == INVALID_SOCKET) {
//		std::cerr << "Socket cannot be opened (error " << WSAGetLastError() << ")\n";
//		throw 1;
//	}
}

TuioServer::~TuioServer() {
	// TODO implement
//	if (this->sock >= 0) {
//		closesocket(this->sock);
//	}
//	WSACleanup();
}

void TuioServer::sendMessage(TrackedFiducial fiducials[]) {
	WOscBundle bundle;

	// Alive message
	WOscMessage *aliveMsg = new WOscMessage(OSC_PATH);
	aliveMsg->Add("alive");
	for (int i = 0; i < MAX_FIDUCIALS; i++) {
		TrackedFiducial &fid = fiducials[i];
		if (fid.isTracked) {
			aliveMsg->Add(i);
		}
	}
	bundle.Add(aliveMsg);

	// Set message
	for (int i = 0; i < MAX_FIDUCIALS; i++) {
		TrackedFiducial &fid = fiducials[i];
		if (fid.isTracked) {
			WOscMessage *setMsg = new WOscMessage(OSC_PATH);
			setMsg->Add("set");
			setMsg->Add(i); // session id
			setMsg->Add(i); // fiducial id
			setMsg->Add(fid.x); // horizontal position
			setMsg->Add(fid.y); // vertical position
			setMsg->Add(fid.a); // angle
			setMsg->Add(fid.xspeed); // horizontal motion speed
			setMsg->Add(fid.yspeed); // vertical motion speed
			setMsg->Add(fid.aspeed); // rotation speed
			setMsg->Add(fid.xyacc); // motion acceleration
			setMsg->Add(fid.aacc); // rotation acceleration
			bundle.Add(setMsg);
		}
	}

	// Frame sequence message
	WOscMessage *seqMsg = new WOscMessage(OSC_PATH);
	seqMsg->Add("fseq");
	seqMsg->Add(this->fseq++);
	bundle.Add(seqMsg);

	// Send the bundle via UDP
	// TODO implement
//	struct sockaddr_in servaddr;
//	servaddr.sin_family = AF_INET;
//	servaddr.sin_addr.s_addr = inet_addr(this->ipaddr.c_str());
//	servaddr.sin_port = htons(this->port);
//
//	sendto(this->sock, bundle.GetBuffer(), bundle.GetBufferLen(), 0,
//		 (struct sockaddr *) &servaddr, sizeof(servaddr));
}
