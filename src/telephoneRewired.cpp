//
//  telephoneRewired.h
//
//  Created by Sean Montgomery on 12/18/12.
//  http://produceconsumerobot.com/
//
//  This work is licensed under the Creative Commons 
//  Attribution-ShareAlike 3.0 Unported License. 
//  To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
//

#include "telephoneRewired.h"

/*-------------------------------------------------
* FreqOutThread
* Thread to handle frequency modulation of light and sound
*-------------------------------------------------*/

FreqOutThread::FreqOutThread() {
#ifdef DEBUG_PRINT 
	printf("FreqOutThread()\n");
#endif
	_output = false;
	_outputDelay = 0.0;
	_nFreqs = 0;
	_freqIterator = -1;
	_currentFreqStartTime = myGetElapsedTimeMillis();
	_currentOutputStartTime = myGetElapsedTimeMillis();
	_sendMidi = true;
	_printOut = OUTPUT_DELAYS;

	_midiChannel = 0;
	_midiId  = 0;
	_midiValue = 0;
	_midiout = NULL;

	_absMaxOutDelay = 0;
	_absAveOutDelay = 0;
	_aveDecay = 100;

	_bSetupArduino = false;
	_flashScreen = false;

	// Testing rollover of ElapsedTime
	debuggingRollover = false;
	if (debuggingRollover) {
		startRolloverTest = -500;
		timeRolloverTest = -2000;
		_currentOutputStartTime = _testTimeRollOver();
	}

	//const int nHighPins = 1;
	//const int highPins[nHighPins] = {12};
	//vector<int> _highPins(highPins, highPins + nHighPins);
}

// Destructor
FreqOutThread::~FreqOutThread() {
#ifdef DEBUG_PRINT 
	printf("~FreqOutThread()\n");
#endif

	//if (isThreadRunning()) lock();
	//lock();



	//unlock();

	// Stop the thread if it's still running
	//if (isThreadRunning()) {
		//stopThread();
	//	waitForThread(true); // Stops thread and waits for thread to be cleaned up
	//}

	_nFreqs = 0;

	// Turn all the outputs off
	turnOutputsOff();

	// Disconnect the arduino
	if (_bSetupArduino) 
		_arduino.disconnect();

	_bSetupArduino = false;
}

void FreqOutThread::setupMidi(ofxMidiOut * midiout, int midiChannel, int midiId, int midiValue) {
	//lock();

	_midiChannel = midiChannel;
	_midiId  = midiId;
	_midiValue = midiValue;

	_midiout = midiout;

	// Set up MIDI port
	//_midiout.listPorts();
	//_midiout.openPort(0);

	//unlock();
}


// Sends MIDI off signal and unsets midiout
void FreqOutThread::unsetMidi() {
	//lock();

	if (_sendMidi) {
		// Send midi
		if (_midiout != NULL) {
			_midiout->sendNoteOff(_midiChannel, _midiId, _midiValue );
		}
	}

	_midiChannel = 0;
	_midiId  = 0;
	_midiValue = 0;
	_midiout = NULL;

	//unlock();
}

// toggleMidiOut
// Turns on/off the Midi Ouput
void FreqOutThread::toggleMidiOut() {
	//lock(); // Lock the thread

	_sendMidi = !_sendMidi;

	//unlock(); // Unlock the thread
}

void FreqOutThread::setFreqCycle(std::vector< freqInterval > freqs) {
	//lock();

	_freqCycle = freqs;
	_nFreqs = _freqCycle.size();

	// Send a callback that frequency has changed
	float f = _getCurrentFreq();
	ofNotifyEvent(freqChanged, f, this);

	//unlock();
}

// setFreqCycle
// Sets the entrainment freq cycle
void FreqOutThread::setFreqCycle(const int nFreqs, const float freqs[][2]) {
#ifdef DEBUG_PRINT 
	printf("setFreqCycle()\n");
#endif
	//lock();

	_nFreqs = nFreqs;
	_freqCycle.resize(_nFreqs);

	for (int i=0; i<_nFreqs; i++) {
		_freqCycle.at(i).freq = freqs[i][0];
		_freqCycle.at(i).duration = freqs[i][1];
	}

	_freqIterator = 0;

	// Send a callback that frequency has changed
	float f = _getCurrentFreq();
	ofNotifyEvent(freqChanged, f, this);
	
	//unlock();
}

void FreqOutThread::resetFreqCycle() 
{
	_freqIterator = 0;
	_outputDelay = 0.0;
}

// printFreqCycle
// prints the current entrainment freq cycle
void FreqOutThread::printFreqCycle() {
#ifdef DEBUG_PRINT 
	printf("printFreqCycle()\n");
#endif

	//lock();

	if (_freqCycle.size() > 0) {
		int counter = 0;
		printf("Entrainment Freq Cycle:\n");
		for (auto it = _freqCycle.begin(); it != _freqCycle.end(); ++it) {
			counter ++;
			if (counter < 100) printf(" "); // align printing
			if (counter < 10) printf(" ");  // align printing
			printf("%i, ", counter);
			if (it->freq < 10) printf(" ");  // align printing			
			if (it->freq < 100) printf(" ");  // align printing
			printf("%f, ", it->freq);
			if (it->duration < 100) printf(" "); // align printing
			if (it->duration < 10) printf(" ");  // align printing
			printf("%f\n", it->duration);
		}
	} else {
		printf("No Freq Cycle set\n");
	}

	//unlock();
}

// setupLights
// Sets up the light outputs
void FreqOutThread::setupLights(string device, int baud, std::vector<int> ledPins, std::vector<int> ledPWMs) {
#ifdef DEBUG_PRINT 
	printf("setupLights()\n");
#endif

	//lock();

	_ledPins = ledPins;
	_ledPWMs = ledPWMs;

	// Set up Arduino
	//_serial.enumerateDevices();
	_arduino.connect(device, baud);
	_bSetupArduino	= false;	// flag so we setup arduino when its ready, you don't need to touch this :)
	while(!_arduino.isArduinoReady());
	setupArduino(_arduino.getMajorFirmwareVersion());

	//unlock();
}

