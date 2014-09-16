// xtrack.cpp : Defines the entry point for the console application.

#include <csignal>
#include <unordered_map>
#include "stdafx.h"
#include "opencv2/opencv.hpp"

// Parameter definitions

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

int intParam(std::unordered_map<std::string, std::string> parameters,
		std::string key, int defaultValue) {
	int result = defaultValue;
	if (parameters.find(key) != parameters.end()) {
		std::stringstream convert(parameters[key]);
		if (!(convert >> result)) {
			std::cerr << "Invalid value set for parameter " << key << "\n";
			throw 1;
		}
	}
	return result;
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
    namedWindow("test", 1);
	Mat grayScaleMat, thresholdMat;

	do {
		// Capture a frame
		Mat frameMat;
        capture >> frameMat;
		// Convert to grayscale
        cvtColor(frameMat, grayScaleMat, CV_BGR2GRAY);
		// Apply a threshold
		threshold(grayScaleMat, thresholdMat, thresholdVal, 255, THRESH_BINARY);
		// Display in the window
        imshow("test", thresholdMat);
	} while (!term_requested && waitKey(frameTime) < 0);
}
