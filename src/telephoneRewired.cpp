//
//  telephoneRewired.h
//
//  Created by Sean Montgomery on 12/18/12.
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
	_output = false;
	_outputDelay = 0;
	_nFreqs = 0;
	_freqIterator = 0;
	_currentStartTime = 0.0;
	_sendMidi = true;
	_printOut = false;
}

FreqOutThread::~FreqOutThread() {
	delete[] _freqCycle;
	_arduino.disconnect();
	unlock();
}

void FreqOutThread::setupMidi(ofxMidiOut * midiout, int midiChannel, int midiId, int midiValue) {
	_midiChannel = midiChannel;
	_midiId  = midiId;
	_midiValue = midiValue;

	_midiout = midiout;

	// Set up MIDI port
	//_midiout.listPorts();
	//_midiout.openPort(0);
}

void FreqOutThread::setupLights(string device, int baud, int ledPin, int ledBrightness) {

	_ledPin = ledPin;
	_ledBrightness = ledBrightness;

	// Set up Arduino
	serial.enumerateDevices();
	//serial.listDevices();
	//serial.setup("\\\\.\\COM24", 57600);

	//_arduino.connect("/dev/tty.usbmodemfd121", 57600);
	_arduino.connect(device, baud);
	//_arduino.connect("COM24", 115200);
	// listen for EInitialized notification. this indicates that
	// the arduino is ready to receive commands and it is safe to
	// call setupArduino()
	//ofAddListener(_arduino.EInitialized, this, &FreqOutThread::setupArduino);
	_bSetupArduino	= false;	// flag so we setup arduino when its ready, you don't need to touch this :)
	while(!_arduino.isArduinoReady());
	setupArduino(_arduino.getMajorFirmwareVersion());
}

void FreqOutThread::setupArduino(const int & version) {
	if (_printOut) printf("setupArduino\n");

	// remove listener because we don't need it anymore
	ofRemoveListener(_arduino.EInitialized, this, &FreqOutThread::setupArduino);

	// it is now safe to send commands to the Arduino
	_bSetupArduino = true;

	// print firmware name and version to the console
	cout << _arduino.getFirmwareName() << endl; 
	cout << "firmata v" << _arduino.getMajorFirmwareVersion() << "." << _arduino.getMinorFirmwareVersion() << endl;
	// set pin D13 as digital output
	//_arduino.sendDigitalPinMode(13, ARD_OUTPUT);
	_arduino.sendDigitalPinMode(11, ARD_PWM);
}

void FreqOutThread::SetFreqCycle(int nFreqs, float freqs[][2]) {

	_nFreqs = nFreqs;

	_freqCycle = new freqInterval [_nFreqs];

	//_freqCycle = ( freqInterval * ) malloc( _nFreqs * sizeof( freqInterval ) );

	for (int i=0; i<_nFreqs; i++) {
		_freqCycle[i].freq = freqs[i][0];
		_freqCycle[i].duration = freqs[i][1];
	}

	_freqIterator = 0;
}

float FreqOutThread::GetCurrentFreq() {
	if ((_nFreqs > 0) && (_freqIterator < _nFreqs)) {
		return _freqCycle[_freqIterator].freq;
	} else {
		return -1;
	}
}

float FreqOutThread::GetCurrentDuration() {
	if ((_nFreqs > 0) && (_freqIterator < _nFreqs)) {
		return _freqCycle[_freqIterator].duration;
	} else {
		return -1;
	}
	return _freqCycle[_freqIterator].duration;
}

int FreqOutThread::getCurrentOutDelay() {
	//lock();
	return _outputDelay;
	//unlock();
}

bool FreqOutThread::getCurrentOutState() {
	//lock();
	return _output;
	//unlock();
}

void FreqOutThread::toggleMidiOut() {
	lock(); // Lock the thread
	_sendMidi = !_sendMidi;
	unlock(); // Unlock the thread
}

void FreqOutThread::threadedFunction() {
	_output = false;

	while (isThreadRunning()) {

		if (GetCurrentDuration() > -1) {

			lock(); // Lock the thread

			float startTime = ofGetElapsedTimef();  // Used to calculate loop lag

			//printf("start - %f\n", ofGetElapsedTimef());

			if (_output) {

				if (_sendMidi) {
					// Send midi
					_midiout->sendNoteOn(_midiChannel, _midiId, _midiValue );
				}
				if (_bSetupArduino) {

					// Send arduino output

					//_arduino.sendDigital(13, ARD_HIGH);
					_arduino.sendPwm(_ledPin, _ledBrightness);
				}
				ofBackground(255,255,255);
			} else {

				if (_sendMidi) {
					// Send midi
					_midiout->sendNoteOff(_midiChannel, _midiId, _midiValue);
				}
				if (_bSetupArduino) {

					// Send arduino output

					//_arduino.sendDigital(13, ARD_LOW);
					_arduino.sendPwm(_ledPin, 0);
				}
				ofBackground(0,0,0);
			}

			_arduino.update(); // Need to do this periodically or things get weird


			// If we've gone over the current duration
			if (ofGetElapsedTimef() - _currentStartTime > GetCurrentDuration()) {
				_freqIterator = ( _freqIterator + 1 ) % _nFreqs; // iterate to the next frequency and rollover if at the end
				_currentStartTime = ofGetElapsedTimef(); // Reset the timer
			}

			_outputDelay = (int) (1000. / GetCurrentFreq() / 2.); // set the output delay to 1/2 period


			//printf("end   - %f\n", ofGetElapsedTimef());
			int loopTime = (int) (1000. * (ofGetElapsedTimef() - startTime));

			if (_printOut) printf("out: Freq=%f, Delay=%i, loopTime=%i, %s - %f\n", GetCurrentFreq(), _outputDelay, loopTime, _output?"true":"false", ofGetElapsedTimef());

			_output = !_output; // Flip the output
			//printf("%i\n", loopTime);
			int adjustedDelay = _outputDelay - loopTime; // Adjust for loop lag

			unlock(); // Unlock the thread

			ofSleepMillis(adjustedDelay); // wait the set delay
		} else {
			if (_printOut) printf("Duration = -1\n");
		}



	}
	if (_printOut) printf("FreqOutThread Finished\n");

}
