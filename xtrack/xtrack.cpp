// xtrack.cpp : Defines the entry point for the console application.

#include <csignal>
#include "stdafx.h"
#include "fiducials.h"
#include "tuio.h"

const float PI = 3.14159265358979f;

static bool term_requested = false;

static void signal_term(int signal) {
	term_requested = true;
}

void process(std::unordered_map<std::string, std::string> parameters);

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

void printFiducials(FiducialX fiducials[], int numFiducials) {
	int printedFids = 0;
	for (int i = 0; i < numFiducials; i++) {
		FiducialX *fidx = &fiducials[i];
		if (fidx->id >= 0) {
			if (printedFids > 0) {
				std::cout << "  |  ";
			}
			std::cout << "id " << fidx->id << " (" << fidx->x << ", " << fidx->y
				<< " / " << (fidx->angle / (2 * PI) * 360) << ")";
			printedFids++;
		}
	}
	if (printedFids > 0) {
		std::cout << "\n";
	}
}

using namespace cv;

void process(std::unordered_map<std::string, std::string> parameters) {
	// Create the camera capture
	VideoCapture capture(intParam(parameters, PARAM_CAMERA, DEFAULT_CAMERA));
	if (!capture.isOpened()) {
        std::cerr << "Camera cannot be opened\n";
		throw 1;
	}
	capture.set(CV_CAP_PROP_FRAME_WIDTH, intParam(parameters, PARAM_FRAME_WIDTH, DEFAULT_FRAME_WIDTH));
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, intParam(parameters, PARAM_FRAME_HEIGHT, DEFAULT_FRAME_HEIGHT));

	int frameTime = intParam(parameters, PARAM_FRAME_TIME, DEFAULT_FRAME_TIME);
	int thresholdVal = intParam(parameters, PARAM_THRESHOLD, DEFAULT_THRESHOLD);
	bool showWindow = boolParam(parameters, PARAM_WINDOW, DEFAULT_WINDOW);
	bool printData = boolParam(parameters, PARAM_PRINT, DEFAULT_PRINT);
	if (showWindow) {
		namedWindow("camera", 1);
	}
	Mat grayScaleMat, thresholdMat;
	FiducialFinder fiducialFinder((int) capture.get(CV_CAP_PROP_FRAME_WIDTH),
		(int) capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	TuioServer tuioServer(parameters);

	do {

		// Capture a frame
		Mat frameMat;
        capture >> frameMat;

		// Convert to grayscale
        cvtColor(frameMat, grayScaleMat, CV_BGR2GRAY);

		// Apply a threshold
		threshold(grayScaleMat, thresholdMat, thresholdVal, 255, THRESH_BINARY);

		// Display in the window
        if (showWindow) {
			imshow("camera", thresholdMat);
		}

		// Find fiducials
		int fidCount = fiducialFinder.findFiducials(&thresholdMat);

		// Send TUIO message
		tuioServer.sendMessage(fiducialFinder.fiducials, fidCount, thresholdMat.cols, thresholdMat.rows);

		// Print fiducial data
		if (printData) {
			printFiducials(fiducialFinder.fiducials, fidCount);
		}

	} while (!term_requested && waitKey(frameTime) < 0);
}
