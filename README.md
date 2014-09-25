Xtrack
======

Fiducial tracking application for Windows, licensed under EPL.

Two libraries are included here, both licensed under LGPL:
* libfidtrack (http://reactivision.sourceforge.net/) for tracking fiducials
* WOscLib (http://wosclib.sourceforge.net/) for sending TUIO messages

OpenCV (http://opencv.org) is required for compiling Xtrack, and is not included
in the repository. We tested it with OpenCV version 2.4.9.

Xtrack can be configured with `key=value` formatted command line parameters.
The available parameters are documented in `xtrack/parameters.h`
(see also `xtrack.bat`).
