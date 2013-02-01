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

#ifndef _TELEPHONE_REWIRED
#define _TELEPHONE_REWIRED

//#include "brainTrainmentCycles.h"

#include <vector>
#include <algorithm> // includes max function

#include "ofMain.h"
#include "ofxMidi.h"

#include "myUtils.h"

#include "zeoParser.h"

#include "Poco/LocalDateTime.h"
//#include "Poco/DateTimeFormatter.h"

/*-------------------------------------------------
* FreqOutThread
* Thread to handle frequency modulation of light and sound
*-------------------------------------------------*/

class FreqOutThread  : public ofThread {

	struct freqInterval {
		float freq;
		float duration;
	};

private:
	bool _output;
	float _outputDelay; // seconds

	int _nFreqs;
	int _freqIterator;
	std::vector<freqInterval> _freqCycle;

	unsigned long _currentFreqStartTime; // milliseconds
	unsigned long _currentOutputStartTime; // milliseconds

	std::vector<int> _ledPins;
	std::vector<int> _ledPWMs;
	//std::vector<int> _highPins;

	// Arduino variables
	ofSerial _serial;
	ofArduino _arduino;
	bool _bSetupArduino;

	// MIDI variables
	ofxMidiOut * _midiout;  
	bool _sendMidi;
	int _midiChannel;
	int _midiId;
	int _midiValue;

	// Screen background flashing
	bool _flashScreen;

	// Debugging variables
	static const char NONE = 0x00;
	static const char OUTPUT_DELAYS = 0x01;
	static const char LED_STATES = 0x02;
	static const char LOOP_DELAYS = 0x04;
	static const char LOOP_TIMES = 0x08;
	char _printOut;
	float _absMaxOutDelay; 
	float _absAveOutDelay; 
	float _aveDecay; 

	void turnOutputsOn();
	void turnOutputsOff();

	void debugOfGetElapsedTimef();

	float _getCurrentFreq();
	float _getCurrentDuration();
	
	// debugging rollover of ofElapsedTime
	bool debuggingRollover;
	unsigned long _testTimeRollOver();
	unsigned long timeRolloverTest;
	unsigned long startRolloverTest;

public:
	FreqOutThread();
	~FreqOutThread();
	void setFreqCycle(std::vector< freqInterval > freqs);  // Sets the entrainment freq cycle
	void setFreqCycle(const int nFreqs, const float freqs[][2]); // Sets the entrainment freq cycle
	void printFreqCycle(); // Prints the entrainment freq cycle

	void setupMidi(ofxMidiOut * midiout, int midiChannel, int midiId, int midiValue);
	void unsetMidi();
	void setupLights(string device, int baud, std::vector<int> ledPin, std::vector<int> ledPWMs); // Setup light outputs
	void setupLights(string device, int baud, std::vector<int> ledPins);
	void setupArduino(const int & version); 
	void turnOnScreenFlashing();
	float getCurrentFreq();
	float getCurrentDuration();
	int getCurrentOutDelay();
	bool getCurrentOutState();

	ofEvent<bool> outputChanged;
	ofEvent<float> freqChanged;

	void update();
	void threadedFunction();

	void toggleMidiOut();
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

class Stimulus {
public:
	enum types {Text, Sound, None};
	Stimulus();
	Stimulus(types type, string data);
	void playStimulus();
	void stopStimulus();
	bool isPlaying();
	string str();

private:
	string _data;
	types _type;
	ofColor _fontColor;
	ofTrueTypeFont _font;
	ofPoint _stimulusCenter;
	bool _isPlaying;
	ofSoundPlayer _mySound;

	void _setup();
};

class StimulusPlayer {
private:
	std::vector<Stimulus> _allStimuli;
	std::vector<Stimulus> _stimulusCycle;
	//Stimulus _currentStimulus;
	bool _stimulusCycleOn;

	// Stimulus timing varaiables (in milliseconds)
	unsigned long _currentStimulusTimerStart;
	unsigned long _stimulusOnTime;
	unsigned long _currentStimulusDelayTime;
	unsigned long _interStimulusBaseDelayTime;
	unsigned long _interStimulusRandDelayTime;
	//unsigned long _initialStimulusDelay;

	int _stimulusIterator;
	int _nStimuliToShow;

	// Stimuli properties
	ofColor _fontColor;
	//ofPoint _stimulusCenter;

	void _setup();
	
public:
	/* 
	** StimulusPlayer()
	**		Sets random seed?
	**		Set stimulus iterator to number of stimuli to show
	*/
	StimulusPlayer();

	// StimulusPlayer(string path)
	//		Loads stimuli from given path
	StimulusPlayer(string path); 

	/* 
	** loadStimuli(string path)
	**		Loads stimuli from path
	*/
	int loadStimuli(string textFilePath, string soundDirPath);

	/*
	** void setTimes(unsigned long baseOnTime, unsigned long randOnTime, unsigned long interStimulusDelay)
	**		Sets stimulus timing (in milliseconds)
	*/
	void setTimes(float baseOnTime, float randOnTime, float interStimulusDelay);

	/* 
	** startStimulusCycle()
	**		Starts the stimulus cycle
	**	ALGORITHM:
	**		Sets timers
	**		Flips cycleOn bit
	**		Resets the stimulus iterator
	**		Recalculate the stimulus ON time (with randomness)
	*/
	void start();

	void randomizeStimuli(); // Randomizes the stimulus order

	/* 
	** updateStimulus()
	**	ALGORITHM:
	**		Checks current time against stimulus times 
	**		Shows the stimulus if it's time 
	**		Updates the stimulus iterator
	**	OUTPUT:
	**		int nRemaining	-- number of remaining stimuli to show
	*/
	int updateStimulus();

	//Stimulus getCurrentStimulus();

	ofEvent<Stimulus> stimulusPlay;
	ofEvent<Stimulus> stimulusStop;
};

class InstructionsPlayer 
{

private:
	//const int _nPages = 3;
	int _currentPage;

	//const float _timeoutDelay = 3.; // Seconds
	unsigned long _lastButtonPressTime;

public:
	InstructionsPlayer();

	/* 
	** update()
	**	ALGORITHM:
	**		shows the appropriate text for the current page
	**		checks for timeout since last button press.
	*/
	void update();

	/* 
	** buttonPressed()
	**	ALGORITHM:
	**		updates _lastButtonPressTime
	**		initiates page change
	*/
	void buttonPressed();

	void goToPage(int i);

	ofEvent<int> newPage;
};

// Directs traffic of Experiment, what to show, when
class ExperimentGovernor 
{

public:
	static enum states {Instructions, StimulusPresentation};

	ExperimentGovernor();

	void update(); // Controls instructions/stimulus presentation
	void buttonPressed(); // Input detected
	states getState();
	//void setInstructionsPlayer(InstructionsPlayer p);
	void StimulusPlayer(StimulusPlayer p);

	ofEvent<string> newState;

	//void includeInstructions(bool b);
	//void includeStimuli(bool b);
private:
	states _currentState;
	//InstructionsPlayer _instructionsPlayer;
	//StimulusPlayer _stimulusPlayer;
};

#endif