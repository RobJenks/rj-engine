#pragma once

#ifndef __FrameProfilerH__
#define __FrameProfilerH__

#include "GlobalFlags.h"
#include "CompilerSettings.h"

// Frame profiling functions will only be included in the build if the preprocessor define is set
#ifdef RJ_ENABLE_FRAME_PROFILER

	namespace FrameProfiler
	{
		// Flag which indicates that the next frame should be profiled
		extern bool			ActivateProfiler;

		// Flag which indicates whether this frame is currently being profiled
		extern bool			ProfilerActive;

		// Activate profiling of the upcoming frame
		CMPINLINE void		ProfileNextFrame(void)		{ ActivateProfiler = true; }

		// Called at the start of each frame
#		define RJ_FRAME_PROFILER_NEW_FRAME	\
			if (FrameProfiler::ActivateProfiler) \
			{ \
				FrameProfiler::ProfilerActive = true; \
				FrameProfiler::ActivateProfiler = false; \
				OutputDebugString("### BEGIN FRAME PROFILING ###\n"); \
			} 

		// Called at the end of each frame
#		define RJ_FRAME_PROFILER_END_FRAME \
			if (FrameProfiler::ProfilerActive) \
			{ \
				FrameProfiler::ProfilerActive = false; \
				OutputDebugString("### END FRAME PROFILING\n"); \
			} 
		
		// Executes an expression if the per-frame profiler is running this frame
#		define RJ_FRAME_PROFILER_EXECUTE(...) \
			if (FrameProfiler::ProfilerActive) { __VA_ARGS__ }

		// Outputs an expression if the per-frame profiler is running this frame
#		define RJ_FRAME_PROFILER_OUTPUT(...) \
			if (FrameProfiler::ProfilerActive) { OutputDebugString(__VA_ARGS__); }

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

}


#endif



#endif







