//
//  zeo.h
//
//  Created by Sean Montgomery on 12/19/12.
//  Ported from http://www.ecstaticlyrics.com/secret/zeo_raw_data_parser.tar.bz2
//

#ifndef _ZEO_PARSER
#define _ZEO_PARSER

#include "ofMain.h"
#include <stdio.h>
#include <math.h>
#include "ofEvents.h"

class ZeoParser : public ofThread {

#define DATA(OFFSET, TYPE) *((TYPE*) (buffer + OFFSET))

public:
	struct splice_data {
		// All values are -1 if they weren't sent.
		int number;
		int bin[7];
		int sqi;
		int time;
		int impendance;
		int signal;
		int stage;
		short rawData[128];
	};

	ZeoParser();
	~ZeoParser();
	void setupSerial(string device, int baud);
	void process_splice();
	void process_waveform(short *buffer);
	void parse_inner_packet(char *buffer, int count);
	int parse_outer_packet(char *buffer, int available); 
	void threadedFunction();

	ofEvent<bool> dataReady;


	//static string label = {" 2-4 ", " 4-8 ", " 8-13", "13-18", "18-21", "11-14", "30-50"};
private:
	splice_data splice;
	ofSerial	serial;

	char * label[7];
	char * stage[5];

	bool stopThread;
};

#endif