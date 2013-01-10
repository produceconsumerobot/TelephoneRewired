#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	// **** COMPUTER SPECIFIC VARIABLES **** //
	//string arduinoPort = "\\\\.\\COM28"; // Sean, Windows, Uno
	string arduinoPort = "tty.usbmodemfa141"; // Sean, Mac, Arduino Decimila
	//string arduinoPort = "/dev/cu.usbserial-A70064Yu"; // Sean, Mac, Arduino Decimila
    //tty.usbmodemfa141
    //cu.usbmodemfa141
    string zeoPort = "tty.usbserial"; //Mac 
	
    //string zeoPort = "\\\\.\\COM26";
	// **** END COMPUTER SPECIFIC VARIABLES **** //


	// **** OPTIONS **** //

	showStimuli = false;
	showOscilloscope = true;
	showScreenEntrainment = false;


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
	ofBackground(255, 255, 255);
	ofSetVerticalSync(true);
    

	outState = true;
	prevLoopTime = ofGetElapsedTimef();
	absMaxOutDelay = 0;
	absAveOutDelay = 0;
	prevFreq = 0;

	zeoThread.setupSerial(zeoPort);
	ofAddListener(zeoThread.newRawData, this, &testApp::newZeoRawData);
	ofAddListener(zeoThread.newSliceData, this, &testApp::newZeoSliceData);
	
	//ofAddListener(zeo.dataReady,this,&testApp::newData);
	
	SetupOscilloscopes();

	// Start threads
	// Start outputs thread
	freqOutThread.startThread(true, false);
	// Start zeo thread
	zeoThread.startThread(true, false);
	//logger.addData(LoggerData(ofGetElapsedTimef(), LoggerData::ENTRAINMENT_FREQ, freqOutThread.GetCurrentFreq()));

	//logger.startThread(true, false);

	//stimuli[0].loadImage("data/images/01.jpg");
	//stimuli[1].loadImage("data/images/02.jpg");
	//stimuli[2].loadImage("data/images/03.jpg");

	counter = 0;
	// **** END GENERAL SETUP **** //
}

//--------------------------------------------------------------
void testApp::SetupOscilloscopes(){
	// Setup oscilloscopes
	int nScopes = 4;
	ofPoint min = ofPoint(0., 10.);
	ofPoint max = ofPoint(ofGetWindowSize().x, ofGetWindowSize().y-10);
	scopeWin = ofxMultiScope(nScopes, min, max);
	int rawTimeWindow = 15;
	int powerTimeWindow = 300;
	{ // Filtered EEG Scope
		const int nVariables = 1;
		ofColor colors[nVariables] = {ofColor(0,200,0)};
		string names[nVariables] = {"Filt EEG"};
		scopeWin.scopes[0].setup(rawTimeWindow, ZeoParser::RAW_DATA_LEN, names, colors, nVariables, 7., 0.);
	}
	{ // Power Data Scope
		const int nVariables = ZeoParser::NUM_FREQS;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200), 
			ofColor(0,200,200), ofColor(200,0,200), ofColor(200,200,0), ofColor(100,100,100)};
		string names[nVariables];
		for (int i=0; i<nVariables; i++) {
			names[i] = ZeoParser::labels[i];
		}
		scopeWin.scopes[1].setup(powerTimeWindow, 1, names, colors, nVariables, 0.07, -350.);
	}
	{ // Entrainment Signal Scope
		const int nVariables = NUM_ENTRAINMENT_FREQS;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200), 
			ofColor(0,200,200), ofColor(100,100,100)};
		string names[nVariables] = {"DELTA", "THETA", "ALPHA", "BETA", "GAMMA"};
		scopeWin.scopes[2].setup(powerTimeWindow, 1, names, colors, nVariables, 750, -350.);
	}
	{ // Data Reliability Scope
		const int nVariables = 3;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200)};
		string names[nVariables] = {"Impedance", "SQI", "Signal"};
		scopeWin.scopes[3].setup(powerTimeWindow, 1, names, colors, nVariables, 0.5, -350.);
	}
	
	// Initialize Oscilloscope data arrays	
	rawData.resize(1, vector<float>(ZeoParser::RAW_DATA_LEN, 0));
	filtData.resize(1, vector<float>(ZeoParser::RAW_DATA_LEN, 0));
	powerData.resize(ZeoParser::NUM_FREQS, vector<float>(1, 0));
	sliceData.resize(3, vector<float>(1, 0));
	entrainmentFreqData.resize(NUM_ENTRAINMENT_FREQS, vector<float>(1, 0));
}

