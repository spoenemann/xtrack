/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Displaying the camera image and tracking information in a window

#include "display.h"

using namespace cv;

CameraDisplay::CameraDisplay(std::unordered_map<std::string, std::string> &parameters) {
	namedWindow("input", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL);

	this->trackRectSize = intParam(parameters, PARAM_TRACK_RECT_SIZE, DEFAULT_TRACK_RECT_SIZE);
	std::string trackNamesString = stringParam(parameters, PARAM_TRACK_NAMES, DEFAULT_TRACK_NAMES);
	size_t lastPos = 0;
	size_t pos = trackNamesString.find_first_of(',');
	while (pos != std::string::npos) {
		this->trackNames.push_back(trackNamesString.substr(lastPos, pos - lastPos));
		lastPos = pos + 1;
		pos = trackNamesString.find_first_of(',', lastPos);
	}
	if (lastPos < trackNamesString.length()) {
		this->trackNames.push_back(trackNamesString.substr(lastPos, trackNamesString.length() - lastPos));
	}

	this->colors.push_back(Scalar(180, 40, 30));
	this->colors.push_back(Scalar(30, 40, 180));

	this->fontColor = Scalar(240, 240, 240);
}

void CameraDisplay::displayTrackedInput(InputArray input, FiducialX fiducials[], int numFiducials) {
	Mat frameMat = input.getMat();

	// Draw tracking information for fiducials
	for (int i = 0; i < numFiducials; i++) {
		FiducialX &fidx = fiducials[i];
		if (fidx.id >= 0) {
			// Draw the rotated rectangle
			const Point points[] = {
				Point((int) (fidx.x + trackRectSize * cos(fidx.angle + 0.75f * PI)),
					(int) (fidx.y + trackRectSize * sin(fidx.angle + 0.75f * PI))),
				Point((int) (fidx.x + trackRectSize * cos(fidx.angle + 0.25f * PI)),
					(int) (fidx.y + trackRectSize * sin(fidx.angle + 0.25f * PI))),
				Point((int) (fidx.x + trackRectSize * cos(fidx.angle + 1.75f * PI)),
					(int) (fidx.y + trackRectSize * sin(fidx.angle + 1.75f * PI))),
				Point((int) (fidx.x + trackRectSize * cos(fidx.angle + 1.25f * PI)),
					(int) (fidx.y + trackRectSize * sin(fidx.angle + 1.25f * PI)))
			};
			const Scalar &color = colors.at(fidx.id % colors.size());
			fillConvexPoly(frameMat, points, 4, color);

			// Draw the tracked object name
			if (!trackNames.empty()) {
				std::string name = trackNames.at(fidx.id % trackNames.size());
				int baseLine;
				Size &textSize = getTextSize(name, FONT_HERSHEY_PLAIN, 1.5, 2, &baseLine);
				Point textPos((int) fidx.x - textSize.width / 2, (int) fidx.y + textSize.height / 2);
				putText(frameMat, name, textPos, FONT_HERSHEY_PLAIN, 1.5, fontColor, 2);
			}
		}
	}

	imshow("input", frameMat);
}