// setupLights
// Sets up the light outputs
void FreqOutThread::setupLights(string device, int baud, std::vector<int> ledPins) {
#ifdef DEBUG_PRINT 
	printf("setupLights()\n");
#endif

	//lock();

	_ledPins = ledPins;
	//_ledPWMs = ledPWMs;

	// Set up Arduino
	//_serial.enumerateDevices();
	_arduino.connect(device, baud);
	_bSetupArduino	= false;	// flag so we setup arduino when its ready, you don't need to touch this :)
	while(!_arduino.isArduinoReady());
	setupArduino(_arduino.getMajorFirmwareVersion());
	
	//unlock();
}

// setupArduino
// Sets up the Arduino
void FreqOutThread::setupArduino(const int & version) {
#ifdef DEBUG_PRINT 
	printf("setupArduino\n");
#endif

	//lock();

	// remove listener because we don't need it anymore
	ofRemoveListener(_arduino.EInitialized, this, &FreqOutThread::setupArduino);

	// it is now safe to send commands to the Arduino
	_bSetupArduino = true;

	// print firmware name and version to the console
	cout << _arduino.getFirmwareName() << endl; 
	cout << "firmata v" << _arduino.getMajorFirmwareVersion() << "." << _arduino.getMinorFirmwareVersion() << endl;

	for (int i=0; i<_ledPins.size(); i++) {
		if (i < _ledPWMs.size()) {
			// Setup pin for PWM
			_arduino.sendDigitalPinMode(_ledPins.at(i), ARD_PWM);
			_arduino.update();
		} else {
			// Setup pin for digital out
			_arduino.sendDigitalPinMode(_ledPins.at(i), ARD_OUTPUT);
			_arduino.update();
		}
	}

	// Setup pins to be constantly high
	_arduino.sendDigitalPinMode(12, ARD_OUTPUT);
	_arduino.update();
	_arduino.sendDigital(12, ARD_HIGH);
	_arduino.update();

	// Setup pins to be constantly low
	_arduino.sendDigitalPinMode(8, ARD_OUTPUT);
	_arduino.update();
	_arduino.sendDigital(8, ARD_LOW);
	_arduino.update();

	//unlock();
}

// turnOnScreenFlashing
// Turns on screen flashing entrainment
void FreqOutThread::turnOnScreenFlashing() {
	_flashScreen = true;
}

// GetCurrentFreq
// Gets the current entrainment frequency
float FreqOutThread::_getCurrentFreq() {

	bool t1 = isCurrentThread();
	bool t2 = isMainThread();
	string t3 = getThreadName();
	int t4 = getThreadId();

	//bool u1 = getCurrentThread()->isCurrentThread();
	//bool u2 = getCurrentThread()->isMainThread();
	//string u3 = getCurrentThread()->getThreadName();
	//int u4 = getCurrentThread()->getThreadId();

	if ((_nFreqs > 0) && (_freqIterator >= 0) && (_freqIterator < _freqCycle.size())) {
		return _freqCycle.at(_freqIterator).freq;
	} else {
		return -1;
	}

}

// GetCurrentFreq
// Gets the current entrainment frequency
// Public function with blocking
float FreqOutThread::getCurrentFreq() {
	//lock();
	return _getCurrentFreq();
	//unlock();
}

// GetCurrentDuration
// Gets the duration of the current frequency
float FreqOutThread::_getCurrentDuration() {

	if ((_nFreqs > 0) && (_freqIterator >= 0) && (_freqIterator < _freqCycle.size())) {
		return _freqCycle.at(_freqIterator).duration;
	} else {
		return -1;
	}

}

// GetCurrentDuration
// Gets the duration of the current frequency
float FreqOutThread::getCurrentDuration() {
	//lock();

	return _getCurrentDuration();

	//unlock();
}

// getCurrentOutDelay
// Gets the current delay interval for entrainment
int FreqOutThread::getCurrentOutDelay() {
	//lock();

	return _outputDelay;

	//unlock();
}

// getCurrentOutState
// Gets the current boolean state of the output
bool FreqOutThread::getCurrentOutState() {
	//lock();

	return _output;

	//unlock();
}

void FreqOutThread::turnOutputsOn() {
	//lock();

	_output = !_output; // Flip the output

	if (_sendMidi) {
		// Send midi
		if (_midiout != NULL) {
			_midiout->sendNoteOn(_midiChannel, _midiId, _midiValue );
		}
	}				
	if (_bSetupArduino) {

		// Send arduino output
		for (int i=0; i<_ledPins.size(); i++) {
			if (i < _ledPWMs.size()) {
				_arduino.sendPwm(_ledPins.at(i), _ledPWMs.at(i));
				_arduino.update();
			} else {
				if (_printOut & LED_STATES) printf("LED[%i]=HIGH\n",i);
				_arduino.sendDigital(_ledPins.at(i), ARD_HIGH);
				_arduino.update();
			}
		}
	}
	if (_flashScreen) {
		ofBackground(255,255,255);
	}

	if (_nFreqs > 0) { // If we're not deconstructing
		//Send callback to notify output has changed
		bool b = true;
		ofNotifyEvent(outputChanged, b, this);
	}

	//unlock();
}

