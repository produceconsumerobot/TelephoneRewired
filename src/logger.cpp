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
// LoggerThread()
// -------------------------------------------------------
LoggerThread::LoggerThread() {
	_logDirPath = "../../LogData/";
	_fileName = fileDateTimeString(ofGetElapsedTimeMillis());
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

	while(!loggerQueue.empty()) {
		log(loggerQueue.front());
		loggerQueue.pop();
	}
}

void LoggerThread::setDirPath(string logDirPath) {
	_fileName = fileDateTimeString(ofGetElapsedTimeMillis());
}

string LoggerThread::fileDateTimeString(unsigned long long ofTime)
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
    output = output + ofToString(ofTime - (ofTime / 1000));
    
    return output;
}


void LoggerThread::log(string data) {
	ofDirectory dir(_logDirPath);
	dir.create(true);
	//_mkdir( _logDirPath.c_str() );//, S_IRWXU | S_IRWXG | S_IRWXO);

    string fileName = _logDirPath + _fileName;
    
    ofstream mFile;
    mFile.open(fileName.c_str(), ios::out | ios::app);
	//mFile.precision(3);
	//mFile << fixed << data.getTimeStamp() << ",";
	mFile << data;
	
    mFile.close();
}


void LoggerThread::threadedFunction() {
	while (isThreadRunning()) {
		lock();
		if (!loggerQueue.empty()) {
			log(loggerQueue.front());
			loggerQueue.pop();
		}
		unlock();

		sleep(10);
	}
}