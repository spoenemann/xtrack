/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Parameter definitions.
// Parameters can be given on the command line with a 'key=value' format, e.g. 'camera=1'.

#pragma once

#include "stdafx.h"

// Activate or deactivate a window showing the contrast image used to track fiducials.
// This is useful for calibrating the camera: use your camera driver software to adapt
// the brightness until the white and black dots of the fiducials are all visible.
#define DEFAULT_SHOW_CONTRAST false
#define PARAM_SHOW_CONTRAST "showcontr"

// Activate or deactivate a window showing the unprocessed input from the camera.
// The same window is also used to play back recorded video files.
#define DEFAULT_SHOW_INPUT false
#define PARAM_SHOW_INPUT "showinput"

// Activate or deactivate printing of tracking information to the console.
#define DEFAULT_PRINT false
#define PARAM_PRINT "print"

// The camera index: if there are multiple cameras connected to your computer,
// select one with this parameter.
#define DEFAULT_CAMERA 0
#define PARAM_CAMERA "camera"

// The time in milliseconds between two frames captured from the camera.
// The frame rate can be computed from this as 'framerate = 1000 / frametime'.
// The default frame time corresponds to a frame rate of 30 fps.
#define DEFAULT_FRAME_TIME 33
#define PARAM_FRAME_TIME "ftime"

// The width in pixels of the captured frames.
#define DEFAULT_FRAME_WIDTH 320
#define PARAM_FRAME_WIDTH "fwidth"

// The height in pixels of the captured frames.
#define DEFAULT_FRAME_HEIGHT 240
#define PARAM_FRAME_HEIGHT "fheight"

// If activated, the input frames are cut such that only a quadratic area is processed.
// The resolution of this area is fheight x fheight.
#define DEFAULT_QUADRATIC false
#define PARAM_QUADRATIC "quadratic"

// The threshold value applied to input images to create the contrast images.
// Pixels that are brighter than this value are set to white, otherwise they are set to black.
#define DEFAULT_THRESHOLD 128
#define PARAM_THRESHOLD "threshold"

// The target address for UDP messages containing tracking information.
// The messages are sent in the TUIO format, see http://tuio.org/
#define DEFAULT_ADDRESS "127.0.0.1"
#define PARAM_ADDRESS "address"

// The target port for UDP messages containing tracking information.
#define DEFAULT_PORT 3333
#define PARAM_PORT "port"

// The size of rectangles drawn onto tracked figures in the input window.
#define DEFAULT_TRACK_RECT_SIZE 40
#define PARAM_TRACK_RECT_SIZE "trackrectsize"

// The names written onto tracked figures. This is a comma-separated list.
// The empty string means not to write any names.
#define DEFAULT_TRACK_NAMES ""
#define PARAM_TRACK_NAMES "tracknames"

// The directory where recorded video files are stored.
#define DEFAULT_RECORD_DIR ".\\"
#define PARAM_RECORD_DIR "recorddir"

// The codec used to write video files, in FOURCC format.
// See http://www.fourcc.org/ or http://msdn.microsoft.com/en-us/library/ms922669.aspx for more information.
// If a shorter string is given, e.g. '-', a codec selection dialog is opened whenever a recording
// is started, listing the codecs available in your system.
#define DEFAULT_CODEC "PIM1"
#define PARAM_CODEC "codec"

// A scaling factor applied to the resolution of recorded videos.
// This helps to reduce the size of video files.
#define DEFAULT_RECORD_SCALE 0.4
#define PARAM_RECORD_SCALE "recordscale"

// A scaling factor applied to the frame rate of recorded videos.
// This helps to reduce the size of video files.
#define DEFAULT_RECORD_FPS_SCALE 0.6
#define PARAM_RECORD_FPS_SCALE "recfpsscale"

// A circle with this radius is drawn in the contrast image window.
// It can be used to calibrate the camera position according to a
// given arena.
#define DEFAULT_ARENA_RADIUS 100
#define PARAM_ARENA_RADIUS "arenarad"


// Utility methods for accessing parameter values

int intParam(std::unordered_map<std::string, std::string> &parameters,
		const std::string &key, const int defaultValue);

std::string stringParam(std::unordered_map<std::string, std::string> &parameters,
		const std::string &key, const std::string &defaultValue);

bool boolParam(std::unordered_map<std::string, std::string> &parameters,
		const std::string &key, const bool defaultValue);

double doubleParam(std::unordered_map<std::string, std::string> &parameters,
		const std::string &key, const double defaultValue);
