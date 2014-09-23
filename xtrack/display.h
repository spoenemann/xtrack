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

class CameraDisplay {
public:
	CameraDisplay(std::unordered_map<std::string, std::string> &parameters);

	void displayTrackedInput(cv::InputArray input, FiducialX fiducials[], int numFiducials);

private:
	int trackRectSize;
	std::vector<std::string> trackNames;
	std::vector<cv::Scalar> colors;
	cv::Scalar fontColor;
};
