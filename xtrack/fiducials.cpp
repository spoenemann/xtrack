/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Fiducial tracking using libfidtrack (http://reactivision.sourceforge.net/)

#include "fiducials.h"

FiducialFinder::FiducialFinder(cv::Size &size) {
	initialize_treeidmap(&treeidmap);

	dmap = new ShortPoint[size.height * size.width];
	for (int y = 0; y < size.height; y++) {
		for (int x = 0; x < size.width; x++) {
			dmap[y * size.width + x].x = x;
			dmap[y * size.width + x].y = y;
		}
	}

	initialize_fidtrackerX(&fidtrackerx, &treeidmap, dmap);
	initialize_segmenter(&segmenter, size.width, size.height, treeidmap.max_adjacencies);
}

FiducialFinder::~FiducialFinder() {
	delete dmap;
}

int FiducialFinder::findFiducials(cv::InputArray input) {
	cv::Mat frame = input.getMat();
	step_segmenter(&segmenter, frame.data);
	return find_fiducialsX(fiducials, MAX_FIDUCIALS,
			&fidtrackerx, &segmenter, frame.cols, frame.rows);
}
