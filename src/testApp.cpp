#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
	myResetElapsedTimeCounter();

	// **** COMPUTER SPECIFIC VARIABLES **** //

	// Arduino for outputing on LEDs
	//string ledArduinoPort = "\\\\.\\COM4"; // Sean, Windows, Uno
	string ledArduinoPort = "\\\\.\\COM7"; // Sean, Windows, Uno
	//string ledArduinoPort = "tty.usbmodemfa141"; // Sean, Mac, Arduino Decimila
	//string ledArduinoPort = "/dev/cu.usbserial-A70064Yu"; // Sean, Mac, Arduino Decimila
    //tty.usbmodemfa141
    //cu.usbmodemfa141
    //string ledArduinoPort = "tty.usbmodem1411"; //Mac

	// Arduino for taking button press inputs
	//string inputArduinoPort = "\\\\.\\COM6";
	string inputArduinoPort = "\\\\.\\COM8";

	// Zeo Port
	string zeoPort = "\\\\.\\COM5";
    //string zeoPort = "tty.usbserial"; //Mac
    
    // Midi Port
    //int midiPort = 0;
	int midiPort = 1;
    
	// Log Directory
	logDirPath = "../../LogData/";

	// **** END COMPUTER SPECIFIC VARIABLES **** //

	vLogFormat = 1; // Log format specifier
	logger.setDirPath(logDirPath);

	// **** OPTIONS **** //

	// Variables to control output functionality
	checkButtonPresses = true; // requires Arduino
	showInstructions = true;
	showStimuli = true;

	showScreenEntrainment = false;
	showLedEntrainment = true; // requires Arduino
	playMidi = true;

	readEEG = true; // requires Zeo
	showOscilloscope = false; // sloooows down screen drawing
	logData = true;

	// Experiment Timing Variables
	float stimulusOnTime =				.1;		// Seconds
	float interStimulusBaseDelayTime =	0.3;	// Seconds
	float interStimulusRandDelayTime =	0.1;	// Seconds
	float instructionsTimeoutDelay =	3.;		// Seconds
	float congratulationsTime =			3.;	//Seconds

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

	// Set up input button press Arduino
	inputArduino.connect(inputArduinoPort, 57600);
	while(!inputArduino.isArduinoReady()); 
	inputArduino.sendAnalogPinReporting(0, ARD_ANALOG);
	inputArduino.update();
	isButtonPressed = false;

	// Setup LED output Arduino
	if (showLedEntrainment) {
		freqOutThread.setupLights(ledArduinoPort, 57600, vLedPins);
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

	// Setup experimentGovernor and Listeners
	experimentGovernor = ExperimentGovernor();
	ofAddListener(experimentGovernor.newState, this, &testApp::newExperimentState);
	ofAddListener(experimentGovernor.newParticipant, this, &testApp::newParticipant);
	experimentGovernor.setCongratulationsTime(congratulationsTime);

	// Setup Instruction Player
	if (showInstructions) {
		instructionsPlayer = InstructionsPlayer(instructionsTimeoutDelay);
		
		// TODO: Setup Listeners
		ofAddListener(instructionsPlayer.newPage, this, &testApp::newInstructionsPage);

		experimentGovernor.setInstructionsPlayer(&instructionsPlayer);
		experimentGovernor.nextState();
	}

	// Setup StimulusPlayer
	if (showStimuli) {
		//stimulusPlayer = StimulusPlayer("data/stimuli/");
		stimulusPlayer.loadStimuli("data/stimuli/form1.txt", "stimuli/sounds/form4/");
		stimulusPlayer.setTimes(stimulusOnTime, interStimulusBaseDelayTime, interStimulusRandDelayTime);

		// Setup Listeners
		ofAddListener(stimulusPlayer.stimulusPlay, this, &testApp::stimulusPlay);
		ofAddListener(stimulusPlayer.stimulusStop, this, &testApp::stimulusStop);
		//stimulusPlayer.randomizeStimuli();

		experimentGovernor.setStimulusPlayer(&stimulusPlayer);
	}
	
	// Startup screen display parameters
	ofBackground(0, 0, 0);

	// Sync with screen (~50Hz?)
	//ofSetVerticalSync(true);

	if (showScreenEntrainment) {
		// Turn on screen flashing
		freqOutThread.turnOnScreenFlashing();
	}

	SetupOscilloscopes();
	isScopePaused = false;


	//participantNumber = 0;
	
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

	drawTime = myGetElapsedTimeMillis();
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
// Callback function to log stimulus ON events
void testApp::stimulusPlay(Stimulus & stimulus) {
	if (logData) {
		if (logger.isThreadRunning()) logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << STIMULUS_PLAY_CODE << 
			"," << stimulus.str() << ",\n";
		logger.loggerQueue.push(ss.str());
		//logger.push_back(myGetElapsedTimeMillis(), LoggerData::STIMULUS_PLAY, stimulus.logPrint());
		if (logger.isThreadRunning()) logger.unlock();
	}
}

//--------------------------------------------------------------
// Callback function to log stimulus OFF events
void testApp::stimulusStop(Stimulus & stimulus) {
	if (logData) {
		if (logger.isThreadRunning()) logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << ","  << STIMULUS_STOP_CODE << 
			"," << stimulus.str() << ",\n";
		logger.loggerQueue.push(ss.str());
		//logger.push_back(myGetElapsedTimeMillis(), LoggerData::STIMULUS_STOP, stimulus.logPrint());
		if (logger.isThreadRunning()) logger.unlock();
	}
}

//--------------------------------------------------------------
// Callback function to log changes in entrainment on/off state
void testApp::entrainmentOutChange(bool & output) {
	if (logData) {
		if (logger.isThreadRunning()) logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << ","  << ENTRAINMENT_OUT_CODE << 
			"," << (output ? "1":"0") << ",\n";
		logger.loggerQueue.push(ss.str());
		//logger.push_back(myGetElapsedTimeMillis(), LoggerData::STIMULUS_STOP, output);
		if (logger.isThreadRunning()) logger.unlock();
	}
}

//--------------------------------------------------------------
// Callback function to log changes in entrainment frequency
void testApp::entrainmentFreqChange(float & freq) {
	if (logData) {
		if (logger.isThreadRunning()) logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << ","  << ENTRAINMENT_FREQ_CODE << 
			"," <<  fixed << setprecision(3) << freq  << ",\n";
		logger.loggerQueue.push(ss.str());
		//logger.push_back(myGetElapsedTimeMillis(), LoggerData::ENTRAINMENT_FREQ, freq);
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
		if (logger.isThreadRunning()) logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << ","  << RAW_DATA_CODE << 
			"," << strVectorF(zeoRawData.at(0)) << "\n";
		logger.loggerQueue.push(ss.str());
		//logger.push_back(myGetElapsedTimeMillis(), LoggerData::RAW_DATA, zeoRawData.at(0));
		if (logger.isThreadRunning()) logger.unlock();
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
		if (logger.isThreadRunning()) logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << SPLICE_DATA_CODE << 
			"," << zeoSlice.str() << ",\n";
		logger.loggerQueue.push(ss.str());
		//logger.push_back(myGetElapsedTimeMillis(), LoggerData::SLICE_DATA, zeoSlice);
		if (logger.isThreadRunning()) logger.unlock();
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
void testApp::buttonDown(){
#ifdef DEBUG_PRINT 
	printf("buttonDown()\n");
#endif
	if (!isButtonPressed) {
		if (logData) {
			logger.lock();
			std::stringstream ss;
			ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << BUTTON_DOWN_CODE <<
				"" << ",\n";
			logger.loggerQueue.push(ss.str());
			logger.unlock();
		}
		//if (showInstructions) {
		//	instructionsPlayer.buttonPressed();
		//}
		experimentGovernor.buttonPressed();
	}
	isButtonPressed = true;
}

//--------------------------------------------------------------
void testApp::buttonUp(){
#ifdef DEBUG_PRINT 
	printf("buttonUp()\n");
#endif
	if (isButtonPressed) {
		if (logData) {
			logger.lock();
			std::stringstream ss;
			ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << BUTTON_UP_CODE << 
				"" << ",\n";
			logger.loggerQueue.push(ss.str());
			logger.unlock();
		}
	}
	isButtonPressed = false;
}


//--------------------------------------------------------------
void testApp::newExperimentState(string & state){
#ifdef DEBUG_PRINT 
	printf("newExperimentState()\n");
#endif
	if (logData) {
		logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << EXPERIMENT_STATE_CODE << 
			"," << state << ",\n";
		logger.loggerQueue.push(ss.str());
		logger.unlock();
	}
	if (state == ExperimentGovernor::getStateString(ExperimentGovernor::StimulusPresentation)) {
		freqOutThread.lock();
		freqOutThread.resetFreqCycle();
		freqOutThread.unlock();
	}
}

//--------------------------------------------------------------
void testApp::newParticipant(unsigned long & participantID){
#ifdef DEBUG_PRINT 
	printf("newParticipant()\n");
#endif
	if (logData) {
		logger.lock();

		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << PARTICIPANT_ID_CODE << 
			"," << participantID << ",\n";
		logger.loggerQueue.push(ss.str());
		
		//experimentGovernor.reverseParticipantID(participantID);

		//unsigned long participantID = (participantNumber ^ 2999975935);
		//unsigned long participantID = (participantNumber ^ 313717);
		//unsigned long temp = (participantID ^ 313717);

		std::stringstream ss2;
		ss2 << myGetElapsedTimeMillis() << "," << vLogFormat << "," << PARTICIPANT_NUMBER_CODE << 
			"," << participantID << ",\n";
		logger.loggerQueue.push(ss2.str());

		logger.unlock();
	}
}

//--------------------------------------------------------------
void testApp::newInstructionsPage(int & pageNumber){
#ifdef DEBUG_PRINT 
	printf("newInstructionsPage()\n");
#endif
	if (logData) {
		logger.lock();
		std::stringstream ss;
		ss << myGetElapsedTimeMillis() << "," << vLogFormat << "," << INSTRUCTIONS_PAGE_CODE <<
			"," << pageNumber << ",\n";
		logger.loggerQueue.push(ss.str());
		logger.unlock();
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
		logger.push_back(myGetElapsedTimeMillis(), LoggerData::IS_ENTRAINMENT_ON, outOn);
		logger.push_back(myGetElapsedTimeMillis(), LoggerData::ENTRAINMENT_FREQ, freq);
		logger.unlock();
	}
	*/
	if (checkButtonPresses) {
		inputArduino.update();
		int input = inputArduino.getAnalog(0);
		if (input > 512) {
			buttonDown();
		} else {
			buttonUp();
		}
	}

	experimentGovernor.update();

	//if (showInstructions) {
	//	instructionsPlayer.update();
	//}
	//if (showStimuli) {
	//	if (stimulusPlayer.update() <= 0) {
	//		//cout << "stimulus list complete \n";
	//	}
	//}

	// Draw oscilloscope data
	if (showOscilloscope) {
		scopeWin.plot();
	}

	if (showScreenEntrainment) {
		//freqOutThread.lock(); 
		freqOutThread.update();
		//freqOutThread.unlock();
	} else {
		ofSleepMillis(1);
		//sleep(1);
	}

	cout << "time=" << myGetElapsedTimeMillis() << ", diff=" << myGetElapsedTimeMillis() - drawTime << "\n";
	drawTime = myGetElapsedTimeMillis();
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
	inputArduino.disconnect();
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
	if ( key == 's') {
		if (showStimuli) {
			//stimulusPlayer.randomizeStimuli();

			//logger.push_back(myGetElapsedTimeMillis(), LoggerData::PARTICIPANT_NUMBER, participantNumber);
			//logger.push_back(myGetElapsedTimeMillis(), LoggerData::PARTICIPANT_ID, participantID);
			//logger.unlock();
			//if (participantNumber % 2) { 
			//	stimulusPlayer.loadStimuli("data/stimuli/text/form4.txt", "stimuli/audio/form1/");
			//} else {
			//	stimulusPlayer.loadStimuli("data/stimuli/text/form1.txt", "stimuli/audio/form4/");
			//}
			//stimulusPlayer.start();
		}
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
