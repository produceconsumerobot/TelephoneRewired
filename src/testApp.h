#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "telephoneRewired.h"
#include "zeo.h"

class MyThread : public ofThread {
	void MyThread::threadedFunction() {
		while (isThreadRunning()) {
			//printf("2 - %s\n", ofGetTimestampString().c_str());
			ofSleepMillis(10);
		}
	}
};



class testApp : public ofBaseApp{
	public:

		FreqOutThread freqOutThread;
		MyThread thread;
		
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
		ZeoParser	zeo;

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
};
