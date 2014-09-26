Xtrack
======

Fiducial tracking application for Windows, licensed under EPL.
The fiducial symbols are available at
http://reactivision.sourceforge.net/data/fiducials.pdf
and can be printed on white paper.

Two libraries are included here, both licensed under LGPL:
* libfidtrack (http://reactivision.sourceforge.net/) for tracking fiducials
* WOscLib (http://wosclib.sourceforge.net/) for sending TUIO messages

OpenCV (http://opencv.org) is required for compiling Xtrack, and is not included
in the repository. We tested it with OpenCV version 2.4.9.

Xtrack can be configured with `key=value` formatted command line parameters.
The available parameters are documented in `xtrack/parameters.h`
(see also `xtrack.bat`).


### User Interface

Depending on the command line parameters, up to two windows are opened after the
application has started: an *input image* window and a *contrast image* window.
The input image window shows the camera input augmented with symbols and optional
text marking the positions of detected fiducial symbols. The contrast image shows
the processing result that is used to detect fiducials. The camera brightness or
the threshold parameter should be adjusted such that all black and white dots are
clearly visible in the contrast image.

Xtrack supports recording and playback of video files when the input image
window is available. Use the following keys to control the application.
* **R**: start recording
* **P**: start playback
* **S**: stop recording or playback
* **Q** or **Esc**: quit Xtrack
