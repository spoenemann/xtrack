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

CameraDisplay::CameraDisplay(std::unordered_map<std::string, std::string> &parameters, cv::Size &screenSize) {
	namedWindow("input", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
	cvSetWindowProperty("input", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	this->trackRectSize = intParam(parameters, PARAM_TRACK_RECT_SIZE, DEFAULT_TRACK_RECT_SIZE);
	this->screenSize = screenSize;

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
}

void CameraDisplay::drawTrackingInfo(Mat &frameMat, TrackedFiducial fiducials[]) {
	// Draw tracking information for fiducials
	for (int i = 0; i < MAX_FIDUCIALS; i++) {
		TrackedFiducial &fid = fiducials[i];
		if (fid.isTracked) {
			float fidx = fid.x * frameMat.cols;
			float fidy = fid.y * frameMat.rows;
			// Draw a rotated rectangle
			const Point points[] = {
				// Top left corner
				Point((int) (fidx + trackRectSize * cos(fid.a + 0.75f * PI)),
					(int) (fidy + trackRectSize * sin(fid.a + 0.75f * PI))),
				// Top right corner
				Point((int) (fidx + trackRectSize * cos(fid.a + 0.25f * PI)),
					(int) (fidy + trackRectSize * sin(fid.a + 0.25f * PI))),
				// Bottom right corner
				Point((int) (fidx + trackRectSize * cos(fid.a + 1.75f * PI)),
					(int) (fidy + trackRectSize * sin(fid.a + 1.75f * PI))),
				// Arrow head
				Point((int) (fidx + trackRectSize * 1.3f * cos(fid.a + 1.5f * PI)),
					(int) (fidy + trackRectSize * 1.3f * sin(fid.a + 1.5f * PI))),
				// Bottom left corner
				Point((int) (fidx + trackRectSize * cos(fid.a + 1.25f * PI)),
					(int) (fidy + trackRectSize * sin(fid.a + 1.25f * PI)))
			};
			int npt[] = { 5 };
			const Scalar &color = trackColors.at(i % trackColors.size());
			fillConvexPoly(frameMat, points, *npt, color);
			const Point* ppt[] = { points };
			const Scalar outlineColor = color * 0.7;
			polylines(frameMat, ppt, npt, 1, true, outlineColor, 3, CV_AA);

			// Draw the tracked object name
			if (!trackNames.empty()) {
				std::string name = trackNames.at(i % trackNames.size());
				int baseLine;
				Size &textSize = getTextSize(name, FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseLine);
				Point textPos((int) fidx - textSize.width / 2,
					(int) fidy + textSize.height / 2);
				putText(frameMat, name, textPos, FONT_HERSHEY_SIMPLEX, 0.8, fontColor, 2, CV_AA);
			}
		}
	}
}

void CameraDisplay::displayTrackedImage(InputArray input) {
	Mat frameMat = input.getMat();
	Size frameSize = frameMat.size();
	Mat displayMat = Mat::zeros(screenSize, frameMat.type());
	frameMat.copyTo(Mat(displayMat, Rect(
		(screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2,
		frameSize.width, frameSize.height)));
	imshow("input", displayMat);
}
