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

#ifndef _TELEPHONE_REWIRED
#define _TELEPHONE_REWIRED

//#include "brainTrainmentCycles.h"

#include <vector>
#include <algorithm> // includes max function

#include "ofMain.h"
#include "ofxMidi.h"

#include "zeoParser.h"

#include "Poco/LocalDateTime.h"
//#include "Poco/DateTimeFormatter.h"

/*-------------------------------------------------
* FreqOutThread
* Thread to handle frequency modulation of light and sound
*-------------------------------------------------*/

class FreqOutThread  : public ofThread {

	struct freqInterval {
		float freq;
		float duration;
	};

private:
	bool _output;
	float _outputDelay; // seconds

	int _nFreqs;
	int _freqIterator;
	std::vector<freqInterval> _freqCycle;

	unsigned long long _currentFreqStartTime; // milliseconds
	unsigned long long _currentOutputStartTime; // milliseconds

	std::vector<int> _ledPins;
	std::vector<int> _ledPWMs;
	//std::vector<int> _highPins;

	// Arduino variables
	ofSerial _serial;
	ofArduino _arduino;
	bool _bSetupArduino;

	// MIDI variables
	ofxMidiOut * _midiout;  
	bool _sendMidi;
	int _midiChannel;
	int _midiId;
	int _midiValue;

	// Screen background flashing
	bool _flashScreen;

	// Debugging variables
	static const char NONE = 0x00;
	static const char OUTPUT_DELAYS = 0x01;
	static const char LED_STATES = 0x02;
	static const char LOOP_DELAYS = 0x04;
	static const char LOOP_TIMES = 0x08;
	char _printOut;
	float _absMaxOutDelay; 
	float _absAveOutDelay; 
	float _aveDecay; 

	void turnOutputsOn();
	void turnOutputsOff();

	void debugOfGetElapsedTimef();

	float _getCurrentFreq();
	float _getCurrentDuration();
	
	// debugging rollover of ofElapsedTime
	bool debuggingRollover;
	unsigned long long _testTimeRollOver();
	unsigned long timeRolloverTest;
	unsigned long startRolloverTest;

public:
	FreqOutThread();
	~FreqOutThread();
	void setFreqCycle(std::vector< freqInterval > freqs);  // Sets the entrainment freq cycle
	void setFreqCycle(const int nFreqs, const float freqs[][2]); // Sets the entrainment freq cycle
	void printFreqCycle(); // Prints the entrainment freq cycle

	void setupMidi(ofxMidiOut * midiout, int midiChannel, int midiId, int midiValue);
	void unsetMidi();
	void setupLights(string device, int baud, std::vector<int> ledPin, std::vector<int> ledPWMs); // Setup light outputs
	void setupLights(string device, int baud, std::vector<int> ledPins);
	void setupArduino(const int & version); 
	void turnOnScreenFlashing();
	float getCurrentFreq();
	float getCurrentDuration();
	int getCurrentOutDelay();
	bool getCurrentOutState();

	ofEvent<bool> outputChanged;
	ofEvent<float> freqChanged;

	void update();
	void threadedFunction();

	void toggleMidiOut();
};



/*-------------------------------------------------
* ZeoReaderThread
* Thread to read zeo data and parse into various locations
*-------------------------------------------------*/

class ZeoReaderThread : public ofThread {

private:
	ofSerial	_serial;
	ZeoParser	_zeo;
	bool		_zeoReady;

public:
	ZeoReaderThread();
	~ZeoReaderThread();
	void setupSerial(string serialPort);
	ZeoParser getZeoParser();
	void threadedFunction();

	ofEvent<bool> newRawData;
	ofEvent<bool> newSliceData;
};


#endif