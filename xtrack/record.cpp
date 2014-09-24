/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Recording the camera image and playing recorded files

#include <windows.h>
#include "record.h"

const std::string FILE_PREFIX = "xtrack";
const std::string FILE_EXT = ".avi";

using namespace cv;

CameraRecorder::CameraRecorder(std::unordered_map<std::string, std::string> &parameters, cv::Size &fsize) {
	int frameTime = intParam(parameters, PARAM_FRAME_TIME, DEFAULT_FRAME_TIME);
	if (frameTime <= 0) {
		std::cerr << "Illegal value given for parameter " << PARAM_FRAME_TIME << "\n";
        throw 1;
	}
	this->videoDir = stringParam(parameters, PARAM_RECORD_DIR, DEFAULT_RECORD_DIR);
	if (videoDir.at(videoDir.length() - 1) != '\\') {
		videoDir.append("\\");
	}
	this->frameRate = 1000 / frameTime;
	std::string codecStr = stringParam(parameters, PARAM_CODEC, DEFAULT_CODEC);
	if (codecStr.length() == 4) {
		this->codec = CV_FOURCC(codecStr.at(0), codecStr.at(1), codecStr.at(2), codecStr.at(3));
	} else {
		// This special value means open a dialog for selecting a codec
		this->codec = -1;
	}
	this->origSize = fsize;
	double recordScale = doubleParam(parameters, PARAM_RECORD_SCALE, DEFAULT_RECORD_SCALE);
	this->recordSize = Size((int) (recordScale * fsize.width), (int) (recordScale * fsize.height));
	this->lastPlayedFileNum = -1;
}

bool CameraRecorder::startRecording() {
	int nextFileNum = getLastFileNum() + 1;
	std::string fileName = getFileName(nextFileNum);
	videoWriter.open(fileName, codec, frameRate, recordSize);
	if (videoWriter.isOpened()) {
		std::cout << "Recording to file " << fileName << "\n";
		lastPlayedFileNum = nextFileNum;
	} else {
		std::cerr << "Could not open output file " << fileName << " for video recording.\n";
		return false;
	}
	return true;
}

void CameraRecorder::recordFrame(InputArray input) {
	Mat resizedMat;
	resize(input.getMat(), resizedMat, recordSize);
	videoWriter << resizedMat;
}

void CameraRecorder::stopRecording() {
	videoWriter.release();
	std::cout << "Stopped recording.\n";
}

bool CameraRecorder::startPlayback() {
	int nextFileNum = lastPlayedFileNum;
	if (lastPlayedFileNum < 0) {
		nextFileNum = getNextFileNum(lastPlayedFileNum);
	}
	std::string fileName = getFileName(nextFileNum);
	videoReader.open(fileName);
	if (videoReader.isOpened()) {
		std::cout << "Starting playback at file " << fileName << "\n";
		lastPlayedFileNum = nextFileNum;
	} else {
		std::cerr << "No video file found for playback.\n";
		return false;
	}
	return true;
}

void CameraRecorder::playbackFrame(OutputArray output) {
	Mat videoFileFrame;
	videoReader >> videoFileFrame;
	while (videoFileFrame.rows == 0 || videoFileFrame.cols == 0) {
		videoReader.release();
		lastPlayedFileNum = getNextFileNum(lastPlayedFileNum);
		std::string fileName = getFileName(lastPlayedFileNum);
		videoReader.open(fileName);
		videoReader >> videoFileFrame;
		std::cout << "Next playback at file " << fileName << "\n";
	}
	resize(videoFileFrame, output, origSize);
}

void CameraRecorder::stopPlayback() {
	videoReader.release();
	std::cout << "Stopped playback.\n";
}

int CameraRecorder::getNextFileNum(const int num) {
	int nextFileNum = -1;

    WIN32_FIND_DATA file_data;
	static const std::string pattern = videoDir + FILE_PREFIX + "*" + FILE_EXT;
	HANDLE fileSearch = FindFirstFile(pattern.c_str(), &file_data);
    if (fileSearch != INVALID_HANDLE_VALUE) {
		do {
			if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				std::string fileName = file_data.cFileName;
				std::stringstream convert(fileName.substr(FILE_PREFIX.length(),
					fileName.length() - FILE_PREFIX.length() - FILE_EXT.length()));
				int fileNum;
				if ((convert >> fileNum) && (
						nextFileNum < 0
						|| fileNum > num && (nextFileNum <= num || fileNum < nextFileNum)
						|| nextFileNum <= num && fileNum < nextFileNum)) {
					nextFileNum = fileNum;
				}
			}
		} while (FindNextFile(fileSearch, &file_data));
		FindClose(fileSearch);
	}

	return nextFileNum;
}

int CameraRecorder::getLastFileNum() {
	int lastFileNum = 0;

    WIN32_FIND_DATA file_data;
	static const std::string pattern = videoDir + FILE_PREFIX + "*" + FILE_EXT;
	HANDLE fileSearch = FindFirstFile(pattern.c_str(), &file_data);
    if (fileSearch != INVALID_HANDLE_VALUE) {
		do {
			if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				std::string fileName = file_data.cFileName;
				std::stringstream convert(fileName.substr(FILE_PREFIX.length(),
					fileName.length() - FILE_PREFIX.length() - FILE_EXT.length()));
				int fileNum;
				if ((convert >> fileNum) && fileNum > lastFileNum) {
					lastFileNum = fileNum;
				}
			}
		} while (FindNextFile(fileSearch, &file_data));
		FindClose(fileSearch);
	}

	return lastFileNum;
}

std::string CameraRecorder::getFileName(const int fileNum) {
	std::ostringstream output;
	output << videoDir;
	output << FILE_PREFIX;
	output.width(3);
	output.fill('0');
	output << fileNum;
	output << FILE_EXT;
	return output.str();
}
