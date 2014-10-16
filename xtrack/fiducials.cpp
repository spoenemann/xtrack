/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Fiducial tracking using libfidtrack (http://reactivision.sourceforge.net/)

#include "fiducials.h"

bool isNaN(float f) {
	return f != f;
}

FiducialFinder::FiducialFinder(cv::Size &fsize) {
	this->fsize = fsize;
	initialize_treeidmap(&treeidmap);

	dmap = new ShortPoint[fsize.height * fsize.width];
	for (int y = 0; y < fsize.height; y++) {
		for (int x = 0; x < fsize.width; x++) {
			dmap[y * fsize.width + x].x = x;
			dmap[y * fsize.width + x].y = y;
		}
	}

	initialize_fidtrackerX(&fidtrackerx, &treeidmap, dmap);
	initialize_segmenter(&segmenter, fsize.width, fsize.height, treeidmap.max_adjacencies);

	for (int i = 0; i < MAX_FIDUCIALS; i++) {
		TrackedFiducial &trackedFid = trackedFiducials[i];
		trackedFid.isTracked = false;
		trackedFid.timestamp = 0.0;
		float nan = std::numeric_limits<float>::quiet_NaN();
		trackedFid.x = nan;
		trackedFid.y = nan;
		trackedFid.a = nan;
		trackedFid.xspeed = nan;
		trackedFid.yspeed = nan;
		trackedFid.aspeed = nan;
		trackedFid.xyacc = nan;
		trackedFid.aacc = nan;
	}
}

FiducialFinder::~FiducialFinder() {
	delete dmap;
}

int FiducialFinder::findFiducials(cv::InputArray input, double timestamp) {
	cv::Mat frame = input.getMat();
	step_segmenter(&segmenter, frame.data);
	int num = find_fiducialsX(rawFiducials, MAX_FIDUCIALS,
			&fidtrackerx, &segmenter, frame.cols, frame.rows);

	// Transfer the raw fiducial data to the tracked fiducial data and derive speed values
	double secTime = timestamp / 1000;
	int tracked = 0;
	for (int i = 0; i < num; i++) {
		FiducialX &fidx = rawFiducials[i];
		if (fidx.id >= 0 && fidx.id < MAX_FIDUCIALS) {
			TrackedFiducial &trackedFid = trackedFiducials[fidx.id];
			if (!trackedFid.isTracked || trackedFid.timestamp != secTime) {
				trackedFid.isTracked = true;
				float timeDiff = (float) (secTime - trackedFid.timestamp);
				trackedFid.timestamp = secTime;

				float oldx = trackedFid.x;
				float oldxSpeed = trackedFid.xspeed;
				trackedFid.x = fidx.x / fsize.width;
				if (!isNaN(oldx)) {
					trackedFid.xspeed = (trackedFid.x - oldx) / timeDiff;
				}

				float oldy = trackedFid.y;
				float oldySpeed = trackedFid.yspeed;
				trackedFid.y = fidx.y / fsize.height;
				if (!isNaN(oldy)) {
					trackedFid.yspeed = (trackedFid.y - oldy) / timeDiff;
				}

				if (!isNaN(oldxSpeed) && !isNaN(oldySpeed)) {
					float oldSpeed = sqrt(oldxSpeed * oldxSpeed + oldySpeed * oldySpeed);
					float newSpeed = sqrt(trackedFid.xspeed * trackedFid.xspeed + trackedFid.yspeed * trackedFid.yspeed);
					trackedFid.xyacc = (newSpeed - oldSpeed) / timeDiff;
				}

				float olda = trackedFid.a;
				float oldaSpeed = trackedFid.aspeed;
				trackedFid.a = fidx.angle;
				if (!isNaN(olda)) {
					trackedFid.aspeed = (trackedFid.a - olda) / timeDiff;
					if (!isNaN(oldaSpeed)) {
						trackedFid.aacc = (trackedFid.aspeed - oldaSpeed) / timeDiff;
					}
				}
			}
		}
	}

	// Mark the fiducials that have not been tracked
	for (int i = 0; i < MAX_FIDUCIALS; i++) {
		if (trackedFiducials[i].timestamp != secTime) {
			trackedFiducials[i].isTracked = false;
		}
	}
	return tracked;
}
