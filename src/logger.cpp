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

#include "logger.h"


// ------------------------------------------------------- 
// LoggerData()
// -------------------------------------------------------
LoggerData::LoggerData() {
	_ofTimestamp = -1;
	_dataTypeTag = "-1";
	_dataPayload = NULL;
}
LoggerData::LoggerData(float ofTimestamp, string dataTypeTag) {
	_ofTimestamp = ofTimestamp;
	_dataTypeTag = dataTypeTag;
	_dataPayload = NULL;
}

LoggerData::LoggerData(float ofTimestamp, string dataTypeTag, void * payload) {
	_ofTimestamp = ofTimestamp;
	_dataTypeTag = dataTypeTag;
	_dataPayload = payload;
}
float LoggerData::getTimeStamp() {
	return _ofTimestamp;
}
string LoggerData::getTypeTag() {
	return _dataTypeTag;
}
void * LoggerData::getPayload() {
	return _dataPayload;
}

const int LoggerData::VERSION = 1;
const string LoggerData::RAW_DATA = "RD";
const string LoggerData::SLICE_DATA = "SD";
const string LoggerData::IS_ENTRAINMENT_ON = "EO";
const string LoggerData::ENTRAINMENT_FREQ = "EF";


// ------------------------------------------------------- 
// LoggerThread()
// -------------------------------------------------------
LoggerThread::LoggerThread() {
	_logDirPath = "../../LogData/";
	_fileName = fileDateTimeString(ofGetElapsedTimef());
}
LoggerThread::LoggerThread(string logDirPath) {
	_logDirPath = logDirPath;
	setDirPath(logDirPath);
}
	
LoggerThread::~LoggerThread() {
	// Stop the thread if it's still running
	//if (isThreadRunning()) {
		//stopThread();
	//	waitForThread(true); // Stops thread and waits for thread to be cleaned up
	//}

	while(!_loggerQueue.empty()) {
		log_front();
		pop_front();
	}
}

void LoggerThread::setDirPath(string logDirPath) {
	_fileName = fileDateTimeString(ofGetElapsedTimef());
}

string LoggerThread::fileDateTimeString(float ofTime)
{
    string output = "";
    
    int year = ofGetYear();
    int month = ofGetMonth();
    int day = ofGetDay();
    int hours = ofGetHours();
    int minutes = ofGetMinutes();
    int seconds = ofGetSeconds();
    
    output = output + ofToString(year) + ".";
    if (month < 10) output = output + "0";
    output = output + ofToString(month) + ".";
    if (day < 10) output = output + "0";
    output = output + ofToString(day) + ", ";
    if (hours < 10) output = output + "0";
    output = output + ofToString(hours) + ".";
    if (minutes < 10) output = output + "0";
    output = output + ofToString(minutes) + ".";
    if (seconds < 10) output = output + "0";
    output = output + ofToString(seconds) + ", ";
    output = output + ofToString(ofTime, 3);
    
    return output;
}


void LoggerThread::push_back(float ofTimestamp, string dataTypeTag, std::vector<float> payload) {
	void * dataPayload;

	if (dataTypeTag.compare(LoggerData::RAW_DATA) == 0) {
		if (payload.size() != ZeoParser::RAW_DATA_LEN) {
			fprintf(stderr, "ERROR mismatch between inputs size: %i found and %i expected for %s\n", payload.size(), ZeoParser::RAW_DATA_LEN, dataTypeTag.c_str());
			return;
		}
		std::vector<float> * data = new std::vector<float>(payload.size());
		*data = payload;
		dataPayload = data;
	} else {
		fprintf(stderr, "ERROR type mismatch between inputs std::vector<float> and %s\n", dataTypeTag.c_str());
		return;
	}

	_loggerQueue.push(LoggerData(ofTimestamp, dataTypeTag, dataPayload));
}

void LoggerThread::push_back(float ofTimestamp, string dataTypeTag, ZeoSlice payload) {
	void * dataPayload;

	if (dataTypeTag.compare(LoggerData::SLICE_DATA) == 0) {
		ZeoSlice * data = new ZeoSlice;
		*data = payload;
		dataPayload = data;
	} else {
		fprintf(stderr, "ERROR type mismatch between inputs ZeoSlice and %s\n", dataTypeTag.c_str());
		return;
	}

	_loggerQueue.push(LoggerData(ofTimestamp, dataTypeTag, dataPayload));
}

void LoggerThread::push_back(float ofTimestamp, string dataTypeTag, float payload) {
	void * dataPayload;

	if (dataTypeTag.compare(LoggerData::ENTRAINMENT_FREQ) == 0) {
		float * data = new float;
		*data = payload;
		dataPayload = data;
	} else {
		fprintf(stderr, "ERROR type mismatch between inputs float and %s\n", dataTypeTag.c_str());
		return;
	}

	_loggerQueue.push(LoggerData(ofTimestamp, dataTypeTag, dataPayload));
}

