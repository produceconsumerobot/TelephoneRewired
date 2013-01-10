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
	_arduino.sendDigitalPinMode(11, ARD_OUTPUT);
    _arduino.update();
	_arduino.sendDigitalPinMode(10, ARD_OUTPUT);
    _arduino.update();
	_arduino.sendDigitalPinMode(9, ARD_OUTPUT);
    _arduino.update();
    _arduino.sendDigitalPinMode(12, ARD_OUTPUT);
    _arduino.update();
    _arduino.sendDigitalPinMode(13, ARD_OUTPUT);
    _arduino.update();
    //_arduino.sendDigital(12, ARD_LOW);
    //_arduino.sendDigital(12, ARD_HIGH);
    //_arduino.update();
    //_arduino.sendDigital(13, ARD_LOW);
    //_arduino.sendDigital(13, ARD_HIGH);
    //_arduino.update();
	//_arduino.sendDigitalPinMode(11, ARD_PWM);
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

					_arduino.sendDigital(9, ARD_LOW);
                    _arduino.update();
                    _arduino.sendDigital(10, ARD_LOW);
                    _arduino.update();
                    _arduino.sendDigital(11, ARD_LOW);
                    _arduino.update();
                    _arduino.sendDigital(12, ARD_HIGH);
                    _arduino.update();
					//_arduino.sendPwm(_ledPin, _ledBrightness);
				}
				//ofBackground(255,255,255);
			} else {

				if (_sendMidi) {
					// Send midi
					_midiout->sendNoteOff(_midiChannel, _midiId, _midiValue);
				}
				if (_bSetupArduino) {

					// Send arduino output

					_arduino.sendDigital(9, ARD_HIGH);
                    _arduino.update();
                    _arduino.sendDigital(10, ARD_HIGH);
                    _arduino.update();
                    _arduino.sendDigital(11, ARD_HIGH);
                    _arduino.update();
                    _arduino.sendDigital(12, ARD_HIGH);
                    _arduino.update();
					//_arduino.sendPwm(_ledPin, 0);
				}
				//ofBackground(0,0,0);
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

ZeoReaderThread::ZeoReaderThread() {
	_zeoReady = false;
}

ZeoReaderThread::~ZeoReaderThread() {
	if (isThreadRunning()) lock();
	_zeoReady = false;
	if (isThreadRunning()) unlock();
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
				fprintf(stderr, "Error reading data!\n");
				//exit();
			};
			if (count > 0) {
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
		ofSleepMillis(1);
	}
}

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
/*
LoggerData::LoggerData(float ofTimestamp, string dataTypeTag, void * payload) {
	LoggerData(ofTimestamp, dataTypeTag);
	if (_dataTypeTag.compare(RAW_DATA)) {
		float * data = new float[ZeoParser::RAW_DATA_LEN];
		float * temp = (float *) payload;
		for (int i=0; i<ZeoParser::RAW_DATA_LEN; i++) {
			data[i] = temp[i];
		}
		_dataPayload = data;
	} else if (_dataTypeTag.compare(SLICE_DATA)){
		ZeoSlice * data = new ZeoSlice();
		ZeoSlice * temp = (ZeoSlice *) payload;
		temp->copyTo(data);
		_dataPayload = data;
	} else if (_dataTypeTag.compare(IS_ENTRAINMENT_ON)){
		bool * data = new bool;
		bool * temp = (bool *) payload;
		*data = temp;
		_dataPayload = data;
	} else if (_dataTypeTag.compare(ENTRAINMENT_FREQ)){
	} else {
		fprintf(stderr, "LoggerData::~LoggerData() dataTypeTag %s unknown\n", _dataTypeTag);
	}
}
*/
LoggerData::LoggerData(float ofTimestamp, string dataTypeTag, float rawData[ZeoParser::RAW_DATA_LEN]) {
	//LoggerData(ofTimestamp, dataTypeTag);
	_ofTimestamp = ofTimestamp;
	_dataTypeTag = dataTypeTag;
	float * data = new float[ZeoParser::RAW_DATA_LEN];
	for (int i=0; i<ZeoParser::RAW_DATA_LEN; i++) {
		data[i] = rawData[i];
	}
	_dataPayload = data;
}
LoggerData::LoggerData(float ofTimestamp, string dataTypeTag, ZeoSlice &zeoSlice) {
	//LoggerData(ofTimestamp, dataTypeTag);
	_ofTimestamp = ofTimestamp;
	_dataTypeTag = dataTypeTag;
	ZeoSlice * data = new ZeoSlice();
	zeoSlice.copyTo(data);
	_dataPayload = data;
}
LoggerData::LoggerData(float ofTimestamp, string dataTypeTag, bool boolIn) {
	//LoggerData(ofTimestamp, dataTypeTag);
	_ofTimestamp = ofTimestamp;
	_dataTypeTag = dataTypeTag;
	bool * data = new bool;
	*data = boolIn;
	_dataPayload = data;
}
LoggerData::LoggerData(float ofTimestamp, string dataTypeTag, float floatIn) {
	//LoggerData(ofTimestamp, dataTypeTag);
	_ofTimestamp = ofTimestamp;
	_dataTypeTag = dataTypeTag;
	float * data = new float;
	*data = floatIn;
	_dataPayload = data;
}
LoggerData::~LoggerData() {
	if (_dataTypeTag.compare(RAW_DATA) == 0) {
		//if (_dataPayload != NULL) delete[] (float*) _dataPayload;
	} else if (_dataTypeTag.compare(SLICE_DATA) == 0){
		//if (_dataPayload != NULL) delete (ZeoSlice*) _dataPayload;
		//if (_dataPayload != NULL) delete _dataPayload;
		ZeoSlice* temp = (ZeoSlice*) _dataPayload;
		if (_dataPayload != NULL) 
			//;
			delete temp;
	} else if (_dataTypeTag.compare(IS_ENTRAINMENT_ON) == 0){
		//if (_dataPayload != NULL) delete (bool*) _dataPayload;
	} else if (_dataTypeTag.compare(ENTRAINMENT_FREQ) == 0){
		float* temp = (float*) _dataPayload;
		float value = *temp;
		//if (_dataPayload != NULL) delete temp;
	} else if (_dataTypeTag.compare("-1") == 0){
	} else {
		fprintf(stderr, "LoggerData::~LoggerData() dataTypeTag %s unknown\n", _dataTypeTag);
	}

	_ofTimestamp = -1;
	_dataTypeTag = "-1";
	_dataPayload = NULL;
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
	_fileName = fileDateTimeString(ofGetElapsedTimef());
}
/* EmergenceLog::dateTimeString
 * Returns the current date/time in following format:
 * 2011.05.26,14.36.58,469
 */
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
void LoggerThread::addData(LoggerData data) {
	if (isThreadRunning()) lock();
	_loggerQueue.push(data);
	if (isThreadRunning()) unlock();
}


