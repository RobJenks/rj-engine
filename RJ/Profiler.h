#pragma once

#ifndef __ProfilerH__
#define __ProfilerH__

#include <time.h>
#include <string>
#include "GlobalFlags.h"
#include "Utility.h"
#include "GameVarsExtern.h"
#include "LogManager.h"
#include "Timers.h"

// The components in this file have no special alignment requirements

/*** Determines the method(s) by which profiling data is reported ***/
//#define RJ_PROFILE_TO_DEBUG_OUT
#define RJ_PROFILE_TO_STREAM_OUT

// The functions in scope for profiling, and struct definition for holding the data
namespace Profiler {
	enum ProfiledFunctions {
		Prf_BeginCycle = 0,
		Prf_ProcessInput,
		Prf_CentralScheduler,
		Prf_SimulateSpaceObjectMovement,
		Prf_CollisionDetection,
		Prf_UpdateRegions,
		Prf_UpdateAudio,
		Prf_Render,									
		Prf_Render_AnalyseFrameLighting,
		Prf_Render_SimpleShips,						
		Prf_Render_ComplexShips,					
		Prf_Render_ObjectEnvironments,				
		Prf_Render_Actors,							
		Prf_Render_ImmediateRegion,					
		Prf_Render_SystemRegion,					
		Prf_Render_UI,								
		Prf_Render_Effects,							
		Prf_Render_Particles,						
		Prf_Render_ProcessRenderQueue,				
		Prf_DebugInfoRendering,						

		Prf_COUNT
	};

	struct ProfilingDataType {
		Timers::HRClockTime clock_start;
		Timers::HRClockTime clock_end;

		Timers::HRClockDuration total_clocks;		// Sum of (clock_end-clock_start) accumulated so far
		unsigned int iterations;					// The number of times we have summed the total above

		std::string description;					// Description of the profiling group

		// Default constructor to initialise all values 
		ProfilingDataType() : clock_start(0), clock_end(0), total_clocks(0), iterations(0U), description("[Unknown]") { }
	};
};


// Store of profiling data per main function
#ifdef RJ_PROFILER_ACTIVE

	// The number of clocks between each log of profiling data
#	define RJ_PROFILER_LOG_FREQ 1000

	// Data used to log profiling data
	namespace Profiler {
		extern ProfilingDataType		ProfilingData[Profiler::ProfiledFunctions::Prf_COUNT];
		extern unsigned int				ClocksSinceLastProfile;
		extern unsigned int				FramesSinceLastProfile;
		extern Timers::HRClockDuration	CurrentResult;				// Used to store current profile calculation during streaming

		// Initialisation method for the profiler
		void InitialiseProfiler(void);
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
		(Profiler::ProfilingData[fn].clock_start = Timers::GetHRClockTime());

	// End profiling of the specific area
#	define RJ_PROFILE_END(fn) \
		(Profiler::ProfilingData[fn].clock_end = Timers::GetHRClockTime()); \
		(Profiler::ProfilingData[fn].total_clocks += Timers::GetMillisecondDuration(Profiler::ProfilingData[fn].clock_start, Profiler::ProfilingData[fn].clock_end)); \
		(++(Profiler::ProfilingData[fn].iterations)); 

	// Called once per frame.  Update profiling results and output details when applicable
#	define RJ_PROFILE_LOG \
		(++Profiler::FramesSinceLastProfile); \
		(Profiler::ClocksSinceLastProfile += Game::PersistentClockDelta); \
		if (Profiler::ClocksSinceLastProfile >= RJ_PROFILER_LOG_FREQ) \
				{ \
			RJ_PROFILE_STREAMOUT_OPEN \
			RJ_PROFILE_DEBUGOUT_OPEN \
			for (int i = 0; i < Profiler::ProfiledFunctions::Prf_COUNT; ++i) \
							{ \
					Profiler::CurrentResult = (Profiler::ProfilingData[i].iterations == 0U ? 0.0f : (double)Profiler::ProfilingData[i].total_clocks / (double)Profiler::ProfilingData[i].iterations); \
					RJ_PROFILE_DEBUG_OUT(i) \
					RJ_PROFILE_STREAM_OUT(i) \
					(Profiler::ProfilingData[i].clock_start = Profiler::ProfilingData[i].clock_end = Timers::GetZeroTime()); \
					(Profiler::ProfilingData[i].total_clocks = Timers::GetZeroDuration()); (Profiler::ProfilingData[i].iterations = 0U); \
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
			Game::Log.ProfilingStream() << Game::PersistentClockMs << ", -1, " << Profiler::FramesSinceLastProfile << "\n";
#	else
#		define RJ_PROFILE_STREAMOUT_OPEN ;
#	endif

	// Opens the debug streaming for a set of results
#	ifdef RJ_PROFILE_TO_DEBUG_OUT
#		define RJ_PROFILE_DEBUGOUT_OPEN \
			OutputDebugString(concat("PROFILE, ")(Game::PersistentClockMs)(", -1, ")(Profiler::FramesSinceLastProfile)("\n").str().c_str());
#	else
#		define RJ_PROFILE_DEBUGOUT_OPEN ;
#	endif

	// Stream the results for profile i to the output file
#	ifdef RJ_PROFILE_TO_STREAM_OUT
#		define RJ_PROFILE_STREAM_OUT(i) \
			Game::Log.ProfilingStream() << Game::PersistentClockMs << ", " << i << ", " << Profiler::ProfilingData[i].description << ", " \
										<< Profiler::ProfilingData[i].total_clocks << ", " \
										<< Profiler::ProfilingData[i].iterations << ", " << Profiler::CurrentResult << "\n";
#	else
#		define RJ_PROFILE_STREAM_OUT(i) ;
#	endif

	// Stream the results for profile i to the debug output
#	ifdef RJ_PROFILE_TO_DEBUG_OUT
#		define RJ_PROFILE_DEBUG_OUT(i) \
			OutputDebugString(concat("PROFILE, ")(Game::PersistentClockMs)(", ")(i)(", ")(Profiler::ProfilingData[i].description)(", ") \
									(Profiler::ProfilingData[i].total_clocks)(", ") \
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

	namespace Profiler
	{
		// No profiler initialisation if it is not active
		CMPINLINE InitialiseProfiler(void) { }
	}


#endif	// End test of whether profiling is enabled





#endif	// End header guards










