#pragma once

//#define DEBUG_PRINT

#include "ofMain.h"
#include "ofxMidi.h"

#include "telephoneRewired.h"
#include "brainTrainment.h"
#include "ofxOscilloscope.h"
#include "logger.h"
#include "myUtils.h"

#define STIMULUS_PLAY_CODE		"SP" //
#define STIMULUS_STOP_CODE 		"SS" //
#define ENTRAINMENT_OUT_CODE 	"EO"
#define ENTRAINMENT_FREQ_CODE 	"EF"
#define RAW_DATA_CODE			"RD" 
#define SPLICE_DATA_CODE		"SD"
#define BUTTON_DOWN_CODE		"BD"
#define BUTTON_UP_CODE			"BU"
#define EXPERIMENT_STATE_CODE	"ES"
#define PARTICIPANT_NUMBER_CODE	"PN"
#define PARTICIPANT_ID_CODE		"ID"
#define INSTRUCTIONS_PAGE_CODE	"IP" //

class testApp : public ofBaseApp{
	public:

		FreqOutThread freqOutThread;

		// MIDI variables
		int midiChannel;
		int midiId;
		int midiValue;
		bool midiMapMode;
		ofxMidiOut midiout;

		// Variables to control output functionality
		bool showInstructions;
		bool showStimuli;
		bool checkButtonPresses;
		bool showOscilloscope;
		bool showScreenEntrainment;
		bool showLedEntrainment;
		bool playMidi;
		bool logData;
		bool readEEG;

		// Input Arduino for button presses
		ofArduino inputArduino;
		bool isButtonPressed; // Track changes in button state

		//ZeoParser zeo;
		ZeoReaderThread zeoThread;
		bool printData; // prints zeo data to stdout

		// Data Logger
		LoggerThread logger;
		string logDirPath;


		// Player to show stimuli
		StimulusPlayer stimulusPlayer;
		// Player to show instructions
		InstructionsPlayer instructionsPlayer;
		// State machine to step through the experiment
		ExperimentGovernor experimentGovernor;

		//unsigned long participantNumber;

		int vLogFormat;

		float drawTime;

		// Oscilloscope
		ofxMultiScope scopeWin;
		bool isScopePaused;
		// Oscilloscope data arrays	
		std::vector<std::vector<float> >	zeoRawData;
		std::vector<std::vector<float> >	zeoFiltData;
		std::vector<std::vector<float> >	zeoPowerData;
		std::vector<std::vector<float> >	zeoQualityData;
		std::vector<std::vector<float> >	entrainmentFreqData;

		void SetupOscilloscopes();
		void newZeoSliceData(bool & ready);
		void newZeoRawData(bool & ready);
		void entrainmentOutChange(bool & output);
		void entrainmentFreqChange(float & freq);
		void plotEntrainmentFreqData(float freq);
		void stimulusPlay(Stimulus & stimulus);
		void stimulusStop(Stimulus & stimulus);
		void newExperimentState(string & state);
		void newParticipant(unsigned long & participantID);
		void newInstructionsPage(int & pageNumber);
		void buttonDown();
		void buttonUp();

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