void FreqOutThread::turnOutputsOff() {

	//if (isThreadRunning()) lock();

	_output = !_output; // Flip the output

	if (_sendMidi) {
		// Send midi
		if (_midiout != NULL) {
			_midiout->sendNoteOff(_midiChannel, _midiId, _midiValue );
		}
	}
	if (_bSetupArduino) {

		// Send arduino output

		for (int i=0; i<_ledPins.size(); i++) {
			if (i < _ledPWMs.size()) {
				_arduino.sendPwm(_ledPins.at(i), 0);
				_arduino.update();
			} else {
				if (_printOut & LED_STATES) printf("LED[%i]=LOW\n",i);
				_arduino.sendDigital(_ledPins.at(i), ARD_LOW);
				_arduino.update();
			}
		}
	}
	if (_flashScreen) {
		ofBackground(0,0,0);
	}

	//Send callback to notify output has changed
	if (_nFreqs > 0) { // If we're not deconstructing
		bool b = false;
		ofNotifyEvent(outputChanged, b, this);
	}

	//unlock();
}


// update
// Updates the outputs, check the current frequency duration 
// and iterates through the frequency cycle
void FreqOutThread::update() {
#ifdef DEBUG_PRINT 
	printf("update()\n");
#endif

	if (_getCurrentDuration() > -1) {

		unsigned long startTime;
		
		if (debuggingRollover) { // If we're debugging rollover
			startTime = _testTimeRollOver(); // Used to test ElapseTime rollover
		} else {
			startTime = myGetElapsedTimeMillis();  // Used to calculate loop lag
		}

		/* THIS ISN'T NEEDED BECAUSE WE'RE CASTING myGetElapsedTimeMillis() to an unsigned long
		unsigned long diffTime = startTime - _currentOutputStartTime;
		float startTimeF = (float) startTime;
		float _currentOutputStartTimeF = (float) _currentOutputStartTime;
		// Check for rollover of ofGetElapsedTime
		if ((startTimeF - _currentOutputStartTimeF) < 0) {
			//debugOfGetElapsedTimef();

			if (debuggingRollover) {
				startRolloverTest = timeRolloverTest;
				startTime = _testTimeRollOver();
			} else {
				void ofResetElapsedTimeCounter();
				startTime = myGetElapsedTimeMillis();
			}

			_currentOutputStartTime = startTime;
			_currentFreqStartTime = startTime;
		}
		*/

		if ((startTime - _currentOutputStartTime) >= (_outputDelay * 1000)) {
			float outDelay = _outputDelay - ((startTime - _currentOutputStartTime) / 1000.);
			float absOutDelay = abs(outDelay);
			_absMaxOutDelay = max(absOutDelay, _absMaxOutDelay);
			_absAveOutDelay = (_absAveOutDelay*(_aveDecay-1.) + absOutDelay)/_aveDecay;
			if (_printOut & OUTPUT_DELAYS) cout << fixed << setprecision(4) << "absMax: " << _absMaxOutDelay 
				<< ", absAve: " << _absAveOutDelay << ", Delay: " << outDelay << ", Dur: " << _outputDelay 
				<< ", Freq: " << _getCurrentFreq() << "\n" ;
				//printf("absMax: %.3f, absAve: %.3f, Delay: %.3f\n", _absMaxOutDelay, _absAveOutDelay, outDelay);

			_currentOutputStartTime = startTime;

			if (_output) {
				turnOutputsOff();
			} else {
				turnOutputsOn();
			}

			// If we've gone over the current duration
			if ((startTime - _currentFreqStartTime) >= (_getCurrentDuration() * 1000)) {
				_freqIterator = ( _freqIterator + 1 ) % _nFreqs; // iterate to the next frequency and rollover if at the end
				_currentFreqStartTime = startTime; // Reset the timer

				// Send a callback that frequency has changed
				float f = _getCurrentFreq();
				ofNotifyEvent(freqChanged, f, this);
			}

			_outputDelay = (1. / _getCurrentFreq() / 2.); // set the output delay to 1/2 period

			//int loopTime = (int) (1000. * (ofGetElapsedTimef() - startTime));

			if (_printOut & LOOP_DELAYS) {
				cout << fixed << setprecision(4) << "out: Freq=" << _getCurrentFreq() << ", Delay=" << (unsigned long)(_outputDelay*1000) << ", " 
				<< (_output?"true":"false") << " - " << myGetElapsedTimeMillis() << "\n";
				//printf("out: Freq=%f, Delay=%f, %s - %f\n", _getCurrentFreq(), _outputDelay, _output?"true":"false", ofGetElapsedTimef());
			}
			if (_printOut & LOOP_DELAYS) {
				cout << fixed << setprecision(4) << "start=" << startTime << ", outputStart=" 
					<< _currentOutputStartTime << ", delay=" << (unsigned long)(_outputDelay*1000) << "\n";
				//printf("startTime=%f, outputStartTime=%f, _outputDelay=%f\n", startTime, _currentOutputStartTime, _outputDelay);
			}

		} else { 
			if (_printOut & LOOP_TIMES) {
				cout << fixed << setprecision(4) << "start=" << startTime << ", outStart=" 
					<< _currentOutputStartTime << ", delay=" << (unsigned long)(_outputDelay*1000) << "\n";
				//printf("startTime=%f, outputStartTime=%f, _outputDelay=%f\n", startTime, _currentOutputStartTime, _outputDelay);
			}
		}
	} else {
		if (_printOut & LOOP_TIMES) printf("Duration = -1\n");
	}

	// Flush the Arduino buffer
	_arduino.update();
}

void FreqOutThread::threadedFunction() {
	while (isThreadRunning()) {
		lock();
		update();
		unlock();

		sleep(1);
		//ofSleepMillis(10);
	}
}

unsigned long FreqOutThread::_testTimeRollOver() {
	timeRolloverTest++;
	unsigned long temp = timeRolloverTest - startRolloverTest;
	//cout << "s:" << startRolloverTest << ", c=" << timeRolloverTest << ", t=" << temp << "\n";
	return temp; 
}

