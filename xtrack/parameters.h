/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Parameter definitions

#pragma once

#include "stdafx.h"

#define DEFAULT_WINDOW false
#define PARAM_WINDOW "window"

#define DEFAULT_PRINT false
#define PARAM_PRINT "print"

#define DEFAULT_CAMERA 0
#define PARAM_CAMERA "camera"

#define DEFAULT_FRAME_TIME 30
#define PARAM_FRAME_TIME "ftime"

#define DEFAULT_FRAME_WIDTH 320
#define PARAM_FRAME_WIDTH "fwidth"

#define DEFAULT_FRAME_HEIGHT 240
#define PARAM_FRAME_HEIGHT "fheight"

#define DEFAULT_THRESHOLD 128
#define PARAM_THRESHOLD "threshold"

#define DEFAULT_ADDRESS "127.0.0.1"
#define PARAM_ADDRESS "address"

#define DEFAULT_PORT 3333
#define PARAM_PORT "port"

int intParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, int defaultValue);

std::string stringParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, std::string defaultValue);

bool boolParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, bool defaultValue);
