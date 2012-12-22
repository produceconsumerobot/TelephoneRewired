//
//  zeo.h
//
//  Created by Sean Montgomery on 12/19/12.
//  Ported from http://www.ecstaticlyrics.com/secret/zeo_raw_data_parser.tar.bz2
//

#include "zeo.h"

ZeoParser::ZeoParser() {
	splice.number = -1;
	for (int i=0; i<7; i++) splice.bin[i] = -1;
	splice.sqi = -1;
	splice.time = -1;
	splice.impendance = -1;
	splice.signal = -1;
	splice.stage = -1;

	label[0] = " 2-4 ";
	label[1] = " 4-8 ";
	label[2] = " 8-13";
	label[3] = "13-18";
	label[4] = "18-21";
	label[5] = "11-14";
	label[6] = "30-50";

	stage[0] = "Undefined";
	stage[1] = "Awake";
	stage[2] = "R.E.M.";
	stage[3] = "Light";
	stage[4] = "Deep";
	//const char * label[] = {" 2-4 ", " 4-8 ", " 8-13", "13-18", "18-21", "11-14", "30-50"};
	//const char * stage[] = {"Undefined", "Awake", "R.E.M.", "Light", "Deep"};

	stopThread = false;
}

ZeoParser::~ZeoParser() {
	lock();
	stopThread = true;
	unlock();
	serial.close();
}

void ZeoParser::setupSerial(string device, int baud) {
	lock();
	serial.listDevices();
	serial.setup(device, baud); //open the serial device
	unlock();
}

void ZeoParser::threadedFunction() {
	while (isThreadRunning()) {
		lock();
		if (!stopThread) {

#define BUFFER_SIZE 4096
			static char buffer[BUFFER_SIZE];
			static int available = 0;

			//lock();
			int count = serial.readBytes((unsigned char *) buffer + available, BUFFER_SIZE - available);
			//unlock();

			if (count < 0) {
				fprintf(stderr, "Error reading data!\n");
				//exit();
			};
			if (count > 0) {
				available += count;

				//lock();
				int remaining = parse_outer_packet(buffer, available);
				//lock();

				memmove(buffer, buffer + available - remaining, remaining);
				available = remaining;
			};

		}
		unlock();
	}
}

void ZeoParser::process_splice() {
	int i;
	printf("Splice number %d:\n", splice.number);
	for (i = 0; i < 7; i++) {
		if (splice.bin[0] >= 0) printf("    %s: %d\n", label[i], splice.bin[i]);
	};
	if (splice.sqi >= 0) printf("    SQI: %d\n", splice.sqi);
	//printf("    Date: %s", asctime(gmtime((time_t*) &splice.time)));
	if (splice.impendance >= 0) printf("    Impendance: %d\n", splice.impendance);
	if (splice.signal >= 0) printf("    Signal: %s\n", splice.sqi ? "Good" : "Bad");
	if (splice.stage >= 0 && splice.stage <= 4) printf("    Sleep Stage: %s\n", stage[splice.stage]);
	if (splice.time > 0) printf("    RTC Time: %d\n", splice.time);
	// put callback to main function here?
	bool ready;
	ofNotifyEvent(dataReady, ready, this);
	//printf("impendance=%i\n", splice.impendance);
}

void ZeoParser::process_waveform(short *buffer) {
	int i;
	printf("We have waveform data: ");
	for (i = 0; i < 128; i++) {
		splice.rawData[i] = buffer[i];
		if (i<6) printf("%i, ", splice.rawData[i]);
	};
	printf("... Yay!\n");
}

