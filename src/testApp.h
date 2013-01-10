#pragma once

#include "ofMain.h"
#include "ofxMidi.h"

#define DEBUG_PRINT

#include "telephoneRewired.h"
#include "zeoParser.h"
#include "ofxOscilloscope.h"

class testApp : public ofBaseApp{
	public:

		FreqOutThread freqOutThread;
		int counter;
		int midiChannel;
		int midiId;
		int midiValue;
		bool midiMapMode;
		ofxMidiOut midiout;
		bool outState;
		float prevLoopTime;
		int absMaxOutDelay;
		float absAveOutDelay;
		float prevFreq;
		ofImage stimuli[3];

		bool showStimuli;
		bool showOscilloscope;
		bool showScreenEntrainment;

		//ZeoParser zeo;
		ZeoReaderThread zeoThread;
		LoggerThread logger;
		
		
		ofxMultiScope scopeWin;

		// Oscilloscope data arrays	
		std::vector<std::vector<float>>  rawData;
		std::vector<std::vector<float>>  powerData;
		std::vector<std::vector<float>>  filtData;
		std::vector<std::vector<float>>	 sliceData;
		std::vector<std::vector<float>>	 entrainmentFreqData;

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

		void SetupOscilloscopes();
		void newZeoSliceData(bool & ready);
		void newZeoRawData(bool & ready);
		void entrainmentOutChange(bool & output);
		void entrainmentFreqChange(float & freq);
		void plotEntrainmentFreqData(float freq);
};
