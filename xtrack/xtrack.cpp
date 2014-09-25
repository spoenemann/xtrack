/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// xtrack.cpp : Defines the entry point for the console application.

#include <csignal>
#include <ctime>
#include <cmath>

#include "stdafx.h"
#include "fiducials.h"
#include "tuio.h"
#include "display.h"
#include "record.h"

const float CLOCK_FACTOR = CLOCKS_PER_SEC / 1000.0f;

static bool term_requested = false;

static void signal_term(int signal) {
	term_requested = true;
}

void process(std::unordered_map<std::string, std::string> &parameters);

// The main function of the application.
int _tmain(int argc, _TCHAR* argv[])
{
	signal(SIGTERM, signal_term);
	signal(SIGINT, signal_term);

	// Parse the command line parameters
	std::unordered_map<std::string, std::string> parameters;
	for (int i = 1; i < argc; i++) {
		std::string param = argv[i];
		size_t equalsIndex = param.find_first_of('=', 1);
		if (equalsIndex != std::string::npos && equalsIndex < param.size() - 1) {
			std::string key = param.substr(0, equalsIndex);
			std::string value = param.substr(equalsIndex + 1, param.size() - equalsIndex - 1);
			parameters[key] = value;
		}
	}

	// Do the processing
	try {
		process(parameters);
	} catch (int exception) {
		return exception;
	}

	return 0;
}

// Print the tracked fiducials on the command line.
void printFiducials(FiducialX fiducials[], int numFiducials) {
	int printedFids = 0;
	for (int i = 0; i < numFiducials; i++) {
		FiducialX &fidx = fiducials[i];
		if (fidx.id >= 0) {
			if (printedFids > 0) {
				std::cout << "  |  ";
			}
			std::cout << "id " << fidx.id << " (" << (int) fidx.x << ", " << (int) fidx.y
				<< " / " << (int) (fidx.angle / (2 * PI) * 360) << ")";
			printedFids++;
		}
	}
	if (printedFids > 0) {
		std::cout << "\n";
	}
}

using namespace cv;

// Display the contrast image in a window.
void displayContrastImage(InputArray input) {
	Mat frameMat = input.getMat();
	int centerx = frameMat.cols / 2;
	int centery = frameMat.rows / 2;
	const Scalar color(160);

	// Draw the center position
	line(frameMat, Point(centerx - 20, centery), Point(centerx + 20, centery), color, 2);
	line(frameMat, Point(centerx, centery - 20), Point(centerx, centery + 20), color, 2);

	// Draw the arena
	circle(frameMat, Point(centerx, centery), 500, color, 2, CV_AA);

	imshow("contrast", frameMat);
}