void ZeoParser::parse_inner_packet(char *buffer, int count) {
	int type;
	int i;
	int event;
	int real;
	int imaginary;
	if (count < 1) return;


	type = DATA(0, unsigned char);
	buffer++; count--;

#define REQUIRES(X) if (count < X) { fprintf(stderr, "Inner packet type %u requires %d data bytes but only contains %d data bytes!\n", (unsigned char) buffer[-1], X, count); return; }

	if (type == 0x00) { // Events
		REQUIRES(4);
		event = DATA(0, unsigned int);
		if (event == 0x05) printf("Event: Start of night!\n");
		if (event == 0x07) printf("Event: Sleep onset!\n");
		if (event == 0x0E) printf("Event: Headband docked!\n");
		if (event == 0x0F) printf("Event: Headband undocked!\n");
		if (event == 0x10) printf("Event: Alarm off!\n");
		if (event == 0x11) printf("Event: Alarm snooze!\n");
		if (event == 0x13) printf("Event: Alarm play!\n");
		if (event == 0x15) printf("Event: End of night!\n");
		if (event == 0x24) printf("Event: New headband!\n");
	} else if (type == 0x02) { // Splice Boundary
		REQUIRES(4);
		if (DATA(0, unsigned int) == splice.number + 1) {
			process_splice();
		} else {
			if (splice.number >= 0) {
				int missing = DATA(0, unsigned int) - splice.number;
				if (missing > 0) {
					fprintf(stderr, "Discarding %d splices due to missing splice boundary packets!\n", missing);
				} else {
					fprintf(stderr, "Discarding unknown number of splices due to Zeo reboot!\n");
				};
			};
		};
		splice.number = DATA(0, unsigned int);
		for (i = 0; i < 7; i++) {
			splice.bin[i] = -1;
		};
		splice.sqi = -1;
		splice.time = -1;
		splice.impendance = -1;
		splice.signal = -1;
		splice.stage = -1;
	} else if (type == 0x03) { // Protocol Version
		REQUIRES(4);
		// If the version is different, does it matter?
		// Depends on how similar the newer versions are.
	} else if (type == 0x80) { // Waveform Data
		REQUIRES(256);
		process_waveform((short *) buffer);
	} else if (type == 0x83) { // Frequency Bins
		REQUIRES(14);
		for (i = 0; i < 7; i++) {
			splice.bin[i] = DATA(2 * i, unsigned short);
		};
	} else if (type == 0x84) { // SQI
		REQUIRES(4);
		splice.sqi = DATA(0, unsigned int);
	} else if (type == 0x8A) { // Time
		REQUIRES(4);
		splice.time = DATA(0, unsigned int);
	} else if (type == 0x97) { // Impendance
		REQUIRES(4);
		real = DATA(0, unsigned short) - 0x8000;
		imaginary = DATA(2, unsigned short) - 0x8000;
		splice.impendance = (int) sqrt((double)(real * real + imaginary * imaginary));
	} else if (type == 0x9C) { // Good / Bad Signal
		REQUIRES(4);
		splice.signal = DATA(0, unsigned int);
	} else if (type == 0x9D) { // Sleep Stage
		REQUIRES(4);
		splice.stage = DATA(0, unsigned int);
	} else {
		fprintf(stderr, "Unknown message:");
		for (i = 0; i < count; i++) {
			fprintf(stderr, " %02x", (unsigned char) buffer[i]);
		};
		fprintf(stderr, "\n");
	};

}

int ZeoParser::parse_outer_packet(char *buffer, int available) {

	while (available >= 11) {
		int data_length;
		int packet_size;
		int sum;
		int i;

		// First byte must be character 'A'

		if (DATA(0, unsigned char) != 'A') {
			buffer++; available--; continue;
		};

		// Check that message length matches inverted message length.

		if (DATA(3, unsigned short) != (0xFFFF & ~DATA(5, unsigned short))) {
			buffer++; available--; continue;
		};

		// Also discard unusually long messages...

		if (DATA(3, unsigned short) > 257) {
			fprintf(stderr, "Discarding unusually long inner packet (%d bytes).\n", DATA(3, unsigned short));
			buffer++; available--; continue;
		};

		data_length = DATA(3, unsigned short);
		packet_size = 11 + data_length;

		// Check that we have full message...

		if (available < packet_size) return available;

		// Verify checksum...

		sum = 0;
		for (i = 0; i < data_length; i++) {
			sum += DATA(11 + i, unsigned char);
		};
		sum %= 256;

		if (sum != DATA(2, unsigned char)) {
			fprintf(stderr, "Outer packet failed checksum, but I'm not sure I care...\n");
			buffer++; available--; continue;
		};

		//printf("Outer packet dump:");
		//for (int i = 0; i < packet_size; i++) {
		//  printf(" %02x", (unsigned char) buffer[i]);
		//};
		//printf("\n");

		parse_inner_packet(buffer + 11, data_length);

		buffer += packet_size;
		available -= packet_size;

	};

	return available;

}