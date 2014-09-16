// Sending TUIO messages using WOscLib (http://wosclib.sourceforge.net/)

#include "tuio.h"
#include "WOscBundle.h"

#define OSC_PATH "/tuio/2Dobj"

TuioServer::TuioServer(std::unordered_map<std::string, std::string> parameters) {
	// TODO
}

TuioServer::~TuioServer() {
}

void TuioServer::sendMessage(FiducialX fiducials[], int numFiducials, int width, int height) {
	WOscBundle *bundle = new WOscBundle();;

	// Alive message
	WOscMessage *msg = new WOscMessage(OSC_PATH);
	msg->Add("alive");
	for (int i = 0; i < numFiducials; i++) {
		FiducialX *fidx = &fiducials[i];
		if (fidx->id >= 0) {
			msg->Add(fidx->id);
		}
	}
	bundle->Add(msg);

	// Set message
	for (int i = 0; i < numFiducials; i++) {
		FiducialX *fidx = &fiducials[i];
		msg = new WOscMessage(OSC_PATH);
		msg->Add("set");
		// /tuio/2Dobj set s i x y a X Y A m r
		msg->Add(fidx->id); // blob id
		msg->Add(fidx->id); // fiducial id
		msg->Add(fidx->x / width); // x
		msg->Add(fidx->y / height); // y
		msg->Add(fidx->angle); // a
		msg->Add(0.0f); // X
		msg->Add(0.0f); // Y
		msg->Add(0.0f); // A
		msg->Add(0.0f); // m
		msg->Add(0.0f); // r
		bundle->Add(msg);
	}

	// Frame sequence message
	msg = new WOscMessage(OSC_PATH);
	msg->Add("fseq");
	msg->Add(this->fseq++);
	bundle->Add(msg);

	// TODO send the bundle

	delete bundle;
}
