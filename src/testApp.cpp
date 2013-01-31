#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {

	// **** COMPUTER SPECIFIC VARIABLES **** //

	// Arduino Port
	//string arduinoPort = "\\\\.\\COM28"; // Sean, Windows, Uno
	//string arduinoPort = "tty.usbmodemfa141"; // Sean, Mac, Arduino Decimila
	//string arduinoPort = "/dev/cu.usbserial-A70064Yu"; // Sean, Mac, Arduino Decimila
    //tty.usbmodemfa141
    //cu.usbmodemfa141
    string arduinoPort = "tty.usbmodem1411"; //Mac

	// Zeo Port
	//string zeoPort = "\\\\.\\COM26";
    string zeoPort = "tty.usbserial"; //Mac
    
    // Midi Port
    int midiPort = 0;
    
	// Log Directory
	logDirPath = "../../LogData/";

	// **** END COMPUTER SPECIFIC VARIABLES **** //

	logger.setDirPath(logDirPath);

	// **** OPTIONS **** //

	// Variables to control output functionality
	showStimuli = false;
	showOscilloscope = false;
	showScreenEntrainment = true;
	showLedEntrainment = false;
	playMidi = true;
	logData = false;
	readEEG = false;

	//Setup entrainment data listeners
	ofAddListener(freqOutThread.outputChanged, this, &testApp::entrainmentOutChange);
	ofAddListener(freqOutThread.freqChanged, this, &testApp::entrainmentFreqChange);

	// Set the brainwave entrainment frequencies cycle... see brainTrainment.h
	freqOutThread.setFreqCycle(nENTRAINMENT_DEBUGGING2_SCREEN, ENTRAINMENT_DEBUGGING2_SCREEN);
	//freqOutThread.setFreqCycle(nBRAIN_MACHINE, BRAIN_MACHINE);
	//freqOutThread.setFreqCycle(nBRAIN_MACHINE_FAST, BRAIN_MACHINE_FAST);
	
	freqOutThread.printFreqCycle();
	
	// Setup arduino LED output pins
	const int nEntrainmentLedPins = 3;
	const int entrainmentLedPins[nEntrainmentLedPins] = {9, 10, 11};
	const vector<int> vLedPins(entrainmentLedPins, entrainmentLedPins + nEntrainmentLedPins);
	//const int ledPWMs[nLedPins] = {255, 255, 255};
	//const vector<int> vLedPWMs(ledPWMs, ledPWMs+nLedPins);	

	// MIDI parameters
	midiChannel = 1;
	midiId = 60;
	midiValue = 100;
	// *** END OPTIONS *** //

	// Setup Arduino
	if (showLedEntrainment) {
		freqOutThread.setupLights(arduinoPort, 57600, vLedPins);
	}

	// Setup Midi
	midiMapMode = false;
	if (playMidi) {
		midiout.listPorts();
		midiout.openPort(midiPort);
		freqOutThread.setupMidi(&midiout, midiChannel, midiId, midiValue);
	}

	// Setup zeo and listeners
	if (readEEG) {
		zeoThread.setupSerial(zeoPort);
		ofAddListener(zeoThread.newRawData, this, &testApp::newZeoRawData);
		ofAddListener(zeoThread.newSliceData, this, &testApp::newZeoSliceData);
	}

	// Turn on/off zeo data printfs
	printData = false;

	// Startup screen display parameters
	ofBackground(0, 0, 0);

	// Sync with screen (~50Hz?)
	ofSetVerticalSync(true);

	if (showScreenEntrainment) {
		// Turn on screen flashing
		freqOutThread.turnOnScreenFlashing();
	}

	SetupOscilloscopes();
	isScopePaused = false;
	
	// **** Start threads **** //
	// DO THIS LAST OR YOU NEED TO LOCK() ON SETUP FUNCTIONS

	// Start outputs thread
	if (!showScreenEntrainment && // If screen entrainment, we'll manage timing in the main thread
		(showLedEntrainment || playMidi)) {
		freqOutThread.startThread(true, false);
	}

	// Start zeo thread
	if (readEEG) {
		zeoThread.startThread(true, false);
	}

	// Start the data logger thread
	if (logData) {
		logger.startThread(true, false);
	}
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
		scopeWin.scopes.at(0).setup(rawTimeWindow, ZeoParser::RAW_DATA_LEN, names, colors, nVariables, 7., 0.);
	}
	{ // Power Data Scope
		const int nVariables = ZeoParser::NUM_FREQS;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200), 
			ofColor(200,200,0), ofColor(200,0,200), ofColor(0,200,200), ofColor(100,100,100)};
		string names[nVariables];
		for (int i=0; i<nVariables; i++) {
			names[i] = ZeoParser::labels[i];
		}
		scopeWin.scopes.at(1).setup(powerTimeWindow, 1, names, colors, nVariables, 0.07, -350.);
	}
	{ // Entrainment Signal Scope
		const int nVariables = NUM_ENTRAINMENT_FREQS;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200), 
			ofColor(200,200,0), ofColor(100,100,100)};
		string names[nVariables] = {"DELTA", "THETA", "ALPHA", "BETA", "GAMMA"};
		scopeWin.scopes.at(2).setup(powerTimeWindow, 1, names, colors, nVariables, 500, -350.);
	}
	{ // Data Reliability Scope
		const int nVariables = 3;
		ofColor colors[nVariables] = {ofColor(200,0,0), ofColor(0,200,0), ofColor(0,0,200)};
		string names[nVariables] = {"Impedance", "SQI", "Signal"};
		scopeWin.scopes.at(3).setup(powerTimeWindow, 1, names, colors, nVariables, 0.5, -350.);
	}
	
	// Initialize Oscilloscope data arrays	
	zeoRawData.resize(1, vector<float>(ZeoParser::RAW_DATA_LEN, 0.));
	zeoFiltData.resize(1, vector<float>(ZeoParser::RAW_DATA_LEN, 0.));
	zeoPowerData.resize(ZeoParser::NUM_FREQS, vector<float>(1, 0.));
	zeoQualityData.resize(3, vector<float>(1, 0.));
	entrainmentFreqData.resize(NUM_ENTRAINMENT_FREQS, vector<float>(1, 0.));
}

