#include "Profiler.h"


// Store of profiling data per main function
#ifdef RJ_PROFILER_ACTIVE

	namespace Profiler 
	{
		// Data used to log profiling data
		ProfilingDataType			ProfilingData[Profiler::ProfiledFunctions::Prf_COUNT];
		unsigned int				ClocksSinceLastProfile = 0U;
		unsigned int				FramesSinceLastProfile = 0U;
		Timers::HRClockDuration		CurrentResult = Timers::GetZeroDuration();


		// Initialisation method for the profiler
		void InitialiseProfiler(void)
		{
			// Set the description of each profiling group
			ProfilingData[ProfiledFunctions::Prf_BeginCycle].description = "Begin cycle";
			ProfilingData[ProfiledFunctions::Prf_ProcessInput].description = "Process input";
			ProfilingData[ProfiledFunctions::Prf_CentralScheduler].description = "Central scheduler";
			ProfilingData[ProfiledFunctions::Prf_SimulateSpaceObjectMovement].description = "Simulate objects";
			ProfilingData[ProfiledFunctions::Prf_CollisionDetection].description = "Collision detection";
			ProfilingData[ProfiledFunctions::Prf_UpdateRegions].description = "Update regions";
			ProfilingData[ProfiledFunctions::Prf_UpdateAudio].description = "Update audio";
			ProfilingData[ProfiledFunctions::Prf_Render].description = "Render";
			ProfilingData[ProfiledFunctions::Prf_Render_AnalyseFrameLighting].description = "Render lighting analysis";
			ProfilingData[ProfiledFunctions::Prf_Render_SimpleShips].description = "Render simple ships";
			ProfilingData[ProfiledFunctions::Prf_Render_ComplexShips].description = "Render complex ships";
			ProfilingData[ProfiledFunctions::Prf_Render_ObjectEnvironments].description = "Render environments";
			ProfilingData[ProfiledFunctions::Prf_Render_Actors].description = "Render actors";
			ProfilingData[ProfiledFunctions::Prf_Render_ImmediateRegion].description = "Render immediate region";
			ProfilingData[ProfiledFunctions::Prf_Render_SystemRegion].description = "Render system region";
			ProfilingData[ProfiledFunctions::Prf_Render_UI].description = "Render UI";
			ProfilingData[ProfiledFunctions::Prf_Render_Effects].description = "Render effects";
			ProfilingData[ProfiledFunctions::Prf_Render_Particles].description = "Render particles";
			ProfilingData[ProfiledFunctions::Prf_Render_ProcessRenderQueue].description = "Process render queue";
			ProfilingData[ProfiledFunctions::Prf_DebugInfoRendering].description = "Render debug info";
		}
	};

#endif