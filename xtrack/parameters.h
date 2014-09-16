// Parameter definitions

#pragma once

#include "stdafx.h"

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
