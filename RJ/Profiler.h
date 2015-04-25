#pragma once

#ifndef __ProfilerH__
#define __ProfilerH__

#include <time.h>
#include <string>
#include "GlobalFlags.h"
#include "Utility.h"
#include "GameVarsExtern.h"
#include "LogManager.h"


/*** Determines the method(s) by which profiling data is reported ***/
//#define RJ_PROFILE_TO_DEBUG_OUT
#define RJ_PROFILE_TO_STREAM_OUT

// The functions in scope for profiling, and struct definition for holding the data
namespace Profiler {
	enum ProfiledFunctions {
		Prf_BeginCycle = 0,							// 0
		Prf_ProcessInput,							// 1
		Prf_CentralScheduler,						// 2
		Prf_SimulateSpaceObjectMovement,			// 3
		Prf_CollisionDetection,						// 4
		Prf_UpdateRegions,							// 5
		Prf_Render,									// 6
		Prf_Render_SimpleShips,						// 7
		Prf_Render_ComplexShips,					// 8
		Prf_Render_ObjectEnvironments,				// 9
		Prf_Render_Actors,							// 10
		Prf_Render_ImmediateRegion,					// 11
		Prf_Render_SystemRegion,					// 12
		Prf_Render_UI,								// 13
		Prf_Render_Effects,							// 14
		Prf_Render_Particles,						// 15
		Prf_Render_ProcessRenderQueue,				// 16

		Prf_COUNT
	};

	struct ProfilingDataType {
		clock_t clock_start;
		clock_t clock_end;

		clock_t total_clocks;		// Sum of (clock_end-clock_start) accumulated so far
		unsigned int iterations;	// The number of times we have summed the total above

		// Default constructor to initialise all values 
		ProfilingDataType() { clock_start = clock_end = total_clocks = 0; iterations = 0U; }
	};
};


// Store of profiling data per main function
#ifdef RJ_PROFILER_ACTIVE

	// The number of clocks between each log of profiling data
#	define RJ_PROFILER_LOG_FREQ 1000

	// Data used to log profiling data
	namespace Profiler {
		extern ProfilingDataType	ProfilingData[Profiler::ProfiledFunctions::Prf_COUNT];
		extern unsigned int			ClocksSinceLastProfile;
		extern unsigned int			FramesSinceLastProfile;
		extern float				CurrentResult;				// Used to store current profile calculation during streaming
	}

#endif


	// Profiling methods
#ifdef RJ_PROFILER_ACTIVE

#	undef RJ_PROFILE_START
#	undef RJ_PROFILE_END
#	undef RJ_PROFILE_LOG
#	undef RJ_ADDPROFILE
#	undef RJ_PROFILED
#	undef RJ_PROFILE_STREAMOUT_OPEN
#	undef RJ_PROFILE_DEBUGOUT_OPEN
#	undef RJ_PROFILE_DEBUG_OUT
#	undef RJ_PROFILE_STREAM_OUT
#	undef RJ_PROFILE_STREAM_FLUSH

	// Begin profiling the specified area
#	define RJ_PROFILE_START(fn) \
		(Profiler::ProfilingData[fn].clock_start = clock());

	// End profiling of the specific area
#	define RJ_PROFILE_END(fn) \
		(Profiler::ProfilingData[fn].clock_end = clock()); \
		(Profiler::ProfilingData[fn].total_clocks += (Profiler::ProfilingData[fn].clock_end - Profiler::ProfilingData[fn].clock_start)); \
		(++(Profiler::ProfilingData[fn].iterations)); 

	// Called once per frame.  Update profiling results and output details when applicable
#	define RJ_PROFILE_LOG \
		(++Profiler::FramesSinceLastProfile); \
		(Profiler::ClocksSinceLastProfile += Game::ClockDelta); \
		if (Profiler::ClocksSinceLastProfile >= RJ_PROFILER_LOG_FREQ) \
		{ \
			RJ_PROFILE_STREAMOUT_OPEN \
			RJ_PROFILE_DEBUGOUT_OPEN \
			for (int i = 0; i < Profiler::ProfiledFunctions::Prf_COUNT; ++i) \
				{ \
					Profiler::CurrentResult = (Profiler::ProfilingData[i].iterations == 0U ? 0.0f : (float)Profiler::ProfilingData[i].total_clocks / (float)Profiler::ProfilingData[i].iterations); \
					RJ_PROFILE_DEBUG_OUT(i) \
					RJ_PROFILE_STREAM_OUT(i) \
					(Profiler::ProfilingData[i].clock_start = Profiler::ProfilingData[i].clock_end = Profiler::ProfilingData[i].total_clocks = 0); (Profiler::ProfilingData[i].iterations = 0U); \
				} \
			(Profiler::ClocksSinceLastProfile = 0U); \
			(Profiler::FramesSinceLastProfile = 0U); \
			RJ_PROFILE_STREAM_FLUSH \
		}


	// Convert a function declaration into a profiled method
