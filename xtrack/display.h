/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Displaying the camera image and tracking information in a window

#pragma once

#include "stdafx.h"
#include "fiducials.h"

class CameraDisplay {
public:
	CameraDisplay(std::unordered_map<std::string, std::string> &parameters, cv::Size &screenSize);

	// Draw tracking information onto the given frame.
	void drawTrackingInfo(cv::Mat &frameMat, TrackedFiducial fiducials[]);
	// Display the given frame in the input window.
	void displayTrackedImage(cv::InputArray input);

private:
	int trackRectSize;
	cv::Size screenSize;
	std::vector<std::string> trackNames;
	std::vector<cv::Scalar> trackColors;
	cv::Scalar fontColor;
};
