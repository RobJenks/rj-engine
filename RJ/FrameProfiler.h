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

		// Internal index used for ad-hoc block profiling
		extern size_t				_INTERNAL_BLOCK_ID;

		// Activate profiling of the upcoming frame
		CMPINLINE void				ProfileNextFrame(void) { ActivateProfiler = true; }

		// Called at the start of each frame
#		define RJ_FRAME_PROFILER_NEW_FRAME	\
			if (FrameProfiler::ActivateProfiler) \
			{ \
				FrameProfiler::ProfilerActive = true; \
				FrameProfiler::ActivateProfiler = false; \
				FrameProfiler::FrameStart = FrameProfiler::LastCheckpoint = Timers::GetHRClockTime(); \
				FrameProfiler::CheckpointNumber = 0; \
				FrameProfiler::_INTERNAL_BLOCK_ID = 0U; \
				Game::Log << LOG_DEBUG << ("### BEGIN FRAME PROFILING ###\n"); \
			} 

		// Called at the end of each frame
#		define RJ_FRAME_PROFILER_END_FRAME \
			if (FrameProfiler::ProfilerActive) \
			{ \
				FrameProfiler::ProfilerActive = false; \
				Timers::HRClockDuration total_frame_time = Timers::GetMillisecondDuration(FrameProfiler::FrameStart, Timers::GetHRClockTime()); \
				Game::Log << LOG_DEBUG << (concat("-- Total frame time: ")(total_frame_time)("ms [approx ")(total_frame_time == 0.0 ? "<ERR>" : concat((int)(1000.0 / total_frame_time)).str().c_str())(" FPS]\n").str().c_str()); \
				Game::Log << LOG_DEBUG << ("### END FRAME PROFILING ###\n"); \
			} 

		// Compiles an expression only if the debug frame profiling components are loaded (at compile time)
#		define RJ_FRAME_PROFILER_EXPR(...) __VA_ARGS__;

		// Executes an expression if the per-frame profiler is running this frame (at runtime)
#		define RJ_FRAME_PROFILER_EXECUTE(...) \
			if (FrameProfiler::ProfilerActive) { __VA_ARGS__ }

		// Outputs an expression if the per-frame profiler is running this frame
#		define RJ_FRAME_PROFILER_OUTPUT(...) \
			if (FrameProfiler::ProfilerActive) { Game::Log << LOG_DEBUG << __VA_ARGS__; }

		// Outputs a frame time checkpoint with the specified label
#		define RJ_FRAME_PROFILER_CHECKPOINT(text) \
			if (FrameProfiler::ProfilerActive) \
			{ \
				Timers::HRClockTime time_now = Timers::GetHRClockTime(); \
				Timers::HRClockDuration event_time = Timers::GetMillisecondDuration(FrameProfiler::FrameStart, time_now); \
				Timers::HRClockDuration last_event_duration = Timers::GetMillisecondDuration(FrameProfiler::LastCheckpoint, time_now); \
				FrameProfiler::LastCheckpoint = time_now; \
				if (FrameProfiler::CheckpointNumber != 0) Game::Log << LOG_DEBUG << (concat("-- CP-")(FrameProfiler::CheckpointNumber)(" completed in ")(last_event_duration)("ms\n").str().c_str()); \
				Game::Log << LOG_DEBUG << (concat("-- CP-")(++FrameProfiler::CheckpointNumber)(" [")(event_time)("ms]: ")(text)("\n").str().c_str()); \
			}

		// Performs high-resolution timing of the given code block, if profiling is enabled
#		define RJ_FRAME_PROFILER_PROFILE_BLOCK(text, ...) \
			if (FrameProfiler::ProfilerActive) \
			{ \
				Timers::HRClockTime start_time_##__LINE__ = Timers::GetHRClockTime(); \
				{ __VA_ARGS__ } \
				Timers::HRClockDuration event_time = Timers::GetMillisecondDuration(start_time_##__LINE__, Timers::GetHRClockTime()); \
				Game::Log << LOG_DEBUG << text << " [" << event_time << "ms]\n"; \
			} \
			else \
			{ \
				__VA_ARGS__; \
			}

	}


#else

namespace FrameProfiler
{
	// Functions are not defined if the preprocessor flag is not set
	CMPINLINE void ProfileNextFrame(void) { }

#	define RJ_FRAME_PROFILER_NEW_FRAME ;
#	define RJ_FRAME_PROFILER_END_FRAME ;
#	define RJ_FRAME_PROFILER_EXPR(...) ;
#	define RJ_FRAME_PROFILER_EXECUTE(...) ;
#	define RJ_FRAME_PROFILER_OUTPUT(...) ;
#	define RJ_FRAME_PROFILER_CHECKPOINT(text) ;

	// Direct block profiling should still execute the block, but with no debug tracking 
#	define RJ_FRAME_PROFILER_PROFILE_BLOCK(text, ...) __VA_ARGS__;

}


#endif



#endif







