#include "FrameProfiler.h"

// Frame profiling functions will only be included in the build if the preprocessor define is set
#ifdef RJ_ENABLE_FRAME_PROFILER

	namespace FrameProfiler
	{
		// Flag which indicates that the next frame should be profiled
		bool					ActivateProfiler = false;

		// Flag which indicates whether this frame is currently being profiled
		bool					ProfilerActive = false;

		// Clock time recorded when profiling began at the start of the frame
		Timers::HRClockTime		FrameStart = Timers::GetZeroTime();

		// Clock time recorded at the last checkpoint
		Timers::HRClockTime		LastCheckpoint = Timers::GetZeroTime();

		// Index of the current checkpoint
		int						CheckpointNumber = 0;
	}


#else

	namespace FrameProfiler
	{

	}


#endif