void FreqOutThread::debugOfGetElapsedTimef() {
													 // 18446744027136
				// Debugging ofGetElapsedTimef() == 18446744069422579320 problem

				Poco::LocalDateTime now1;
				Poco::Timestamp::TimeVal pts1 = now1.timestamp().epochMicroseconds();
				cout << "pts1=" << pts1 << "\n";
				//Poco::Timestamp pts1 = now.timestamp();
				//typedef Int64 TimeVal;
				//pts1.epochMicroseconds();

				unsigned long long etu1 = ofGetElapsedTimeMicros();
				cout << "etu1=" << etu1 << "\n";
				unsigned long long etm1 = myGetElapsedTimeMillis();
				cout << "etm1=" << etm1 << "\n";
				unsigned long long st1 = ofGetSystemTime();
				cout << "st1=" << st1 << "\n";
				unsigned long long stu1 = ofGetSystemTimeMicros();
				cout << "stu1=" << stu1 << "\n";
				float etf1 = ofGetElapsedTimef();
				cout << "etf1=" << etf1 << "\n";
				int fn1 = ofGetFrameNum();
				cout << "fn1=" << fn1 << "\n";
				int h1 = ofGetHours();
				cout << "h1=" << h1 << "\n";
				int m1 = ofGetMinutes();
				cout << "m1=" << m1 << "\n";
				int mo1 = ofGetMonth();
				cout << "mo1=" << mo1 << "\n";
				int s1 = ofGetSeconds();
				cout << "s1=" << s1 << "\n";
				string ts1 = ofGetTimestampString();
				cout << "ts1=" << ts1 << "\n";
				unsigned int ut1 = ofGetUnixTime();
				cout << "ut1=" << ut1 << "\n";

				float junk = ofGetElapsedTimef();

				void ofResetElapsedTimeCounter();
				cout << "\n" << "ofResetElapsedTimeCounter()" << "\n";

				Poco::LocalDateTime now2;
				Poco::Timestamp::TimeVal pts2 = now2.timestamp().epochMicroseconds();
				cout << "pts2=" << pts2 << "\n";
				unsigned long long etu2 = ofGetElapsedTimeMicros();
				cout << "etu2=" << etu2 << "\n";
				unsigned long long etm2 = myGetElapsedTimeMillis();
				cout << "etm2=" << etm2 << "\n";
				unsigned long long st2 = ofGetSystemTime();
				cout << "st2=" << st2 << "\n";
				unsigned long long stu2 = ofGetSystemTimeMicros();
				cout << "stu2=" << stu2 << "\n";
				float etf2 = ofGetElapsedTimef();
				cout << "etf2=" << etf2 << "\n";
				int fn2 = ofGetFrameNum();
				cout << "fn2=" << fn2 << "\n";
				int h2 = ofGetHours();
				cout << "h2=" << h2 << "\n";
				int m2 = ofGetMinutes();
				cout << "m2=" << m2 << "\n";
				int mo2 = ofGetMonth();
				cout << "mo2=" << mo2 << "\n";
				int s2 = ofGetSeconds();
				cout << "s2=" << s2 << "\n";
				string ts2 = ofGetTimestampString();
				cout << "ts2=" << ts2 << "\n";
				unsigned int ut2 = ofGetUnixTime();
				cout << "ut2=" << ut2 << "\n";

				junk = 0;
}


/*-------------------------------------------------
* ZeoReaderThread
* Thread to read zeo data and parse into various locations
*-------------------------------------------------*/

ZeoReaderThread::ZeoReaderThread() {
	_zeoReady = false;
}

ZeoReaderThread::~ZeoReaderThread() {
	// Stop the thread if it's still running
	//if (isThreadRunning()) {
		//stopThread();
		//waitForThread(true); // Stops thread and waits for thread to be cleaned up
	//}
	_zeoReady = false;
	_serial.close();
}

void ZeoReaderThread::setupSerial(string serialPort) {
	printf("ZeoReaderThread::setupSerial(%s)\n", serialPort.c_str());
	_serial.listDevices();
	//printf("connecting to serial port: %s\n", serialPort.c_str());
	_serial.setup(serialPort, 38400);
	
	// TODO: add some logic to test serial port is ready
	_zeoReady = true;
}

ZeoParser ZeoReaderThread::getZeoParser() {
	return _zeo;
}

void ZeoReaderThread::threadedFunction() {
	const int BUFFER_SIZE = 4096;
	static char buffer[BUFFER_SIZE];
	static int available = 0;

	while (isThreadRunning()) {
		lock();

		bool spliceDataReady = false;
		bool rawDataReady = false;

		if (_zeoReady) {
			//printf("_serial.readBytes\n");
			int count = _serial.readBytes((unsigned char *) buffer + available, BUFFER_SIZE - available);
			//printf("serial.count=%i\n", count);

			if (count < 0) {
				//fprintf(stderr, "Error reading data!\n");
				//exit();
			};
			if (count > 0) {
                //printf("read %i bytes\n", count);
				available += count;

				int remaining = _zeo.parsePacket(buffer, available, &spliceDataReady, &rawDataReady);

				memmove(buffer, buffer + available - remaining, remaining);
				available = remaining;
			}
		}

		// Send Callbacks if we've got new data
		if (spliceDataReady) ofNotifyEvent(newSliceData, spliceDataReady, this);
		if (rawDataReady) ofNotifyEvent(newRawData, rawDataReady, this);

		unlock();
		sleep(1);
	}
}

Stimulus::Stimulus() 
{
	_type = None;
	_data = "";

	_setup();
}

Stimulus::Stimulus(types type, string data) 
{
	_type = type;
	_data = data;

	_setup();
}

void Stimulus::_setup() 
{
	_font.loadFont("verdana.ttf", 80, true, true);
	_fontColor = ofColor(0,220,0);
	_stimulusCenter = ofPoint(ofGetWindowWidth()/2, ofGetWindowHeight()/2);
	_isPlaying = false;
	//std::stringstream ss;
	cout << Stimulus::Sound;
	//cout << ss;
}

