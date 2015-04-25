#include "Profiler.h"


// Store of profiling data per main function
#ifdef RJ_PROFILER_ACTIVE

	namespace Profiler 
	{
		// Data used to log profiling data
		ProfilingDataType		ProfilingData[Profiler::ProfiledFunctions::Prf_COUNT];
		unsigned int			ClocksSinceLastProfile = 0U;
		unsigned int			FramesSinceLastProfile = 0U;
		float					CurrentResult = 0.0f;
	};

#endif