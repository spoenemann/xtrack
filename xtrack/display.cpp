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
	namedWindow("input", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
	cvSetWindowProperty("input", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	this->trackRectSize = intParam(parameters, PARAM_TRACK_RECT_SIZE, DEFAULT_TRACK_RECT_SIZE);

	// Read names of tracked objects from parameters
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

	// Define some fixed colors
	this->trackColors.push_back(Scalar(180, 40, 30));
	this->trackColors.push_back(Scalar(30, 40, 180));
	this->fontColor = Scalar(230, 230, 230);

	this->quadraticTracking = boolParam(parameters, PARAM_QUADRATIC, DEFAULT_QUADRATIC);
}

void CameraDisplay::displayTrackedInput(InputArray input, FiducialX fiducials[], int numFiducials) {
	Mat frameMat = input.getMat();
	Point offset;
	if (quadraticTracking) {
		offset.x = (frameMat.cols - frameMat.rows) / 2;
	}

	// Draw tracking information for fiducials
	for (int i = 0; i < numFiducials; i++) {
		FiducialX &fidx = fiducials[i];
		if (fidx.id >= 0) {
			// Draw a rotated rectangle
			const Point points[] = {
				// Top left corner
				Point(offset.x + (int) (fidx.x + trackRectSize * cos(fidx.angle + 0.75f * PI)),
					offset.y + (int) (fidx.y + trackRectSize * sin(fidx.angle + 0.75f * PI))),
				// Top right corner
				Point(offset.x + (int) (fidx.x + trackRectSize * cos(fidx.angle + 0.25f * PI)),
					offset.y + (int) (fidx.y + trackRectSize * sin(fidx.angle + 0.25f * PI))),
				// Bottom right corner
				Point(offset.x + (int) (fidx.x + trackRectSize * cos(fidx.angle + 1.75f * PI)),
					offset.y + (int) (fidx.y + trackRectSize * sin(fidx.angle + 1.75f * PI))),
				// Arrow head
				Point(offset.x + (int) (fidx.x + trackRectSize * 1.3f * cos(fidx.angle + 1.5f * PI)),
					offset.y + (int) (fidx.y + trackRectSize * 1.3f * sin(fidx.angle + 1.5f * PI))),
				// Bottom left corner
				Point(offset.x + (int) (fidx.x + trackRectSize * cos(fidx.angle + 1.25f * PI)),
					offset.y + (int) (fidx.y + trackRectSize * sin(fidx.angle + 1.25f * PI)))
			};
			int npt[] = { 5 };
			const Scalar &color = trackColors.at(fidx.id % trackColors.size());
			fillConvexPoly(frameMat, points, *npt, color);
			const Point* ppt[] = { points };
			const Scalar outlineColor = color * 0.7;
			polylines(frameMat, ppt, npt, 1, true, outlineColor, 3, CV_AA);

			// Draw the tracked object name
			if (!trackNames.empty()) {
				std::string name = trackNames.at(fidx.id % trackNames.size());
				int baseLine;
				Size &textSize = getTextSize(name, FONT_HERSHEY_PLAIN, 2.0, 2, &baseLine);
				Point textPos(offset.x + (int) fidx.x - textSize.width / 2,
					offset.y + (int) fidx.y + textSize.height / 2);
				putText(frameMat, name, textPos, FONT_HERSHEY_PLAIN, 2.0, fontColor, 2, CV_AA);
			}
		}
	}

	imshow("input", frameMat);
}