//--------------------------------------------------------------
// Callback function to log changes in entrainment on/off state
void testApp::entrainmentOutChange(bool & output) {
	if (logData) {
		if (logger.isThreadRunning()) logger.lock();
		logger.push_back(ofGetElapsedTimef(), LoggerData::IS_ENTRAINMENT_ON, output);
		if (logger.isThreadRunning()) logger.unlock();
	}
}

//--------------------------------------------------------------
// Callback function to log changes in entrainment frequency
void testApp::entrainmentFreqChange(float & freq) {
	if (logData) {
		if (logger.isThreadRunning()) logger.lock();
		logger.push_back(ofGetElapsedTimef(), LoggerData::ENTRAINMENT_FREQ, freq);
		if (logger.isThreadRunning()) logger.unlock();
	}
}

//--------------------------------------------------------------
// Function to handle new zeo raw data
void testApp::newZeoRawData(bool & ready){
#ifdef DEBUG_PRINT
	printf("testApp::newZeoRawData\n"); 
#endif

	// Get Zeo filtered data
	zeoThread.lock();
	zeoFiltData.at(0) = zeoThread.getZeoParser().getFilteredData();
	zeoThread.unlock();

	if (printData) {
		printf("    Filt Data:");
		for (int i=0; i<6; i++) {
			cout.precision(5);
			cout << fixed << zeoFiltData.at(0).at(i) << ", " ;
		}
		printf("\n");
	}

	//updateScopes
	if (!isScopePaused) {
		scopeWin.scopes.at(0).updateData(zeoFiltData);
	}

	
	// Get Zeo raw data
	zeoThread.lock();
	zeoRawData.at(0) = zeoThread.getZeoParser().getRawData();
	zeoThread.unlock();

	if (printData) {
		printf("    Filt Data:");
		for (int i=0; i<6; i++) {
			cout.precision(5);
			cout << fixed << zeoRawData.at(0).at(i) << ", " ;
		}
		printf("\n");
	}

	// Log data
	if (logData) {
		logger.lock();
		logger.push_back(ofGetElapsedTimef(), LoggerData::RAW_DATA, zeoRawData.at(0));
		logger.unlock();
	}
}

//--------------------------------------------------------------
// Function to handle new zeo slice data
void testApp::newZeoSliceData(bool & ready){
#ifdef DEBUG_PRINT 
	printf("testApp::newZeoSliceData\n"); 
#endif

	// Get Zeo slice data
	zeoThread.lock();
	ZeoSlice zeoSlice = zeoThread.getZeoParser().getSlice();
	zeoThread.unlock();

	for (int i=0; i<ZeoParser::NUM_FREQS; i++) {
		zeoPowerData.at(i).at(0) = (float) zeoSlice.power.at(i);
		if (printData) {
			printf("    %s: %f, %i\n", ZeoParser::labels[i].c_str(), zeoPowerData.at(i).at(0), zeoSlice.power.at(i));
		}
	}

	//updateScopes
	if (!isScopePaused) {
		scopeWin.scopes.at(1).updateData(zeoPowerData);
	}

	// Update signal quality in scope
	zeoQualityData.at(0).at(0) = zeoSlice.impendance;
	zeoQualityData.at(1).at(0) = zeoSlice.sqi*40; // multiply for display
	zeoQualityData.at(2).at(0) = zeoSlice.signal*1500; // multiply for display
	if (!isScopePaused) {
		scopeWin.scopes.at(3).updateData(zeoQualityData);
	}

	if (logData) {
		logger.lock();
		logger.push_back(ofGetElapsedTimef(), LoggerData::SLICE_DATA, zeoSlice);
		logger.unlock();
	}

	if (showOscilloscope) {
		freqOutThread.lock();
		float freq = freqOutThread.getCurrentFreq();
		freqOutThread.unlock();
		// Plot the current entrainment frequency
		plotEntrainmentFreqData(freq);
	}
}

