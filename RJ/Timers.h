#pragma once

#ifndef __TimersH__
#define __TimersH__

#include <chrono>
#include "CompilerSettings.h"

// Compiler define which overrides the standard high performance timer components with those 
// based off QueryPerformanceCounter (QPF), which is Windows-only.  QPF override used for 
// MSVC builds up to & including VS2013 since the timer resolution fix was only implemented
// in MSVC implementation from VS2015 onwards
#define RJ_USE_QPF_TIMER_OVERRIDE

class Timers
{
public:
	
	// High resolution clock timer types and other required data
#	ifdef RJ_USE_QPF_TIMER_OVERRIDE
		typedef double										HRClockTime;
		typedef double										HRClockDuration;

		static __int64										HRClockStart;
		static double										HRClockFreq, HRClockFreqRecip;
#	else
		typedef std::chrono::steady_clock					HRClock;
		typedef std::chrono::steady_clock::time_point		HRClockTime;
		typedef double										HRClockDuration;
#	endif 

	// Initialisation method for the HR timer
	CMPINLINE static bool				Initialise(void)
	{
#		ifdef RJ_USE_QPF_TIMER_OVERRIDE
			// Determine the clock frequency
			LARGE_INTEGER li;
			if (!QueryPerformanceFrequency(&li)) return false;
			Timers::HRClockFreq = double(li.QuadPart) / 1000.0f;
			Timers::HRClockFreqRecip = 1.0f / Timers::HRClockFreq;

			// Also detemine the start point of the counter
			QueryPerformanceCounter(&li);
			Timers::HRClockStart = li.QuadPart;

			return true;
#		else
			// Do nothing
			return true;
#		endif 
	}

	// Return the current high-resolution clock time
	CMPINLINE static HRClockTime		GetHRClockTime(void) 
	{ 
#		ifdef RJ_USE_QPF_TIMER_OVERRIDE
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return (double(li.QuadPart - Timers::HRClockStart) * Timers::HRClockFreqRecip);
#		else
			return HRClock::now(); 
#		endif
	}

	// Returns the time elapsed between two high-resolution clock times
	CMPINLINE static HRClockDuration	GetMillisecondDuration(HRClockTime start, HRClockTime end) 
	{
#		ifdef RJ_USE_QPF_TIMER_OVERRIDE
			return (end - start);
#		else
			return std::chrono::duration<double, std::milli>(end - start).count(); 
#		endif
	}

	CMPINLINE static HRClockTime		GetZeroTime(void)
	{
#		ifdef RJ_USE_QPF_TIMER_OVERRIDE
			return 0.0;
#		else
			std::chrono::steady_clock::from_time_t(0U)
#		endif
	}

	CMPINLINE static HRClockDuration	GetZeroDuration(void)
	{
#		ifdef RJ_USE_QPF_TIMER_OVERRIDE
			return 0.0;
#		else
			return 0.0;
#		endif
	}
};



#endif



