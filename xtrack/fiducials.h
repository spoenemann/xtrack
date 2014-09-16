// Fiducial tracking using libfidtrack (http://reactivision.sourceforge.net/)

#pragma once

#include "stdafx.h"
#include "segment.h"

#define MAX_FIDUCIALS 16

class FiducialFinder {
public:
	FiducialX fiducials[MAX_FIDUCIALS];

	FiducialFinder(int width, int height);
	~FiducialFinder();

	void findFiducials(cv::Mat*);

private:
	Segmenter segmenter;
	TreeIdMap treeidmap;
	FidtrackerX fidtrackerx;
	ShortPoint *dmap;
};