//--------------------------------------------------------------
// Function to plot new entrainment data
void testApp::plotEntrainmentFreqData(float freq) {
#ifdef DEBUG_PRINT 
	printf("testApp::plotEntrainmentFreqData\n"); 
#endif
	for (int i=0; i<NUM_ENTRAINMENT_FREQS; i++) {
		entrainmentFreqData.at(i).at(0) = 0.;
	}
	if ((int) freq == (int) DELTA)	entrainmentFreqData.at(0).at(0) = 1.;
	if ((int) freq == (int) THETA)	entrainmentFreqData.at(1).at(0) = 1.;
	if ((int) freq == (int) ALPHA)	entrainmentFreqData.at(2).at(0) = 1.;
	if ((int) freq == (int) BETA)	entrainmentFreqData.at(3).at(0) = 1.;
	if ((int) freq == (int) GAMMA)	entrainmentFreqData.at(4).at(0) = 1.;

	if (printData) {
		printf("Entrainment = %f: ", freq);
		for (int i=0; i<NUM_ENTRAINMENT_FREQS; i++) {
			printf("%f, ", entrainmentFreqData.at(i).at(0));
		}
		printf("\n");
	}
	if (!isScopePaused) {
		scopeWin.scopes.at(2).updateData(entrainmentFreqData);
	}
}

//--------------------------------------------------------------
void testApp::update(){
#ifdef DEBUG_PRINT 
	printf("update()\n");
#endif

}

//--------------------------------------------------------------
void testApp::draw(){
#ifdef DEBUG_PRINT 
	printf("draw()\n");
#endif
	/*
	if (logData) {
		freqOutThread.lock();
		bool outOn = freqOutThread.getCurrentOutState();
		float freq = freqOutThread.getCurrentFreq();
		freqOutThread.unlock();

		logger.lock();
		logger.push_back(ofGetElapsedTimef(), LoggerData::IS_ENTRAINMENT_ON, outOn);
		logger.push_back(ofGetElapsedTimef(), LoggerData::ENTRAINMENT_FREQ, freq);
		logger.unlock();
	}
	*/

	if (showScreenEntrainment) {
		//freqOutThread.lock(); 
		freqOutThread.update();
		//freqOutThread.unlock();
	} else {
		ofSleepMillis(1);
		//sleep(1);
	}

	if (showStimuli) {

	}

	// Draw oscilloscope data
	if (showOscilloscope) {
			scopeWin.plot();
	}
}

//--------------------------------------------------------------
void testApp::exit(){
#ifdef DEBUG_PRINT 
	printf("exit()\n");
#endif
    zeoThread.waitForThread(true);
    freqOutThread.waitForThread(true);
    logger.waitForThread(true);

	//freqOutThread.lock();
	//freqOutThread.unsetMidi();
	//freqOutThread.unlock();

	midiout.closePort();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if (((char) key) == '+') {
        midiMapMode = !midiMapMode;
		freqOutThread.lock();
		freqOutThread.toggleMidiOut();
		freqOutThread.unlock();
    }
	if (((char) key) == '1') {
        midiout.sendControlChange(midiChannel, midiId, midiValue);
        printf("1");
    } 
	if (((char) key) == '2') {
        midiout.sendControlChange(midiChannel, midiId+1, midiValue);
        printf("2");
	}
	if (((char) key) == '3') {
        midiout.sendNoteOn(midiChannel, midiId+1, midiValue);
        printf("3");
	}
	if (((char) key) == '4') {
        midiout.sendNoteOff(midiChannel, midiId+1, midiValue);
        printf("3");
	}
	if ( key == 'f') {
		ofToggleFullscreen();
	}
	if ( key == 'r') {
		//absMaxOutDelay = 0;
	}
	if ( key == 'p') {
		isScopePaused = !isScopePaused;
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
