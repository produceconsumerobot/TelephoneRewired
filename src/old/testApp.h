#pragma once

#include "ofMain.h"
#include "ofxMidi.h"

class MyThread : public ofThread {
	void MyThread::threadedFunction() {
		while (isThreadRunning()) {
			//printf("2 - %s\n", ofGetTimestampString().c_str());
			ofSleepMillis(10);
		}
	}
};

class FreqOutThread : public ofThread {

	struct freqInterval {
		float freq;
		float duration;
	};

	private:
		bool _output;
		int _nFreqs;
		int _freqIterator;
		freqInterval * _freqCycle;
		//float currentFreq;
		//int currentDuration;
		float _currentStartTime;
		ofxMidiOut _midiout;  
		ofArduino _arduino;
		bool _bSetupArduino;
		int _ledPin;
		int _ledBrightness;
		bool _sendMidi;
		int _midiChannel;
		int _midiId;
		int _midiValue;
		ofSerial serial;

	public:
		FreqOutThread() {
			_output = false;
			_nFreqs = 0;
			_freqIterator = 0;
			_currentStartTime = 0.0;
			_sendMidi = true;
				
		}

		~FreqOutThread() {
			delete[] _freqCycle;
			_arduino.disconnect();
			_midiout.closePort();
			unlock();
		}

		void setupMidi(int port, int midiChannel, int midiId, int midiValue) {
			_midiChannel = midiChannel;
			_midiId  = midiId;
			_midiValue = midiValue;

			// Set up MIDI port
			_midiout.listPorts();
			_midiout.openPort(0);
		}

		void setupLights(string device, int baud = 57600, int ledPin = 11, int ledBrightness = 255) {

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

		void setupArduino(const int & version) {
			printf("setupArduino\n");

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

		void SetFreqCycle(int nFreqs, float freqs[][2]) {

			_nFreqs = nFreqs;

			_freqCycle = new freqInterval [_nFreqs];

			//_freqCycle = ( freqInterval * ) malloc( _nFreqs * sizeof( freqInterval ) );

			for (int i=0; i<_nFreqs; i++) {
				_freqCycle[i].freq = freqs[i][0];
				_freqCycle[i].duration = freqs[i][1];
			}

			_freqIterator = 0;
		}

		float GetCurrentFreq() {
			if ((_nFreqs > 0) && (_freqIterator < _nFreqs)) {
				return _freqCycle[_freqIterator].freq;
			} else {
				return -1;
			}
		}

		float GetCurrentDuration() {
			if ((_nFreqs > 0) && (_freqIterator < _nFreqs)) {
				return _freqCycle[_freqIterator].duration;
			} else {
				return -1;
			}
			return _freqCycle[_freqIterator].duration;
		}

		void toggleMidiOut() {
			lock(); // Lock the thread
			_sendMidi = !_sendMidi;
			unlock(); // Unlock the thread
		}

		void threadedFunction() {
			_output = false;

			while (isThreadRunning()) {

				if (GetCurrentDuration() > -1) {

					lock(); // Lock the thread

					float startTime = ofGetElapsedTimef();  // Used to calculate loop lag
					
					//printf("start - %f\n", ofGetElapsedTimef());

					if (_output) {

						if (_sendMidi) {
							// Send midi
							_midiout.sendNoteOn(_midiChannel, _midiId, _midiValue );
						}
						if (_bSetupArduino) {

							// Send arduino output

							//_arduino.sendDigital(13, ARD_HIGH);
							_arduino.sendPwm(_ledPin, _ledBrightness);
						}
					} else {

						if (_sendMidi) {
							// Send midi
							_midiout.sendNoteOff(_midiChannel, _midiId, _midiValue);
						}
						if (_bSetupArduino) {

							// Send arduino output

							//_arduino.sendDigital(13, ARD_LOW);
							_arduino.sendPwm(_ledPin, 0);
						}
					}
			
					_arduino.update(); // Need to do this periodically or things get weird
				
									
					// If we've gone over the current duration
					if (ofGetElapsedTimef() - _currentStartTime > GetCurrentDuration()) {
						_freqIterator = ( _freqIterator + 1 ) % _nFreqs; // iterate to the next frequency and rollover if at the end
						_currentStartTime = ofGetElapsedTimef(); // Reset the timer
					}

					int delay = (int) (1000. / GetCurrentFreq() / 2.); // set the output delay to 1/2 period
					
					printf("out: Freq=%f, Delay=%i, %s - %f\n", GetCurrentFreq(), delay, _output?"true":"false", ofGetElapsedTimef());

					_output = !_output; // Flip the output

										
					//printf("end   - %f\n", ofGetElapsedTimef());
					int loopTime = (int) (1000. * (ofGetElapsedTimef() - startTime));
					//printf("%i\n", loopTime);
					int adjustedDelay = delay - loopTime; // Adjust for loop lag
					
					unlock(); // Unlock the thread

					ofSleepMillis(adjustedDelay); // wait the set delay
				} else {
					printf("Duration = -1\n");
				}

				
			
			}
			printf("FreqOutThread Finished\n");

		}
	
};

class testApp : public ofBaseApp{
	public:

		FreqOutThread freqOutThread;
		MyThread thread;
		
		int midiChannel;
		int midiId;
		int midiValue;
		bool midiMapMode;
		ofxMidiOut midiout;

		void setup();
		void update();
		void draw();
		void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
};
