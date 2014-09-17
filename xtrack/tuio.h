// Sending TUIO messages using WOscLib (http://wosclib.sourceforge.net/)

#pragma once

#include "stdafx.h"

class TuioServer {
public:
	TuioServer(std::unordered_map<std::string, std::string> parameters);
	~TuioServer();

	void sendMessage(FiducialX fiducials[], int numFiducials, int width, int height);

private:
	std::string ipaddr;
	unsigned short port;
	int fseq;
	int sock;
};