//--------------------------------------------------------------
// Function to handle new zeo raw data
void testApp::newZeoRawData(bool & ready){
#ifdef DEBUG_PRINT
	printf("testApp::newZeoRawData\n"); 
#endif

	ZeoParser zeo = zeoThread.getZeoParser();

	// Get Zeo raw & filtered data
	float zeoData[ZeoParser::RAW_DATA_LEN];
	zeo.getFilteredData(zeoData);

	//printf("zeo.RAW_DATA_LEN=%i\n", zeo.RAW_DATA_LEN);
	// Update filtered data in scope
	for (int i=0; i<ZeoParser::RAW_DATA_LEN; i++) {
		//printf("%i,",i);
		rawData[0][i] = zeoData[i];
	}
	printf("    Raw Data:");
	for (int i=0; i<6; i++) {
		printf(" %f, %f;", rawData[0][i], zeoData[i]);
	}
	printf("\n");
	scopeWin.scopes[0].updateData(rawData, ZeoParser::RAW_DATA_LEN);

	//LoggerData temp = LoggerData(ofGetElapsedTimef(), LoggerData::RAW_DATA, &zeoData);
	//logger.addData(temp);

	// TODO raw the data to the logger
}

//--------------------------------------------------------------
// Function to handle new zeo slice data
void testApp::newZeoSliceData(bool & ready){
#ifdef DEBUG_PRINT 
	printf("testApp::newZeoSliceData\n"); 
#endif

	ZeoParser zeo = zeoThread.getZeoParser();

	// Get Zeo Slice Data
	ZeoSlice zeoSlice;
	zeo.getSlice(&zeoSlice);

	// Update Power Data in scope
	for (int i=0; i<ZeoParser::NUM_FREQS; i++) {
		powerData[i][0] = (float) zeoSlice.power[i];
		printf("    %s: %f, %i\n", ZeoParser::labels[i], powerData[i][0], zeoSlice.power[i]);
	}
	scopeWin.scopes[1].updateData(powerData, 1);

	// Update signal quality in scope
	sliceData[0][0] = zeoSlice.impendance;
	sliceData[1][0] = zeoSlice.sqi*40; // multiply for display
	sliceData[2][0] = zeoSlice.signal*1500; // multiply for display
	scopeWin.scopes[3].updateData(sliceData, 1);

	//LoggerData * temp = new LoggerData(ofGetElapsedTimef(), LoggerData::SLICE_DATA, zeoSlice);
	//int testing = 1;
	//logger.addData(*temp);
	//delete temp;

	// TODO slice data to the logger

	plotEntrainmentFreqData(freqOutThread.GetCurrentFreq());
}

//--------------------------------------------------------------
// Function to handle new entrainment data
void testApp::entrainmentOutChange(bool & output){

}

void testApp::plotEntrainmentFreqData(float freq) {
#ifdef DEBUG_PRINT 
	printf("testApp::plotEntrainmentFreqData\n"); 
#endif
	for (int i=0; i<NUM_ENTRAINMENT_FREQS; i++) {
		entrainmentFreqData[i][0] = 0.;
	}
	if ((int) freq == (int) DELTA) entrainmentFreqData[0][0] = 1.;
	if ((int) freq == (int) THETA) entrainmentFreqData[1][0] = 1.;
	if ((int) freq == (int) ALPHA) entrainmentFreqData[2][0] = 1.;
	if ((int) freq == (int) BETA) entrainmentFreqData[3][0] = 1.;
	if ((int) freq == (int) GAMMA) entrainmentFreqData[4][0] = 1.;
	printf("Entrainment = %f: ", freq);
	for (int i=0; i<NUM_ENTRAINMENT_FREQS; i++) {
		printf("%f, ", entrainmentFreqData[i][0]);
	}
	printf("\n");
	scopeWin.scopes[2].updateData(entrainmentFreqData, 1);
}

//--------------------------------------------------------------
// Function to handle new entrainment data
void testApp::entrainmentFreqChange(float & freq){
	
}


//--------------------------------------------------------------
void testApp::update(){
	//printf("update(): %f\n", ofGetElapsedTimef());
	//printf("%s\n", ofGetTimestampString().c_str());
	
}

//--------------------------------------------------------------
void testApp::draw(){
	//LoggerData temp = LoggerData(ofGetElapsedTimef(), LoggerData::ENTRAINMENT_FREQ, freqOutThread.GetCurrentFreq());
	//LoggerData temp = LoggerData(ofGetElapsedTimef(), "EF", freqOutThread.GetCurrentFreq());
	//logger.addData(temp);
	//temp = LoggerData(ofGetElapsedTimef(), "EO", freqOutThread.getCurrentOutState());
	//logger.addData(temp);
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

	// Screen-based entrainment
	if (showScreenEntrainment) {
		if (freqOutThread.getCurrentOutState()) {
			ofBackground(0, 0, 0);
		} else {
			ofBackground(255, 255, 255);
		}
	}

	// Stimulus presentation
	if (showStimuli) {
		if (counter < 60*1) {
			stimuli[0].draw(100,100);
		} else if (counter < 60*2) {
			stimuli[1].draw(100,100);
		} else if (counter < 60*3) {
			stimuli[2].draw(100,100);
		} else if (counter > 60*4) {
			counter = 0;
		}
		counter ++;
	}

	// Draw oscilloscope data
	if (showOscilloscope) {
		scopeWin.plot();
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
	zeoThread.stopThread();
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