void Stimulus::playStimulus() 
{
	ofRectangle bounds;

	switch (_type) {
	case Sound:
		// play sound
		if (!_isPlaying) {
			//ofSoundPlayer _mySound;
			_mySound.loadSound(_data); //DOCUMENTATION PAGE EXAMPLE IS WRONG
			_mySound.setVolume(0.75f);
			_mySound.play();
		}

		//cout << "playing sound \n";
		break;
	case Text:
		// show text
		
		ofPushMatrix();
		ofPushStyle();
			bounds = _font.getStringBoundingBox(_data, 0, 0);
			ofTranslate(-bounds.width/2, bounds.height / 2, 0);
			ofSetColor(_fontColor);
			_font.drawString(_data, _stimulusCenter.x, _stimulusCenter.y);
		ofPopStyle();
		ofPopMatrix();

		//cout << "showing text \n";
		break;
	default:
		break;
	}

	_isPlaying = true;
}

void Stimulus::stopStimulus() 
{
	_isPlaying = false;
}

bool Stimulus::isPlaying()
{
	return _isPlaying;
}

string Stimulus::str()
{
	string s;
	switch(_type) 
	{
	case Stimulus::Sound:
		s = "Sound";
		break;
	case Stimulus::Text:
		s = "Text";
		break;
	default:
		s = "None";
	}
	return s + "," + _data;
}


StimulusPlayer::StimulusPlayer() {
	//loadStimuli("/stimuli/");
	//_nStimuliToShow = min(15, (int) _stimulusCycle.size());

	_setup();
}

StimulusPlayer::StimulusPlayer(string path) {
	//string path = "data/stimuli/";
	//loadStimuli(path);
	//_nStimuliToShow = min(15, (int) _stimulusCycle.size());

	_setup();
}

void StimulusPlayer::_setup() 
{
	ofSeedRandom();
	_nStimuliToShow = 0;
	_stimulusIterator = _nStimuliToShow;
	_stimulusCycleOn = false;

	_textFilePath = "";
	_soundDirPath = "";

	isQueuedStart = false;
	//_stimulusCenter = ofPoint(ofGetWindowWidth()/2, ofGetWindowHeight()/2);
}


int StimulusPlayer::loadStimuli(string textFilePath, string soundDirPath) 
{
	//cout << "StimulusPlayer::loadStimuli  " << myGetElapsedTimef() << "\n";

	// Load sounds
	std::vector<Stimulus> sounds;
	//string t = ofToDataPath(soundDirPath);
	ofDirectory dir1(ofToDataPath(soundDirPath)); // REPORT BUG
	ofDirectory dir(soundDirPath);
	bool b = dir.exists();
	string s = dir.getAbsolutePath();
	bool b1 = dir1.exists();
	string s1 = dir1.getAbsolutePath();
	//cout << "StimulusPlayer::loadStimuli dir " << myGetElapsedTimef() << "\n";
	if (dir.exists()) {
		//cout << "StimulusPlayer::loadStimuli dir.exists() " << myGetElapsedTimef() << "\n";
		dir.allowExt("mp3");
		//cout << "StimulusPlayer::loadStimuli dir.allowExt(); " << myGetElapsedTimef() << "\n";
		dir.listDir();
		//cout << "StimulusPlayer::loadStimuli dir.listDir(); " << myGetElapsedTimef() << "\n";
		int n = dir.numFiles();
		//cout << "StimulusPlayer::loadStimuli dir.numFiles(); " << myGetElapsedTimef() << "\n";
		for (int i=0; i<n; i++) {
			sounds.push_back(Stimulus(Stimulus::Sound, dir.getPath(i)));
			//cout << "StimulusPlayer::loadStimuli dir.getPath " << myGetElapsedTimef() << "\n";
		}
	}
	//cout << "StimulusPlayer::loadStimuli sounds loaded " << myGetElapsedTimef() << "\n";
	dir.close();

	// Load Text
	std::vector<Stimulus> text;
	ofFile file(ofToDataPath(textFilePath));
	if (file.exists() && file.canRead()) {
		ofBuffer buffer = file.readToBuffer();
		while (!buffer.isLastLine()) {
			text.push_back(Stimulus(Stimulus::Text, buffer.getNextLine()));
		}
	}
	//cout << "StimulusPlayer::loadStimuli text loaded " << myGetElapsedTimef() << "\n";
	file.close();

	_allStimuli.clear();
	//cout << "StimulusPlayer::loadStimuli allStimuli cleared " << myGetElapsedTimef() << "\n";
	// Create stimulus vector by interleaving sounds and text
	if (text.size() != sounds.size()) {
		fprintf(stderr, "Error StimulusPlayer::loadStimuli -- text.size() != sounds.size()");
		return -1;
	} else {
		for (int i=0; i<sounds.size(); i++) {
			_allStimuli.push_back(sounds.at(i));
			_allStimuli.push_back(text.at(i));
		}
	}
	//cout << "StimulusPlayer::loadStimuli allStimuli pushed " << myGetElapsedTimef() << "\n";

	_stimulusCycle = _allStimuli;
	_nStimuliToShow = _stimulusCycle.size();
	_stimulusIterator = _nStimuliToShow;

	//cout << "END StimulusPlayer::loadStimuli " << myGetElapsedTimef() << "\n";
	return 0;
}

void StimulusPlayer::setTimes(float stimulusOnTime, float interStimulusBaseDelayTime, float interStimulusRandDelayTime) 
{
	_stimulusOnTime = stimulusOnTime * 1000;
	_interStimulusBaseDelayTime = interStimulusBaseDelayTime * 1000;
	_interStimulusRandDelayTime = interStimulusRandDelayTime * 1000;
}

void StimulusPlayer::start() {
	_stimulusCycleOn = true;
	_stimulusIterator = 0;
	_currentStimulusDelayTime = _interStimulusBaseDelayTime + ((unsigned long) ofRandom(0, _interStimulusRandDelayTime));
	_currentStimulusTimerStart = myGetElapsedTimeMillis();
}

