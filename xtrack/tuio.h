/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Sending TUIO messages using WOscLib (http://wosclib.sourceforge.net/)

#pragma once

#include "stdafx.h"

class TuioServer {
public:
	TuioServer(std::unordered_map<std::string, std::string> &parameters, int fwidth, int fheight);
	~TuioServer();

	void sendMessage(FiducialX fiducials[], int numFiducials);

private:
	std::string ipaddr;
	unsigned short port;
	int fseq;
	int sock;
	int fwidth;
	int fheight;
};