void LoggerThread::push_back(float ofTimestamp, string dataTypeTag, bool payload) {
	void * dataPayload;

	if (dataTypeTag.compare(LoggerData::IS_ENTRAINMENT_ON) == 0) {
		bool * data = new bool;
		*data = payload;
		dataPayload = data;
	} else {
		fprintf(stderr, "ERROR type mismatch between inputs bool and %s\n", dataTypeTag.c_str());
		return;
	}

	_loggerQueue.push(LoggerData(ofTimestamp, dataTypeTag, dataPayload));
}

void LoggerThread::pop_front() {
	if (!_loggerQueue.empty()) {
		LoggerData data = _loggerQueue.front();

		if (data.getTypeTag().compare(LoggerData::RAW_DATA) == 0) {
			std::vector<float> * temp = (std::vector<float> *) data.getPayload();
			if (temp != NULL) 
				delete temp;

		} else if (data.getTypeTag().compare(LoggerData::SLICE_DATA) == 0){
			ZeoSlice* temp = (ZeoSlice*) data.getPayload();
			if (temp != NULL) 
				delete temp;

		} else if (data.getTypeTag().compare(LoggerData::IS_ENTRAINMENT_ON) == 0){
			bool* temp = (bool*) data.getPayload();
			if (temp != NULL) 
				delete temp;

		} else if (data.getTypeTag().compare(LoggerData::ENTRAINMENT_FREQ) == 0){
			float* temp = (float*) data.getPayload();
			if (temp != NULL) 
				delete temp;

		} else if (data.getTypeTag().compare("-1") == 0){
		} else {
			fprintf(stderr, "LoggerData::~LoggerData() dataTypeTag %s unknown\n", data.getTypeTag().c_str());
			exit(-1);
		}
		_loggerQueue.pop();
	}
}

void LoggerThread::log(LoggerData data) {
	ofDirectory dir(_logDirPath);
	dir.create(true);
	//_mkdir( _logDirPath.c_str() );//, S_IRWXU | S_IRWXG | S_IRWXO);

    string fileName = _logDirPath + _fileName;
    
    ofstream mFile;
    mFile.open(fileName.c_str(), ios::out | ios::app);
	mFile.precision(3);
	mFile << fixed << data.getTimeStamp() << ",";
	mFile << fixed << LoggerData::VERSION << ",";
	mFile << data.getTypeTag() << ",";

	if (data.getTypeTag().compare(LoggerData::RAW_DATA) == 0) {
		// write raw data
		mFile.precision(3);
		for (int i=0; i<ZeoParser::RAW_DATA_LEN; i++) {
			mFile << fixed << ((std::vector<float> *) data.getPayload())->at(i) << ",";
		}

	} else if (data.getTypeTag().compare(LoggerData::SLICE_DATA) == 0){
		// write zeo slice data
		ZeoSlice * temp = (ZeoSlice *) data.getPayload();
		mFile << temp->number << ","; // packet number
		mFile << temp->time << ","; // zeo time
		for (int i=0; i< ZeoParser::NUM_FREQS; i++) {
			mFile << temp->power.at(i) << ","; // Power in different freq bands
		}
		mFile << temp->impendance << ","; // Impedance
		mFile << temp->sqi << ","; // signal quality index
		mFile << temp->signal << ","; // signal quality (0/1)
		mFile << temp->stage << ","; // sleep stage
		mFile << temp->version << ","; // zeo packet version

	} else if (data.getTypeTag().compare(LoggerData::IS_ENTRAINMENT_ON) == 0){
		// write entrainment "raw" data
		bool * temp = (bool *) data.getPayload();
		mFile << ((*temp) ? "1":"0") << ","; 

	} else if (data.getTypeTag().compare(LoggerData::ENTRAINMENT_FREQ) == 0){
		// write entrainment frequency
		mFile.precision(3);
		float * temp = (float *) data.getPayload();
		mFile << fixed << (*temp) << ",";

	} else {
		fprintf(stderr, "LoggerThread::write() dataTypeTag %s unknown\n", data.getTypeTag().c_str());
	}

    mFile << "\n";
    mFile.close();
}

void LoggerThread::log_front() {
	if (!_loggerQueue.empty()) {
		log(_loggerQueue.front());
	}
}

void LoggerThread::threadedFunction() {
	while (isThreadRunning()) {
		lock();
		log_front();
		pop_front();
		unlock();

		sleep(10);
	}
}