void StimulusPlayer::queueStart(string textFilePath, string soundDirPath) {
	isQueuedStart = true;
	setTextFilePath(textFilePath);
	setSoundDirPath(soundDirPath);
}

void StimulusPlayer::setTextFilePath(string textFilePath) 
{
	_textFilePath = textFilePath;
}

void StimulusPlayer::setSoundDirPath(string soundDirPath)
{
	_soundDirPath = soundDirPath;
}

void StimulusPlayer::randomizeStimuli()
{
	_stimulusCycle = _allStimuli;
	ofRandomize(_stimulusCycle);
	_stimulusCycle.erase(_stimulusCycle.begin(), _stimulusCycle.begin() + _nStimuliToShow);
}

int StimulusPlayer::update() {
	//cout << "StimulusPlayer::updateStimulus()\n";

	if (isQueuedStart) {

		isQueuedStart = false;

		// Load the appropriate stimuli
		loadStimuli(_textFilePath, _soundDirPath);
		//Start the Stimulus Player
		start();
		
	}

	if (_stimulusCycleOn) {

		if (_stimulusIterator >= _nStimuliToShow) {
			return _nStimuliToShow - _stimulusIterator;
		}

		unsigned long currentTime = myGetElapsedTimeMillis();

		// If interstimulus delay exceeded
		if ((currentTime - _currentStimulusTimerStart) > _currentStimulusDelayTime) {

			// If stimulus ON interval exceeded
			if ((currentTime - _currentStimulusTimerStart) 
				> (_currentStimulusDelayTime + _stimulusOnTime)) {

					// Stop current stimulus
					_stimulusCycle.at(_stimulusIterator).stopStimulus();
					// Callback to main code for logging
					Stimulus s = _stimulusCycle.at(_stimulusIterator);
					ofNotifyEvent(stimulusStop, s, this);

					// iterate stimulus
					_stimulusIterator++;
					
					// Recalculate the stimulus ON time (with randomness)
					_currentStimulusDelayTime = _interStimulusBaseDelayTime + ((unsigned long) ofRandom(0, _interStimulusRandDelayTime));
					_currentStimulusTimerStart = myGetElapsedTimeMillis();

					// If we reached the number of stimuli to show
					if (_stimulusIterator >= _nStimuliToShow) {
						// Turn the cycle off
						_stimulusCycleOn = false;
					}

			} else { // If interstimulus delay NOT exceeded

				// If we we're not currently showing a stimlus
				if (!_stimulusCycle.at(_stimulusIterator).isPlaying()) {
					// We're turning on a stimulus
					// Callback to main code for logging
					Stimulus s = _stimulusCycle.at(_stimulusIterator);
					ofNotifyEvent(stimulusPlay, s, this);
				}

				// Show stimulus
				_stimulusCycle.at(_stimulusIterator).playStimulus();
			}
		}

		return _nStimuliToShow - _stimulusIterator;
	}
	return 0;
}

/*
InstructionsPlayer::InstructionsPlayer() 
{
	_nPages = 3;
	_timeoutDelay = 3.;
}
*/

InstructionsPlayer::InstructionsPlayer(float timeout) 
{
	_nPages = 3;
	_currentPage = 0;
	_timeoutDelay = timeout;
	_lastButtonPressTime = myGetElapsedTimeMillis();
	setParticipantCode(777777); 
}

void InstructionsPlayer::update()
{
	if ((myGetElapsedTimeMillis() - _lastButtonPressTime) < ((unsigned long) (_timeoutDelay*1000))) {
		showPage(_currentPage);
	} else {
		goToPage(0);
	}
}

void InstructionsPlayer::buttonPressed()
{
	_lastButtonPressTime = myGetElapsedTimeMillis();
	if (_currentPage + 1 < _nPages) {
		goToPage(_currentPage + 1);
	} 
}

void InstructionsPlayer::goToPage(int i)
{
	if (i != _currentPage) {
		_currentPage = i;
		ofNotifyEvent(newPage, i, this);
	}
	if (_currentPage < 2) {
		setParticipantCode(777777);
	}
	showPage(_currentPage);
}

int InstructionsPlayer::remaining() 
{
	return _nPages - _currentPage -1;
}

void InstructionsPlayer::setParticipantCode(unsigned long participantCode) 
{
	_participantCode = participantCode;
}

void InstructionsPlayer::showPage(int i)
{
	switch (i) {
	case 0:
		{
			ofTrueTypeFont font;
			font.loadFont("verdana.ttf", 20, true, true);
			ofColor fontColor(0,220,0);
			ofPoint stimulusCenter(ofGetWindowWidth()/2, ofGetWindowHeight()/2);

			string data1 = "Please press the GREEN button to begin";
			//string data1 = "Please press the ";
			//string data2 = "GREEN button to begin";
			ofPushMatrix();
			ofPushStyle();
			ofRectangle bounds1 = font.getStringBoundingBox(data1, 0, 0);
			//ofRectangle bounds2 = font.getStringBoundingBox(data2, 0, 0);
			ofTranslate(-bounds1.width/2, bounds1.height / 2, 0);
			ofSetColor(fontColor);
			font.drawString(data1, stimulusCenter.x, stimulusCenter.y);
			ofPopStyle();
			ofPopMatrix();

			break;
		}
	case 1:
		{
			ofTrueTypeFont font;
			font.loadFont("verdana.ttf", 20, true, true);
			ofColor fontColor(0,220,0);
			ofPoint stimulusCenter(ofGetWindowWidth()/2, ofGetWindowHeight()/2);

			string data1 = "this is page 2";
			//string data1 = "Please press the ";
			//string data2 = "GREEN button to begin";
			ofPushMatrix();
			ofPushStyle();
			ofRectangle bounds1 = font.getStringBoundingBox(data1, 0, 0);
			//ofRectangle bounds2 = font.getStringBoundingBox(data2, 0, 0);
			ofTranslate(-bounds1.width/2, bounds1.height / 2, 0);
			ofSetColor(fontColor);
			font.drawString(data1, stimulusCenter.x, stimulusCenter.y);
			ofPopStyle();
			ofPopMatrix();
			break;
		}
	case 2:
		{		
			ofTrueTypeFont font;
			font.loadFont("verdana.ttf", 20, true, true);
			ofColor fontColor(0,220,0);
			ofPoint stimulusCenter(ofGetWindowWidth()/2, ofGetWindowHeight()/2);

			std::stringstream ss;
			ss << "this is page 3\n" << "USER ID: " << _participantCode;
			string data1 = ss.str();//"this is page 3";
			//string data1 = "Please press the ";
			//string data2 = "GREEN button to begin";
			ofPushMatrix();
			ofPushStyle();
			ofRectangle bounds1 = font.getStringBoundingBox(data1, 0, 0);
			//ofRectangle bounds2 = font.getStringBoundingBox(data2, 0, 0);
			ofTranslate(-bounds1.width/2, bounds1.height / 2, 0);
			ofSetColor(fontColor);
			font.drawString(data1, stimulusCenter.x, stimulusCenter.y);
			ofPopStyle();
			ofPopMatrix();
			break;
		}
	default:
		break;
	}
}


