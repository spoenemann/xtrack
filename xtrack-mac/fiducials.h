/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Fiducial tracking using libfidtrack (http://reactivision.sourceforge.net/)

#pragma once

#include "stdafx.h"
#include "fidtrackX.h"
#include "segment.h"

#define MAX_FIDUCIALS 4

class TrackedFiducial {
public:
	// Has the fiducial been tracked?
	bool isTracked;
	// The timestamp of the tracking information (in seconds)
	double timestamp;
	// Horizontal position in the range [0,1]
	float x;
	// Vertical position in the range [0,1]
	float y;
	// Rotation angle of the fiducial
	float a;
	// Horizontal motion speed
	float xspeed;
	// Vertical motion speed
	float yspeed;
	// Rotation speed
	float aspeed;
	// Motion acceleration
	float xyacc;
	// Rotation acceleration
	float aacc;
};

class FiducialFinder {
public:
	// Array of tracked fiducials: the array index corresponds to the fiducial id
	TrackedFiducial trackedFiducials[MAX_FIDUCIALS];

	FiducialFinder(cv::Size &fsize);
	~FiducialFinder();

	// Find fiducials and store them in the 'fiducials' array. The return value
	// is the number of actually found fiducials.
	int findFiducials(cv::InputArray, double timestamp);

private:
	FiducialX rawFiducials[MAX_FIDUCIALS];
	Segmenter segmenter;
	TreeIdMap treeidmap;
	FidtrackerX fidtrackerx;
	ShortPoint *dmap;
	cv::Size fsize;
};
