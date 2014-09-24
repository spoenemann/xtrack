/*******************************************************************************
 * Copyright (c) 2014 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

// Fiducial tracking using libfidtrack (http://reactivision.sourceforge.net/)

#pragma once

#include "stdafx.h"
#include "segment.h"

#define MAX_FIDUCIALS 16

class FiducialFinder {
public:
	FiducialX fiducials[MAX_FIDUCIALS];

	FiducialFinder(cv::Size &fsize);
	~FiducialFinder();

	int findFiducials(cv::InputArray);

private:
	Segmenter segmenter;
	TreeIdMap treeidmap;
	FidtrackerX fidtrackerx;
	ShortPoint *dmap;
};