ExperimentGovernor::ExperimentGovernor() 
{
	_currentState = ExperimentGovernor::None;
	_participantNumber = 0;
	_congratulationsTime = 15;
	
	enabledStates.resize(ExperimentGovernor::None + 1); // only works if None is last!!
	enabledStates.at(ExperimentGovernor::Instructions) = false;
	enabledStates.at(ExperimentGovernor::BlankPage) = false;
	enabledStates.at(ExperimentGovernor::StimulusPresentation) = false;
	enabledStates.at(ExperimentGovernor::Congratulations) = false;
	enabledStates.at(ExperimentGovernor::None) = true;
}

/*
ExperimentGovernor::ExperimentGovernor(InstructionsPlayer ip, StimulusPlayer sp)
{
	_participantNumber = 0;

	setInstructionsPlayer(ip);
	setStimulusPlayer(sp);

	enabledStates.resize(ExperimentGovernor::None + 1); // only works if None is last!!
	enabledStates.at(ExperimentGovernor::None) = true;

	// Go to next state
	_currentState = ExperimentGovernor::Instructions;
}
*/

void ExperimentGovernor::setInstructionsPlayer(InstructionsPlayer * ip) 
{
	_instructionsPlayer = ip;
	enabledStates.at(ExperimentGovernor::Instructions) = true;
}

void ExperimentGovernor::setStimulusPlayer(StimulusPlayer * sp) 
{
	_stimulusPlayer = sp;
	enabledStates.at(ExperimentGovernor::StimulusPresentation) = true;
	enabledStates.at(ExperimentGovernor::BlankPage) = true;
	enabledStates.at(ExperimentGovernor::Congratulations) = true;
}

void ExperimentGovernor::goToState(states state)
{
	switch (state) {
	case ExperimentGovernor::Instructions:
		{
			_currentState = ExperimentGovernor::Instructions;
			string s = getStateString(_currentState);
			ofNotifyEvent(newState, s, this);
			_instructionsPlayer->goToPage(0);
		}
		break;

	case ExperimentGovernor::BlankPage:
		{
			_currentState = ExperimentGovernor::BlankPage;
			string s = getStateString(_currentState);
			ofNotifyEvent(newState, s, this);
			_timedPagePlayer.start(0.5);
		}
		break;

	case ExperimentGovernor::StimulusPresentation:
		{
			_currentState = ExperimentGovernor::StimulusPresentation;
			string s = getStateString(_currentState);
			ofNotifyEvent(newState, s, this);

			if (_participantNumber % 2) { 
				_stimulusPlayer->queueStart("data/stimuli/text/form4.txt", "stimuli/audio/form1/");
			} else {
				_stimulusPlayer->queueStart("data/stimuli/text/form1.txt", "stimuli/audio/form4/");
			}
			/*
			// Load the appropriate stimuli
			if (_participantNumber % 2) { 
			_stimulusPlayer->loadStimuli("data/stimuli/text/form4.txt", "stimuli/audio/form1/");
			} else {
			_stimulusPlayer->loadStimuli("data/stimuli/text/form1.txt", "stimuli/audio/form4/");
			}
			//Start the Stimulus Player
			_stimulusPlayer->start();
			*/
		}
		break;
	case ExperimentGovernor::Congratulations:
		{
			_currentState = ExperimentGovernor::Congratulations;
			string s = getStateString(_currentState);
			ofNotifyEvent(newState, s, this);
			_timedPagePlayer.start(_congratulationsTime, TimedPagePlayer::Congratulations);
		}
		break;
	case ExperimentGovernor::None:
		{
			_currentState = ExperimentGovernor::None;
			string s = "None";
			ofNotifyEvent(newState, s, this);
		}
		break;
	default:
		break;
	}
}

string ExperimentGovernor::getStateString(states state) 
{
	switch (state) {
	case Instructions:
		return "Instructions";
		break;
	case BlankPage:
		return "BlankPage";
		break;
	case StimulusPresentation:
		return "StimulusPresentation";
		break;
	case Congratulations:
		return "Congratulations";
		break;
	case None:
		return "None";
		break;
	default:
		return "";
		break;
	}
}

