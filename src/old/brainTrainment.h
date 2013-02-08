//
//  brainTrainment.h
//
//	Listing of possible brainwave entrainment cycles
//	Each cycle is an array[nFrequencies][2] = {
//	{freq1, duration1},
//	{freq2, duration2},
//	etc...
//	}
//	First number of each pair sets the frequency of entrainment
//	Second number of each pair sets the duration that frequency is delivered
//
//	Give each cycle a unique variable NAME and associated nNAME designating 
//	the number of frequencies in the cycle.
//
//
//  Created by Sean Montgomery on 12/18/12.
//
//  This work is licensed under the Creative Commons 
//  Attribution-ShareAlike 3.0 Unported License. 
//  To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/.
//

#include <vector>

#ifndef _ENTRAINMENT_CYCLES
#define _ENTRAINMENT_CYCLES

// defined frequencies 
#define NUM_ENTRAINMENT_FREQS 5
#define DELTA 2.31
#define THETA 6.1
#define ALPHA 10.1
#define BETA 15.1
#define GAMMA 25.1

// defined frequencies 
#define DELTA_SCREEN	2.28
#define THETA_SCREEN	5.1
#define ALPHA_SCREEN	8.4
#define BETA_SCREEN		12.6
#define GAMMA_SCREEN	25.1

//#define NUM_ENTRAINMENT_FREQS 5
//#define DELTA 2.219
//#define THETA 5.988
//#define ALPHA 11.099
//#define BETA 14.409
//#define GAMMA 30.000


// 5x sped up version of Mitch Altman's Brain Machine Sequence
static const int nBRAIN_MACHINE_FAST = 43;
static const float BRAIN_MACHINE_FAST[nBRAIN_MACHINE_FAST][2] = { // { frequency, duration(seconds) }
	{BETA, 12}, //1
	{ALPHA, 2}, //2
	{BETA, 4}, //3
	{ALPHA, 3}, //4
	{BETA, 3}, //5
	{ALPHA, 4}, //6
	{BETA, 2}, //7
	{ALPHA, 6}, //8
	{BETA, 1}, //9
	{ALPHA, 12}, //10
	{THETA, 2}, //11
	{ALPHA, 6}, //12
	{THETA, 4}, //13
	{ALPHA, 6}, //14
	{THETA, 6}, //15
	{ALPHA, 3},  //16
	{THETA, 12}, //17
	{ALPHA, 3}, //18
	{BETA, 0.2}, //19
	{ALPHA, 3}, //20
	{THETA, 12}, //21
	{DELTA, 1}, //22
	{THETA, 2}, //23
	{DELTA, 1}, //24
	{THETA, 2},//25
	{DELTA, 1}, //26
	{THETA, 6},//27
	{ALPHA, 3},//28
	{BETA, 0.2},//29
	{ALPHA, 3},//30
	{THETA, 6}, //31
	{ALPHA, 3},//32
	{BETA, 0.2},//33
	{ALPHA, 4},//34
	{BETA, 1},//35
	{ALPHA, 4}, //36
	{BETA, 3},//37
	{ALPHA, 3},//38
	{BETA, 4},//39
	{ALPHA, 2},//40
	{BETA, 5}, //41
	{ALPHA, 1},//42
	{BETA, 12},//43
};


// Mitch Altman's Brain Machine Sequence
static const int nBRAIN_MACHINE = 43;
static const float BRAIN_MACHINE[nBRAIN_MACHINE][2] = { // { frequency, duration(seconds) }
	{BETA,	60}, //1
	{ALPHA, 10}, //2
	{BETA,	20}, //3
	{ALPHA, 15}, //4
	{BETA,	15}, //5
	{ALPHA, 20}, //6
	{BETA,	10}, //7
	{ALPHA, 30}, //8
	{BETA,	5}, //9
	{ALPHA, 60}, //10
	{THETA, 10}, //11
	{ALPHA, 30}, //12
	{THETA, 20}, //13
	{ALPHA, 30}, //14
	{THETA, 30}, //15
	{ALPHA, 15},  //16
	{THETA, 60}, //17
	{ALPHA, 15}, //18
	{BETA,	1}, //19
	{ALPHA, 15}, //20
	{THETA, 60}, //21
	{DELTA, 1}, //22
	{THETA, 10}, //23
	{DELTA, 1}, //24
	{THETA, 10},//25
	{DELTA, 1}, //26
	{THETA, 30},//27
	{ALPHA, 15},//28
	{BETA,	1},//29
	{ALPHA, 15},//30
	{THETA, 30}, //31
	{ALPHA, 15},//32
	{BETA,	1},//33
	{ALPHA, 20},//34
	{BETA,	5},//35
	{ALPHA, 20}, //36
	{BETA,	15},//37
	{ALPHA, 15},//38
	{BETA,	20},//39
	{ALPHA, 10},//40
	{BETA,	25}, //41
	{ALPHA, 5},//42
	{BETA,	60},//43
};


