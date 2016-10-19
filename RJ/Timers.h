#pragma once

#ifndef __TimersH__
#define __TimersH__

#include <chrono>
#include "CompilerSettings.h"

class Timers
{
public:
	
	// High resolution clock timer types
	typedef std::chrono::high_resolution_clock			HRClock;
	typedef std::chrono::system_clock::time_point		HRClockTime;
	typedef std::chrono::system_clock::duration			HRClockDuration;

	// Return the current high-resolution clock time
	CMPINLINE static HRClockTime		GetHRClockTime(void) { return HRClock::now(); }

	// Returns the time elapsed between two high-resolution clock times
	CMPINLINE static double				GetMillisecondDuration(HRClockTime start, HRClockTime end) 
	{ 
		return std::chrono::duration<double, std::milli>(end - start).count(); 
	}

	*** UPDATE PROFILER TO USE THIS HR - CLOCK, THEN HAVE IT ALSO REPORT START / END TIMES AND USE TO IDENTIFY ANY 'MISSING' LONG PERIODS IN FRAME TIME ***
};




#endif