void ExperimentGovernor::nextState()
{
	switch (_currentState) {

	// Instructions
	case ExperimentGovernor::Instructions: 
		if (enabledStates.at(ExperimentGovernor::BlankPage)) {
			// Go to BlankPage
			goToState(ExperimentGovernor::BlankPage);

		} else if (enabledStates.at(ExperimentGovernor::StimulusPresentation)) {
			// Go to StimulusPresentation
			goToState(ExperimentGovernor::StimulusPresentation);

		} else if (enabledStates.at(ExperimentGovernor::Instructions) == true) {
			// Go to Instructions
			goToState(ExperimentGovernor::Instructions);

		} else {
			// Go to None
			goToState(ExperimentGovernor::None);
		}
		break;

	// BlankPage
	case ExperimentGovernor::BlankPage: 
		if (enabledStates.at(ExperimentGovernor::StimulusPresentation)) {
			// Go to StimulusPresentation
			goToState(ExperimentGovernor::StimulusPresentation);
		} else {
			// Go to None
			goToState(ExperimentGovernor::None);
		}
		break;

	// StimulusPresentation
	case ExperimentGovernor::StimulusPresentation:
		if (enabledStates.at(ExperimentGovernor::Congratulations) == true) {
			// Go to Instructions
			goToState(ExperimentGovernor::Congratulations);

		} else if (enabledStates.at(ExperimentGovernor::Instructions) == true) {
			// Go to Instructions
			goToState(ExperimentGovernor::Instructions);

		} else if (enabledStates.at(ExperimentGovernor::StimulusPresentation)) {
			// Nothing to do here

		} else {
			// Go to None
			goToState(ExperimentGovernor::None);

		}
		break;
	// Congratulations
	case ExperimentGovernor::Congratulations:
		if (enabledStates.at(ExperimentGovernor::Instructions) == true) {
			// Go to Instructions
			goToState(ExperimentGovernor::Instructions);

		} else {
			// Go to None
			goToState(ExperimentGovernor::None);

		}
		break;
	
	// None
	case ExperimentGovernor::None: 
		if (enabledStates.at(ExperimentGovernor::Instructions) == true) {
			// Go to Instructions
			goToState(ExperimentGovernor::Instructions);

		} else if (enabledStates.at(ExperimentGovernor::StimulusPresentation)) {
			// Go to StimulusPresentation
			goToState(ExperimentGovernor::StimulusPresentation);
		} else {
			// Nothing to do here
		}
		break;

	default:
		break;
	}
}

void ExperimentGovernor::update() {
	switch (_currentState) {
	case ExperimentGovernor::None:
		// Nothing to do here
		break;
	case ExperimentGovernor::Instructions:
		// Refresh the instructions
		_instructionsPlayer->update();
		break;
	case ExperimentGovernor::BlankPage:
		if (_timedPagePlayer.update()) {
			nextState();
		}
		break;	
	case ExperimentGovernor::StimulusPresentation:
		// Refresh the Stimulus
		if (_stimulusPlayer->update() <= 0) {
			// Go to next state
			nextState();
		}		
		break;
	case ExperimentGovernor::Congratulations:
		if (_timedPagePlayer.update()) {
			nextState();
		}
		break;	
	default:
		break;
	}
}

void ExperimentGovernor::buttonPressed() {
	switch (_currentState) {
	case ExperimentGovernor::None:
		// Go to next state
		nextState();
		break;
	case ExperimentGovernor::Instructions:
		if (_instructionsPlayer->remaining() == 1) { 
			// Notify callbacks for logging
			unsigned long p = ++_participantNumber;
			unsigned long id = generateParticipantID(_participantNumber);
			ofNotifyEvent(newParticipant, id, this);

			_instructionsPlayer->setParticipantCode(id);
			_timedPagePlayer.setParticipantCode(id);
			_instructionsPlayer->buttonPressed();

		} else if (_instructionsPlayer->remaining() > 0) { 
			// If there are remaining instructions, go for it
			_instructionsPlayer->buttonPressed();
		} else {
			// Go to next state;
			nextState();
		}
		break;
	case ExperimentGovernor::StimulusPresentation:
		// Nothing to do here
		break;
	default:
		// Nothing to do here
		break;
	}
}

unsigned long ExperimentGovernor::generateParticipantID(unsigned long participantNumber) 
{
	return (participantNumber ^ 313717);
}

unsigned long ExperimentGovernor::reverseParticipantID(unsigned long participantID) 
{
	return (participantID ^ 313717);
}


void ExperimentGovernor::setCongratulationsTime(float congratulationsTime) 
{
	_congratulationsTime = congratulationsTime;
}

TimedPagePlayer::TimedPagePlayer()
{
	_currentPage = TimedPagePlayer::BlankPage;
	_onDuration = 0.0;
	setParticipantCode(777777) ;
}

void TimedPagePlayer::setParticipantCode(unsigned long participantCode) 
{
	_participantCode = participantCode;
}

void TimedPagePlayer::start(float duration, pages page) 
{
	_currentPage = page;
	_onDuration = duration;
	_startTime = myGetElapsedTimeMillis();
}

bool TimedPagePlayer::update() 
{
	if ((myGetElapsedTimeMillis() - _startTime) < _onDuration*1000) {
		switch (_currentPage) {
		case TimedPagePlayer::Congratulations:
			{
				ofTrueTypeFont font;
				font.loadFont("verdana.ttf", 20, true, true);
				ofColor fontColor(0,220,0);
				ofPoint stimulusCenter(ofGetWindowWidth()/2, ofGetWindowHeight()/2);

				std::stringstream ss;
				ss << "CONGRATULATIONS!! YOUR ARE STOKED MAX!\n" << "USER ID: " << _participantCode;
				string data1 = ss.str();//"this is page 3";
				//string data1 = "Please press the ";
				//string data2 = "GREEN button to begin";
				ofPushMatrix();
				ofPushStyle();
				ofRectangle bounds1 = font.getStringBoundingBox(data1, 0, 0);
				//ofRectangle bounds2 = font.getStringBoundingBox(data2, 0, 0);
				ofTranslate(-bounds1.width/2, bounds1.height / 2, 0);
				ofSetColor(fontColor);
				font.drawString(data1, stimulusCenter.x, stimulusCenter.y);
				ofPopStyle();
				ofPopMatrix();
			}
			break;
		default:
			// blank page
			break;
		}
		return false; // isDone == false
	} else {
		return true; // isDone == true
	}
}