// 2.5x sped up version of Mitch Altman's Brain Machine Sequence (sort of)
static const int nBRAIN_MACHINE_2P5X = 43;
static const float BRAIN_MACHINE_2P5X[nBRAIN_MACHINE_2P5X][2] = { // { frequency, duration(seconds) }
	{BETA,	24}, //1
	{ALPHA,	4}, //2
	{BETA,	8}, //3
	{ALPHA, 6}, //4
	{BETA,	6}, //5
	{ALPHA, 8}, //6
	{BETA,	4}, //7
	{ALPHA, 12}, //8
	{BETA,  2}, //9
	{ALPHA, 24}, //10
	{THETA, 4}, //11
	{ALPHA, 12}, //12
	{THETA, 8}, //13
	{ALPHA, 12}, //14
	{THETA, 12}, //15
	{ALPHA, 6},  //16
	{THETA, 24}, //17
	{ALPHA, 6}, //18
	{BETA,	0.4}, //19
	{ALPHA, 6}, //20
	{THETA, 24}, //21
	{DELTA, 1}, //22
	{THETA, 4}, //23
	{DELTA, 1}, //24
	{THETA, 4},//25
	{DELTA, 1}, //26
	{THETA, 12},//27
	{ALPHA, 6},//28
	{BETA,  0.4},//29
	{ALPHA, 6},//30
	{THETA, 12}, //31
	{ALPHA, 6},//32
	{BETA,	0.4},//33
	{ALPHA, 8},//34
	{BETA,	2},//35
	{ALPHA, 8}, //36
	{BETA,	6},//37
	{ALPHA, 6},//38
	{BETA,	8},//39
	{ALPHA, 4},//40
	{BETA,	10}, //41
	{ALPHA, 2},//42
	{BETA,	24},//43
};

// 2.5x sped up version of Mitch Altman's Brain Machine Sequence (sort of)
static const int nBRAIN_MACHINE_2P5X_SCREEN = 43;
static const float BRAIN_MACHINE_2P5X_SCREEN[nBRAIN_MACHINE_2P5X][2] = { // { frequency, duration(seconds) }
	{BETA_SCREEN,	24}, //1
	{ALPHA_SCREEN,	4}, //2
	{BETA_SCREEN,	8}, //3
	{ALPHA_SCREEN,	6}, //4
	{BETA_SCREEN,	6}, //5
	{ALPHA_SCREEN,	8}, //6
	{BETA_SCREEN,	4}, //7
	{ALPHA_SCREEN,	12}, //8
	{BETA_SCREEN,	2}, //9
	{ALPHA_SCREEN,	24}, //10
	{THETA_SCREEN,	4}, //11
	{ALPHA_SCREEN,	12}, //12
	{THETA_SCREEN,	8}, //13
	{ALPHA_SCREEN,	12}, //14
	{THETA_SCREEN,	12}, //15
	{ALPHA_SCREEN,	6},  //16
	{THETA_SCREEN,	24}, //17
	{ALPHA_SCREEN,	6}, //18
	{BETA_SCREEN,	0.4}, //19
	{ALPHA_SCREEN,	6}, //20
	{THETA_SCREEN,	24}, //21
	{DELTA_SCREEN,	1}, //22
	{THETA_SCREEN,	4}, //23
	{DELTA_SCREEN,	1}, //24
	{THETA_SCREEN,	4},//25
	{DELTA_SCREEN,	1}, //26
	{THETA_SCREEN,	12},//27
	{ALPHA_SCREEN,	6},//28
	{BETA_SCREEN,	0.4},//29
	{ALPHA_SCREEN,	6},//30
	{THETA_SCREEN,	12}, //31
	{ALPHA_SCREEN,	6},//32
	{BETA_SCREEN,	0.4},//33
	{ALPHA_SCREEN,	8},//34
	{BETA_SCREEN,	2},//35
	{ALPHA_SCREEN,	8}, //36
	{BETA_SCREEN,	6},//37
	{ALPHA_SCREEN,	6},//38
	{BETA_SCREEN,	8},//39
	{ALPHA_SCREEN,	4},//40
	{BETA_SCREEN,	10}, //41
	{ALPHA_SCREEN,	2},//42
	{BETA_SCREEN,	24},//43
};

// Simple cycle to debug timing
static const int nENTRAINMENT_DEBUGGING = 5;
static const float ENTRAINMENT_DEBUGGING[nENTRAINMENT_DEBUGGING][2] = { // { frequency, duration(seconds) }
	{0.5, 5}, //1
	{1, 5}, //2
	{2, 5}, //3
	{4, 5}, //4
	{8, 5}, //5
};

// Simple cycle to debug timing
static const int nENTRAINMENT_DEBUGGING2 = 5;
static const float ENTRAINMENT_DEBUGGING2[nENTRAINMENT_DEBUGGING][2] = { // { frequency, duration(seconds) }
	{GAMMA, 10}, //1
	{BETA, 10}, //2
	{ALPHA, 10}, //3
	{THETA, 10}, //4
	{DELTA, 10}, //5
};

// Simple cycle to debug timing
static const int nENTRAINMENT_DEBUGGING2_SCREEN = 5;
static const float ENTRAINMENT_DEBUGGING2_SCREEN[nENTRAINMENT_DEBUGGING2_SCREEN][2] = { // { frequency, duration(seconds) }
	{GAMMA_SCREEN, 10}, //1
	{BETA_SCREEN, 10}, //2
	{ALPHA_SCREEN, 10}, //3
	{THETA_SCREEN, 10}, //4
	{DELTA_SCREEN, 10}, //5
};

static std::vector<FreqOutThread::freqInterval> createFreqCycle(const int nFreqs, const float freqs[][2]) {
#ifdef DEBUG_PRINT 
	printf("setFreqCycle()\n");
#endif
	//lock();

	std::vector<FreqOutThread::freqInterval> freqCycle;

	freqCycle.resize(nFreqs);

	for (int i=0; i<nFreqs; i++) {
		freqCycle.at(i).freq = freqs[i][0];
		freqCycle.at(i).duration = freqs[i][1];
	}

	return freqCycle;
};



#endif