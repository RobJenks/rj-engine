#pragma once

#ifndef __FrameProfilerH__
#define __FrameProfilerH__

#include "GlobalFlags.h"
#include "CompilerSettings.h"
#include "Utility.h"
#include "Timers.h"

// Frame profiling functions will only be included in the build if the preprocessor define is set
#ifdef RJ_ENABLE_FRAME_PROFILER

	namespace FrameProfiler
	{
		// Flag which indicates that the next frame should be profiled
		extern bool					ActivateProfiler;

		// Flag which indicates whether this frame is currently being profiled
		extern bool					ProfilerActive;

		// Clock time recorded when profiling began at the start of the frame
		extern Timers::HRClockTime	FrameStart;

		// Clock time recorded at the last checkpoint
		extern Timers::HRClockTime	LastCheckpoint;

		// Index of the current checkpoint
		extern int					CheckpointNumber;

		// Activate profiling of the upcoming frame
		CMPINLINE void				ProfileNextFrame(void)		{ ActivateProfiler = true; }

		// Called at the start of each frame
#		define RJ_FRAME_PROFILER_NEW_FRAME	\
			if (FrameProfiler::ActivateProfiler) \
			{ \
				FrameProfiler::ProfilerActive = true; \
				FrameProfiler::ActivateProfiler = false; \
				FrameProfiler::FrameStart = FrameProfiler::LastCheckpoint = Timers::GetHRClockTime(); \
				FrameProfiler::CheckpointNumber = 0; \
				OutputDebugString("### BEGIN FRAME PROFILING ###\n"); \
			} 

		// Called at the end of each frame
#		define RJ_FRAME_PROFILER_END_FRAME \
			if (FrameProfiler::ProfilerActive) \
			{ \
				FrameProfiler::ProfilerActive = false; \
				OutputDebugString(concat("-- Total frame time: ")(Timers::GetMillisecondDuration(FrameProfiler::FrameStart, Timers::GetHRClockTime()))("ms\n").str().c_str()); \
				OutputDebugString("### END FRAME PROFILING ###\n"); \
			} 

		// Executes an expression if the per-frame profiler is running this frame
#		define RJ_FRAME_PROFILER_EXECUTE(...) \
			if (FrameProfiler::ProfilerActive) { __VA_ARGS__ }

		// Outputs an expression if the per-frame profiler is running this frame
#		define RJ_FRAME_PROFILER_OUTPUT(...) \
			if (FrameProfiler::ProfilerActive) { OutputDebugString(__VA_ARGS__); }

		// Outputs a frame time checkpoint with the specified label
#		define RJ_FRAME_PROFILER_CHECKPOINT(text) \
			if (FrameProfiler::ProfilerActive) \
			{ \
				Timers::HRClockTime time_now = Timers::GetHRClockTime(); \
				Timers::HRClockDuration event_time = Timers::GetMillisecondDuration(FrameProfiler::FrameStart, time_now); \
				Timers::HRClockDuration last_event_duration = Timers::GetMillisecondDuration(FrameProfiler::LastCheckpoint, time_now); \
				FrameProfiler::LastCheckpoint = time_now; \
				if (FrameProfiler::CheckpointNumber != 0) OutputDebugString(concat("-- CP-")(FrameProfiler::CheckpointNumber)(" completed in ")(last_event_duration)("ms\n").str().c_str()); \
				OutputDebugString(concat("-- CP-")(++FrameProfiler::CheckpointNumber)(" [")(event_time)("ms]: ")(text)("\n").str().c_str()); \
			}

	}


#else

namespace FrameProfiler
{
	// Functions are not defined if the preprocessor flag is not set
	CMPINLINE void ProfileNextFrame(void) { }

#	define RJ_FRAME_PROFILER_NEW_FRAME ;
#	define RJ_FRAME_PROFILER_END_FRAME ;
#	define RJ_FRAME_PROFILER_EXECUTE(...) ;
#	define RJ_FRAME_PROFILER_OUTPUT(...) ;
#	define RJ_FRAME_PROFILER_CHECKPOINT(text) ;

}


#endif



#endif