#	define RJ_ADDPROFILE(profile, methodtype, method, argument_list, parameters) \
		methodtype method##_Profiled(argument_list); \
		CMPINLINE methodtype method(argument_list) \
			{ \
			RJ_PROFILE_START(profile); \
			method##_Profiled(parameters); \
			RJ_PROFILE_END(profile); \
			}

	// Designates the definition of a profiled method
#	define RJ_PROFILED(method, ...) \
		method##_Profiled(__VA_ARGS__)


	// Opens the output file steaming for a set of results.  Clears any error bits first, so that we can continue writing in
	// case of a previous failure due to e.g. concurrent access
#	ifdef RJ_PROFILE_TO_STREAM_OUT
#		define RJ_PROFILE_STREAMOUT_OPEN \
			Game::Log.ProfilingStream().clear(); \
			Game::Log.ProfilingStream() << Game::ClockMs << ", -1, " << Profiler::FramesSinceLastProfile << "\n";
#	else
#		define RJ_PROFILE_STREAMOUT_OPEN ;
#	endif

	// Opens the debug streaming for a set of results
#	ifdef RJ_PROFILE_TO_DEBUG_OUT
#		define RJ_PROFILE_DEBUGOUT_OPEN \
			OutputDebugString(concat("PROFILE, ")(Game::ClockMs)(", -1, ")(Profiler::FramesSinceLastProfile)("\n").str().c_str());
#	else
#		define RJ_PROFILE_DEBUGOUT_OPEN ;
#	endif

	// Stream the results for profile i to the output file
#	ifdef RJ_PROFILE_TO_STREAM_OUT
#		define RJ_PROFILE_STREAM_OUT(i) \
			Game::Log.ProfilingStream() << Game::ClockMs << ", " << i << ", " << Profiler::ProfilingData[i].total_clocks << ", " \
										<< Profiler::ProfilingData[i].iterations << ", " << Profiler::CurrentResult << "\n";
#	else
#		define RJ_PROFILE_STREAM_OUT(i) ;
#	endif

	// Stream the results for profile i to the debug output
#	ifdef RJ_PROFILE_TO_DEBUG_OUT
#		define RJ_PROFILE_DEBUG_OUT(i) \
			OutputDebugString(concat("PROFILE, ")(Game::ClockMs)(", ")(i)(", ")(Profiler::ProfilingData[i].total_clocks)(", ") \
									(Profiler::ProfilingData[i].iterations)(", ")(Profiler::CurrentResult)("\n").str().c_str());
#	else
#		define RJ_PROFILE_DEBUG_OUT(i) ;
#	endif

	// Flushes any buffered data to the profiling log file
#	ifdef RJ_PROFILE_TO_STREAM_OUT
#		define RJ_PROFILE_STREAM_FLUSH \
			Game::Log.ProfilingStream().flush();
#	else
#		define RJ_PROFILE_STREAM_FLUSH ;
#	endif




#else	// If profiling is NOT enabled

#	undef RJ_PROFILE_START
#	undef RJ_PROFILE_END
#	undef RJ_PROFILE_LOG
#	undef RJ_ADDPROFILE
#	undef RJ_PROFILED
#	undef RJ_PROFILE_STREAMOUT_OPEN
#	undef RJ_PROFILE_DEBUGOUT_OPEN
#	undef RJ_PROFILE_DEBUG_OUT
#	undef RJ_PROFILE_STREAM_OUT
#	undef RJ_PROFILE_STREAM_FLUSH


#	define RJ_PROFILE_START(fn) ;
#	define RJ_PROFILE_END(fn) ;
#	define RJ_PROFILE_LOG ;
#	define RJ_STREAM_OUT ;

#	define RJ_ADDPROFILE(profile, methodtype, method, ...) \
		methodtype method(__VA_ARGS__); 

#	define RJ_PROFILED(method, ...) \
		method(__VA_ARGS__)

#	define RJ_PROFILE_STREAMOUT_OPEN ;
#	define RJ_PROFILE_DEBUGOUT_OPEN ;
#	define RJ_PROFILE_DEBUG_OUT(i) ;
#	define RJ_PROFILE_STREAM_OUT(i) ;
#	define RJ_PROFILE_STREAM_FLUSH ;


#endif	// End test of whether profiling is enabled





#endif	// End header guards










