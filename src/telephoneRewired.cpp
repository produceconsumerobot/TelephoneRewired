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
	_currentFreqStartTime = ofGetElapsedTimeMillis();
	_currentOutputStartTime = ofGetElapsedTimeMillis();
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

		unsigned long long startTime;
		
		if (debuggingRollover) { // If we're debugging rollover
			startTime = _testTimeRollOver(); // Used to test ElapseTime rollover
		} else {
			startTime = ofGetElapsedTimeMillis();  // Used to calculate loop lag
		}

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
				startTime = ofGetElapsedTimeMillis();
			}

			_currentOutputStartTime = startTime;
			_currentFreqStartTime = startTime;
		}

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
				<< (_output?"true":"false") << " - " << ofGetElapsedTimeMillis() << "\n";
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

unsigned long long FreqOutThread::_testTimeRollOver() {
	timeRolloverTest++;
	unsigned long long temp = timeRolloverTest - startRolloverTest;
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
				unsigned long long etm1 = ofGetElapsedTimeMillis();
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
				unsigned long long etm2 = ofGetElapsedTimeMillis();
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

	_font.loadFont("verdana.ttf", 12, true, true);
	_fontColor = ofColor(100,100,100);
	_stimulusRect = ofRectangle(ofGetWindowHeight()/2, ofGetWindowWidth()/2, 100, 100);
}

Stimulus::Stimulus(types type, string data) 
{
	_type = type;
	_data = data;

	_font.loadFont("verdana.ttf", 12, true, true);
	_fontColor = ofColor(100,100,100);
	_stimulusRect = ofRectangle(ofGetWindowHeight()/2, ofGetWindowWidth()/2, 100, 100);
}

void Stimulus::showStimulus() 
{
	switch (_type) {
	case Sound:
		// play sound
		cout << "playing sound \n";
		break;
	case Text:
		// show text

		ofSetColor(_fontColor);
		_font.drawString(_data, _stimulusRect.getX(), _stimulusRect.getX());

		cout << "showing text \n";
		break;
	default:
		break;
	}
}

StimulusPlayer::StimulusPlayer() {
	loadStimuli("/stimuli/");
	_nStimuliToShow = min(15, (int) _stimulusCycle.size());
	_stimulusIterator = _nStimuliToShow;
	_stimulusCycleOn = false;
	_fontColor = ofColor(100,100,100);
	_stimulusRect = ofRectangle(ofGetWindowHeight()/2, ofGetWindowWidth()/2, 100, 100);
}

StimulusPlayer::StimulusPlayer(string path) {
	//string path = "data/stimuli/";
	loadStimuli(path);
	_nStimuliToShow = min(15, (int) _stimulusCycle.size());
	_stimulusIterator = _nStimuliToShow;
	_stimulusCycleOn = false;
	_stimulusRect = ofRectangle(ofGetWindowHeight()/2, ofGetWindowWidth()/2, 100, 100);
}

void StimulusPlayer::loadStimuli(string path) 
{
	// Load Sounds
	string test = ofToDataPath(path + "sounds/");
	 ofDirectory dir(ofToDataPath(path + "sounds/"));
	 if (dir.exists()) {
		 dir.allowExt("mp3");
		 dir.listDir();
		 for (int i=0; i<dir.numFiles(); i++) {
			 _allStimuli.push_back(Stimulus(Stimulus::types::Sound, dir.getPath(i)));
		 }
	 }

	 // Load Text
	 string filePath = path + "text.txt";
	 ofFile file(ofToDataPath(filePath));
	 if (file.exists() && file.canRead()) {
		 ofBuffer buffer = ofBufferFromFile(filePath);
		 while (!buffer.isLastLine()) {
			 _allStimuli.push_back(Stimulus(Stimulus::types::Text, buffer.getNextLine()));
		 }
	 }

	 _stimulusCycle = _allStimuli;
}

void StimulusPlayer::setTimes(
	unsigned long stimulusOnTime, 
	unsigned long interStimulusBaseDelayTime, 
	unsigned long interStimulusRandDelayTime) 
{
	_stimulusOnTime = stimulusOnTime;
	_interStimulusBaseDelayTime = interStimulusBaseDelayTime;
	_interStimulusRandDelayTime = interStimulusRandDelayTime;
}

void StimulusPlayer::start() {
	_currentStimulusTimerStart = ofGetElapsedTimeMillis();
	_stimulusCycleOn = true;
	_stimulusIterator = 0;
	ofSeedRandom();
	_currentStimulusDelayTime = _interStimulusBaseDelayTime + ((unsigned long) ofRandom(0, _interStimulusRandDelayTime));
}

void StimulusPlayer::randomizeStimuli()
{
	_stimulusCycle = _allStimuli;
	ofRandomize(_stimulusCycle);
	_stimulusCycle.erase(_stimulusCycle.begin(), _stimulusCycle.begin() + _nStimuliToShow);
}

int StimulusPlayer::updateStimulus() {
	if (_stimulusCycleOn) {

		if (_stimulusIterator >= _nStimuliToShow) {
			return _nStimuliToShow - _stimulusIterator;
		}

		unsigned long long currentTime = ofGetElapsedTimeMillis();

		// If interstimulus delay exceeded
		if ((currentTime - _currentStimulusTimerStart) > _currentStimulusDelayTime) {

			// If stimulus ON interval exceeded
			if ((currentTime - _currentStimulusTimerStart) 
				> (_currentStimulusDelayTime + _stimulusOnTime)) {
					// iterate stimulus
					_stimulusIterator++;
					
					// Recalculate the stimulus ON time (with randomness)
					_currentStimulusDelayTime = _interStimulusBaseDelayTime + ((unsigned long) ofRandom(0, _interStimulusRandDelayTime));
					_currentStimulusTimerStart = ofGetElapsedTimeMillis();

					// If we reached the number of stimuli to show
					if (_stimulusIterator >= _nStimuliToShow) {
						// Turn the cycle off
						_stimulusCycleOn = false;
					}

			} else { // If interstimulus delay NOT exceeded
				// Show stimulus
				_stimulusCycle.at(_stimulusIterator).showStimulus();
			}
		}

		return _nStimuliToShow - _stimulusIterator;
	}
	return 0;
}