void LoggerThread::write(LoggerData data) {
	ofDirectory dir(_logDirPath);
	dir.create(true);
	//_mkdir( _logDirPath.c_str() );//, S_IRWXU | S_IRWXG | S_IRWXO);

    string fileName = _logDirPath + _fileName;
    
    ofstream mFile;
    mFile.open(fileName.c_str(), ios::out | ios::app);
	mFile.precision(3);
	mFile << fixed << data.getTimeStamp();
	mFile << ",";
	mFile << data.getTypeTag();
	mFile << ",";

	if (isThreadRunning()) lock();

	if (data.getTypeTag().compare(LoggerData::RAW_DATA) == 0) {
		// write raw data
		mFile.precision(3);
		float * temp = (float *) data.getPayload();
		for (int i=0; i<ZeoParser::RAW_DATA_LEN; i++) {
			mFile << fixed << temp[i] << ",";
		}
	} else if (data.getTypeTag().compare(LoggerData::SLICE_DATA) == 0){
		// write zeo slice data
		ZeoSlice * temp = (ZeoSlice *) data.getPayload();
		mFile << temp->number << ","; // packet number
		mFile << temp->time << ","; // zeo time
		for (int i=0; i< ZeoParser::NUM_FREQS; i++) {
			mFile << temp->power[i] << ","; // Power in different freq bands
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
		fprintf(stderr, "LoggerThread::write() dataTypeTag %s unknown\n", data.getTypeTag());
	}

	if (isThreadRunning()) unlock();

    mFile << "\n";
    mFile.close();
}

void LoggerThread::threadedFunction() {
	while (isThreadRunning()) {
		lock();

		if (!_loggerQueue.empty()) {
			write(_loggerQueue.front());
			_loggerQueue.pop();
		}
		unlock();
		ofSleepMillis(1);
	}
}