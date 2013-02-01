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


#ifndef _MY_UTILS
#define _MY_UTILS

#include "ofMain.h"

static unsigned long myStartTime;

static unsigned long myGetSystemTime( ) {
	return (unsigned long) ofGetSystemTime();
}

static unsigned long myGetElapsedTimeMillis() {
	return myGetSystemTime() - myStartTime;
}

static float myGetElapsedTimef(){
	return myGetElapsedTimeMillis() / 1000.0f;
}

static void myResetElapsedTimeCounter(){
	myStartTime = myGetSystemTime();
}

#endif