// Process the camera stream until the application is quit.
void process(std::unordered_map<std::string, std::string> &parameters) {
	// Create the camera capture
	VideoCapture capture(intParam(parameters, PARAM_CAMERA, DEFAULT_CAMERA));
	if (!capture.isOpened()) {
        std::cerr << "Camera cannot be opened\n";
		throw 1;
	}
	capture.set(CV_CAP_PROP_FRAME_WIDTH, intParam(parameters, PARAM_FRAME_WIDTH, DEFAULT_FRAME_WIDTH));
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, intParam(parameters, PARAM_FRAME_HEIGHT, DEFAULT_FRAME_HEIGHT));

	// Read command line parameters
	int frameTime = intParam(parameters, PARAM_FRAME_TIME, DEFAULT_FRAME_TIME);
	int thresholdVal = intParam(parameters, PARAM_THRESHOLD, DEFAULT_THRESHOLD);
	bool makeQuadratic = boolParam(parameters, PARAM_QUADRATIC, DEFAULT_QUADRATIC);
	bool showInputWindow = boolParam(parameters, PARAM_SHOW_INPUT, DEFAULT_SHOW_INPUT);
	bool showContrastWindow = boolParam(parameters, PARAM_SHOW_CONTRAST, DEFAULT_SHOW_CONTRAST);
	bool printData = boolParam(parameters, PARAM_PRINT, DEFAULT_PRINT);
	
	// Initialize processing data
	if (showContrastWindow) {
		namedWindow("contrast", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
	}
	Size actualFrameSize((int) capture.get(CV_CAP_PROP_FRAME_WIDTH),
			(int) capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	Size trackedFrameSize(makeQuadratic ? actualFrameSize.height : actualFrameSize.width,
			actualFrameSize.height);
	CameraDisplay *cameraDisplay = NULL;
	if (showInputWindow) {
		cameraDisplay = new CameraDisplay(parameters, actualFrameSize);
	}
	FiducialFinder fiducialFinder(trackedFrameSize);
	TuioServer tuioServer(parameters, trackedFrameSize);
	CameraRecorder cameraRecorder(parameters, trackedFrameSize);
	RecordMode recordMode = NORMAL;

	Mat grayScaleMat, thresholdMat;

	do {
		float frameStartClock = clock() * CLOCK_FACTOR;

		// Capture a frame
		Mat frameMat;
		capture >> frameMat;
		if (frameMat.cols == 0 || frameMat.rows == 0) {
			std::cout << "No image from camera.\n";
			break;
		}

		// Cut the frame to make it quadratic
		Mat quadrMat(frameMat, makeQuadratic
			? Rect((frameMat.cols - frameMat.rows) / 2, 0, frameMat.rows, frameMat.rows)
			: Rect(0, 0, frameMat.cols, frameMat.rows));

		// Convert to grayscale
		if (quadrMat.channels() == 3) {
			cvtColor(quadrMat, grayScaleMat, CV_BGR2GRAY);
		} else {
			grayScaleMat = quadrMat;
		}

		// Apply a threshold
		threshold(grayScaleMat, thresholdMat, thresholdVal, 255, THRESH_BINARY);

		// Find fiducials
		int fidCount = fiducialFinder.findFiducials(thresholdMat);

		// Send TUIO message
		tuioServer.sendMessage(fiducialFinder.fiducials, fidCount);

		// Display the contrast image in a window
        if (showContrastWindow) {
			displayContrastImage(thresholdMat);
		}

		// Print fiducial data
		if (printData) {
			printFiducials(fiducialFinder.fiducials, fidCount);
		}

		// Draw tracking information in the image to be displayed
		if (cameraDisplay != NULL && recordMode != PLAYBACK) {
			cameraDisplay->drawTrackingInfo(quadrMat, fiducialFinder.fiducials, fidCount);
		}

		// Record or play back video
		switch (recordMode) {
		case RECORDING:
			cameraRecorder.recordFrame(quadrMat);
			break;
		case PLAYBACK:
			cameraRecorder.playbackFrame(quadrMat);
			break;
		}

		// Display the input image or video image in a window
		if (cameraDisplay != NULL) {
			cameraDisplay->displayTrackedImage(quadrMat);
		}

		float frameEndClock = clock() * CLOCK_FACTOR;
		int waitTime = frameTime - (int) (frameEndClock - frameStartClock + 0.5f);
		// Caution: waitTime <= 0 means to wait forever
		if (waitTime <= 0) {
			waitTime = 1;
		}

		// Check user input to console
		int key = waitKey(waitTime);
		switch(key) {
		case 27:
		case 'q':
			// Quit the application
			term_requested = true;
			break;
		case 'r':
			// Start recording
			if (recordMode == NORMAL) {
				if (cameraRecorder.startRecording()) {
					recordMode = RECORDING;
				}
			}
			break;
		case 's':
			// Stop recording or playback
			switch (recordMode) {
			case RECORDING:
				cameraRecorder.stopRecording();
				break;
			case PLAYBACK:
				cameraRecorder.stopPlayback();
				break;
			}
			recordMode = NORMAL;
			break;
		case 'p':
			// Play back the last recording
			switch (recordMode) {
			case RECORDING:
				cameraRecorder.stopRecording();
				// fall through
			case NORMAL:
				if (cameraDisplay == NULL) {
					std::cerr << "Cannot play back video because the '" << PARAM_SHOW_INPUT << "' parameter is disabled."; 
				} else if (cameraRecorder.startPlayback()) {
					recordMode = PLAYBACK;
				}
				break;
			}
			break;
		}
	} while (!term_requested);

	switch (recordMode) {
	case RECORDING:
		cameraRecorder.stopRecording();
		break;
	case PLAYBACK:
		cameraRecorder.stopPlayback();
		break;
	}
	if (cameraDisplay != NULL) {
		delete cameraDisplay;
	}
}
