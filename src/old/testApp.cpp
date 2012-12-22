#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	// **** COMPUTER SPECIFIC VARIABLES **** //
	string arduinoPort = "\\\\.\\COM24"; // Sean, Windows, Uno
	//string arduinoPort = "/dev/cu.usbserial-A70064Yu"; // Sean, Mac, Arduino Decimila
	// **** END COMPUTER SPECIFIC VARIABLES **** //


	// **** OPTIONS **** //

	// Set up the frequency cycle here
	// First number of each pair sets the frequency
	// Second number of each pair sets the duration that frequency is delivered
	const int nFreqs = 4;
	float freqCycle[nFreqs][2] = { // { frequency, duration(seconds) }
		{5, 2}, 
		{10, 1},
		{1, 4},
		{15, 1},
	};
	freqOutThread.SetFreqCycle(nFreqs, freqCycle);

	midiChannel = 1;
	midiId = 60;
	midiValue = 100;

	int ledBrightness = 255;
	int ledPin = 11;
	// **** END OPTIONS **** //


	// **** GENERAL SETUP **** //
	freqOutThread.setupLights(arduinoPort, 57600, ledPin, ledBrightness);
	freqOutThread.setupMidi(0, midiChannel, midiId, midiValue);

	// Start threads
	thread.startThread(true, false);
	freqOutThread.startThread(true, false);

	// **** END GENERAL SETUP **** //
}

//--------------------------------------------------------------
void testApp::update(){
	//printf("update(): %f\n", ofGetElapsedTimef());
	//printf("%s\n", ofGetTimestampString().c_str());
}

//--------------------------------------------------------------
void testApp::draw(){
	printf("draw() - %f (s)\n", ofGetElapsedTimef());
	//printf("10 - %s\n", ofGetTimestampString().c_str());
	ofSleepMillis(1000);
}

//--------------------------------------------------------------
void testApp::exit(){
	printf("exit()");
	// Stop threads
	thread.stopThread();
	freqOutThread.stopThread();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if (((char) key) == '+') {
        midiMapMode = !midiMapMode;
		freqOutThread.toggleMidiOut();
    }
	if (((char) key) == '1') {
        midiout.sendControlChange(midiChannel, midiId, midiValue);
    } 
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
