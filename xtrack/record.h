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

	// Start recording a new video with the selected codec. If no coded was selected,
	// a dialog is opened listing the available codecs.
	bool startRecording();
	// Record the next frame.
	void recordFrame(cv::InputArray input);
	// Stop recording.
	void stopRecording();
	// Start playback of the last recorded or played video file.
	bool startPlayback();
	// Play back the next frame.
	void playbackFrame(cv::OutputArray output);
	// Stop playback.
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
