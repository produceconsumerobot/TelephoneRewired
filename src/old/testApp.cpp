#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	// **** COMPUTER SPECIFIC VARIABLES **** //
	//string arduinoPort = "\\\\.\\COM24"; // Sean, Windows, Uno
	string arduinoPort = "tty.usbmodemfa141"; // Sean, Mac, Arduino Decimila
	//string arduinoPort = "/dev/cu.usbserial-A70064Yu"; // Sean, Mac, Arduino Decimila
    //tty.usbmodemfa141
    //cu.usbmodemfa141
	//string zeoPort = "\\\\.\\COM26";
	// **** END COMPUTER SPECIFIC VARIABLES **** //


	// **** OPTIONS **** //

	// Set up the frequency cycle here
	// First number of each pair sets the frequency
	// Second number of each pair sets the duration that frequency is delivered
	
	
	// 5x sped up version of Mitch Altman's Brain Machine Sequence
	const int nFreqs = 43;
	float freqCycle[nFreqs][2] = { // { frequency, duration(seconds) }
		{BETA, 12}, //1
		{ALPHA, 2}, //2
		{BETA, 4}, //3
		{ALPHA, 3}, //4
		{BETA, 3}, //5
		{ALPHA, 4}, //6
		{BETA, 2}, //7
		{ALPHA, 6}, //8
		{BETA, 1}, //9
		{ALPHA, 12}, //10
		{THETA, 2}, //11
		{ALPHA, 6}, //12
		{THETA, 4}, //13
		{ALPHA, 6}, //14
		{THETA, 6}, //15
		{ALPHA, 3},  //16
		{THETA, 12}, //17
		{ALPHA, 3}, //18
		{BETA, 0.2}, //19
		{ALPHA, 3}, //20
		{THETA, 12}, //21
		{DELTA, 1}, //22
		{THETA, 2}, //23
		{DELTA, 1}, //24
		{THETA, 2},//25
		{DELTA, 1}, //26
		{THETA, 6},//27
		{ALPHA, 3},//28
		{BETA, 0.2},//29
		{ALPHA, 3},//30
		{THETA, 6}, //31
		{ALPHA, 3},//32
		{BETA, 0.2},//33
		{ALPHA, 4},//34
		{BETA, 1},//35
		{ALPHA, 4}, //36
		{BETA, 3},//37
		{ALPHA, 3},//38
		{BETA, 4},//39
		{ALPHA, 2},//40
		{BETA, 5}, //41
		{ALPHA, 1},//42
		{BETA, 12},//43
	};
	
	
	/*
	const int nFreqs = 4;
	float freqCycle[nFreqs][2] = { // { frequency, duration(seconds) }
		{30, 4}, //1
		{40, 4}, //1
		{50, 4}, //3
		{60, 4}, //5
	};
	*/
	

	freqOutThread.SetFreqCycle(nFreqs, freqCycle);
	
	midiChannel = 1;
	midiId = 60;
	midiValue = 100;

	int ledBrightness = 255;
	int ledPin = 11;
	// **** END OPTIONS **** //


	// **** GENERAL SETUP **** //
	// Setup outputs
	freqOutThread.setupLights(arduinoPort, 57600, ledPin, ledBrightness);
	// Setup MIDI port
	midiout.listPorts();
	midiout.openPort(0);
	freqOutThread.setupMidi(&midiout, midiChannel, midiId, midiValue);
	// Setup zeo
	//zeo.setupSerial(zeoPort, 38400);

	//ofSetupOpenGL(1920,1200, OF_FULLSCREEN);
	//ofSetWindowShape(1920,1200);
	//ofSetBackgroundAuto(false);
	ofSetVerticalSync(true);
    

	outState = true;
	prevLoopTime = ofGetElapsedTimef();
	absMaxOutDelay = 0;
	absAveOutDelay = 0;
	prevFreq = 0;

		
	//ofAddListener(zeo.dataReady,this,&testApp::newData);
	
	// Start threads
	thread.startThread(true, false);
	// Start outputs thread
	freqOutThread.startThread(true, false);
	// Start zeo thread
	//zeo.startThread(true, false);

	// **** END GENERAL SETUP **** //
}

//--------------------------------------------------------------
void testApp::update(){
	//printf("update(): %f\n", ofGetElapsedTimef());
	//printf("%s\n", ofGetTimestampString().c_str());
}

//--------------------------------------------------------------
void testApp::draw(){
	
	// ***** Code for testing the output display delays ***** //
	/*
     if (outState != freqOutThread.getCurrentOutState()) {
		outState = !outState;
		int outDelay = ((int)((ofGetElapsedTimef() - prevLoopTime)*1000)) - freqOutThread.getCurrentOutDelay();
		int absOutDelay = abs(outDelay);
		if (prevFreq != freqOutThread.GetCurrentFreq()) {
			absMaxOutDelay = 0;
			prevFreq = freqOutThread.GetCurrentFreq();
			absOutDelay = 0;
		}
		if (absMaxOutDelay < absOutDelay) absMaxOutDelay = absOutDelay;
		float decay = 20;
		absAveOutDelay = (absAveOutDelay*(decay-1.) + absOutDelay)/decay;
		printf("absMax: %i, absAve: %.1f, Delay: %i\n", absMaxOutDelay, absAveOutDelay, outDelay);
		prevLoopTime = ofGetElapsedTimef();
	}
     */
	// ***** END Code for testing the output display delays ***** //
	
    if (freqOutThread.getCurrentOutState()) {
        ofBackground(0, 0, 0);
    } else {
        ofBackground(255, 255, 255);
    }
    

	//printf("draw() - %f (s)\n", ofGetElapsedTimef());
	
	//ofSetBackgroundColor(255,255,255);
	//ofSleepMillis(1);
    
    //ofBackground(0, 0, 0);
    //ofSetBackgroundColor(0, 0, 0);
}

//--------------------------------------------------------------
void testApp::exit(){
	printf("exit()");
	// Stop threads
	thread.stopThread();
	freqOutThread.stopThread();
	midiout.closePort();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if (((char) key) == '+') {
        midiMapMode = !midiMapMode;
		freqOutThread.toggleMidiOut();
    }
	if (((char) key) == '1') {
        midiout.sendControlChange(midiChannel, midiId, midiValue);
        printf("1");
    } 
	if ( key == 'f') {
		ofToggleFullscreen();
	}
	if ( key == 'r') {
		absMaxOutDelay = 0;
	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

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
