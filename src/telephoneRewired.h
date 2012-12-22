//
//  telephoneRewired.h
//
//  Created by Sean Montgomery on 12/18/12.
//
//  This work is licensed under the Creative Commons 
//  Attribution-ShareAlike 3.0 Unported License. 
//  To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
//

#ifndef _TELEPHONE_REWIRED
#define _TELEPHONE_REWIRED

#include "ofMain.h"
#include "ofxMidi.h"

#define DELTA 2.219
#define THETA 5.988
#define ALPHA 11.099
#define BETA 14.409


/*-------------------------------------------------
* FreqOutThread
* Thread to handle frequency modulation of light and sound
*-------------------------------------------------*/
class FreqOutThread : public ofThread {

	struct freqInterval {
		float freq;
		float duration;
	};

private:
	bool _output;
	int _outputDelay;
	int _nFreqs;
	int _freqIterator;
	freqInterval * _freqCycle;
	//float currentFreq;
	//int currentDuration;
	float _currentStartTime;
	ofxMidiOut * _midiout;  
	ofArduino _arduino;
	bool _bSetupArduino;
	int _ledPin;
	int _ledBrightness;
	bool _sendMidi;
	int _midiChannel;
	int _midiId;
	int _midiValue;
	ofSerial serial;
	bool _printOut;

public:
	FreqOutThread();
	~FreqOutThread();
	void setupMidi(ofxMidiOut * midiout, int midiChannel, int midiId, int midiValue);
	void setupLights(string device, int baud = 57600, int ledPin = 11, int ledBrightness = 255);
	void setupArduino(const int & version);
	void SetFreqCycle(int nFreqs, float freqs[][2]);
	float GetCurrentFreq();
	float GetCurrentDuration();
	int getCurrentOutDelay();
	bool getCurrentOutState();
	void toggleMidiOut();
	void threadedFunction();
};

#endif
