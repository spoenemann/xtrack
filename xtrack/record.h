/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Recording the camera image and playing recorded files

#pragma once

#include "stdafx.h"

enum RecordMode {
	NORMAL, RECORDING, PLAYBACK
};

class CameraRecorder {
public:
	CameraRecorder(std::unordered_map<std::string, std::string> &parameters, cv::Size &fsize);

	bool startRecording();
	void recordFrame(cv::InputArray input);
	void stopRecording();
	bool startPlayback();
	void playbackFrame(cv::OutputArray output);
	void stopPlayback();

private:
	cv::VideoWriter videoWriter;
	cv::VideoCapture videoReader;
	std::string videoDir;
	int frameRate;
	int codec;
	cv::Size origSize;
	cv::Size recordSize;
	int lastPlayedFileNum;
	
	int getNextFileNum(const int num);
	int getLastFileNum();
	std::string getFileName(const int fileNum);

};
