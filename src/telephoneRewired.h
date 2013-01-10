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
#include "zeoParser.h"
#include <queue>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define NUM_ENTRAINMENT_FREQS 5
#define DELTA 2.219
#define THETA 5.988
#define ALPHA 11.099
#define BETA 14.409
#define GAMMA 30.000


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

class LoggerData {
private:
	float _ofTimestamp;
	string _dataTypeTag;
	void * _dataPayload;
public:
	static const string RAW_DATA;
	static const string SLICE_DATA;
	static const string IS_ENTRAINMENT_ON;
	static const string ENTRAINMENT_FREQ;

	LoggerData();
	LoggerData(float ofTimestamp, string dataTypeTag);
	//LoggerData(float ofTimestamp, string dataTypeTag, void * data);
	LoggerData(float ofTimestamp, string dataTypeTag, float rawData[ZeoParser::RAW_DATA_LEN]);
	LoggerData(float ofTimestamp, string dataTypeTag, ZeoSlice &zeoSlice);
	LoggerData(float ofTimestamp, string dataTypeTag, bool boolIn);
	LoggerData(float ofTimestamp, string dataTypeTag, float floatIn);
	~LoggerData();
	float getTimeStamp();
	string getTypeTag();
	void * getPayload();
};



/*-------------------------------------------------
* LoggerThread
* Thread to read log data in the following format
* ofTimestamp, dataTypeTag, dataPayload
* Types of data: 
* zeoRawData 
*	data float[128]
* zeoSliceData
*	int packetNumber
*	zeoTimestamp int
*	powerData float[7]
*	SQI int
*	Impedance int
*	badSignal bool
*	int version
*	int stage
* entrainmentData
*	isOutputOn bool 
*	freqency float
* 
*-------------------------------------------------*/
class LoggerThread : public ofThread {
private:
	queue<LoggerData> _loggerQueue;
	string _logDirPath;
	string _fileName;
	void write(LoggerData data);
public:
	LoggerThread();
	LoggerThread(string logDirPath);
	string fileDateTimeString(float ofTime);
	void addData(LoggerData data);
	void threadedFunction();
};