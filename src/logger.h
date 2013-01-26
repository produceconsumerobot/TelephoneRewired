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

#ifndef _TELEPHONE_REWIRED_LOGGER
#define _TELEPHONE_REWIRED_LOGGER

#include <algorithm>

#include "ofMain.h"

#include "zeoParser.h"

class LoggerData {
private:
	float	_ofTimestamp;
	string	_dataTypeTag;
	void *	_dataPayload;
public:
	static const int	VERSION;
	static const string RAW_DATA;
	static const string SLICE_DATA;
	static const string IS_ENTRAINMENT_ON;
	static const string ENTRAINMENT_FREQ;

	LoggerData();
	LoggerData(float ofTimestamp, string dataTypeTag);
	LoggerData(float ofTimestamp, string dataTypeTag, void * payload);

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
	void log(LoggerData data);
public:
	LoggerThread();
	~LoggerThread();
	LoggerThread(string logDirPath);
	void setDirPath(string logDirPath);
	string fileDateTimeString(float ofTime);
	//void push(float ofTimestamp, string dataTypeTag, std::vector<float> payload);
	//void push_back(float ofTimestamp, string dataTypeTag, void * payload);
	//void push_back(LoggerData data);
	void push_back(float ofTimestamp, string dataTypeTag, std::vector<float> payload);
	void push_back(float ofTimestamp, string dataTypeTag, ZeoSlice payload);
	void push_back(float ofTimestamp, string dataTypeTag, float payload);
	void push_back(float ofTimestamp, string dataTypeTag, bool payload);
	void log_front();
	void pop_front();
	void threadedFunction();
	//template <class T> 
	//void push_back(float ofTimestamp, string dataTypeTag, T payload);
};

#endif


