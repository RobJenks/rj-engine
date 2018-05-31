#include "RenderDevice.h"
#include "Utility.h"
#include "Logging.h"


RenderDevice::RenderDevice(void)
{
	// Initialise all active render processes to null on startup
	for (auto & active_process : m_active_render_processes)
	{
		active_process = NULL;		
	}
}


// Perform rendering; will delegate to the currently-active render process
void RenderDevice::Render(void)
{
	// Perform primary rendering of all scene geometry through the active render process
	ExecuteRenderProcess(RenderProcess::RenderProcessClass::Primary);

	// Perform UI and other orthographic/textured rendering
	ExecuteRenderProcess(RenderProcess::RenderProcessClass::UI);

	// Decal rendering handles all world- and screen-space decals, plus all text rendering in those same reference frames
	ExecuteRenderProcess(RenderProcess::RenderProcessClass::Decal);
}


// Execute the render process for the given phase
void RenderDevice::ExecuteRenderProcess(RenderProcess::RenderProcessClass process_type)
{
	RenderProcess *process = GetActiveRenderProcess(process_type);
	if (process)
	{
		process->Render();
	}
}

// Perform any late initialisation that requires access to loaded game data
void RenderDevice::PerformPostDataLoadInitialisation(void)
{
	Game::Log << LOG_INFO << "Performing post-data load initialisation of base render device\n";
	
	// Run post-data load initialisation for any render processes which were initialised before game data was available
	for (auto & entry : m_render_processes)
	{
		entry.second->PerformPostDataLoadInitialisation();
	}

	Game::Log << LOG_INFO << "Completed post-data load initialisation of base render device\n";
}


// Static method to convert a display mode to readable string
std::string RenderDevice::DXDisplayModeToString(const DXGI_MODE_DESC & mode)
{
	std::string modestr = "";
	if (mode.RefreshRate.Denominator == 0)
	{
		modestr = concat(modestr)("[")(mode.Width)("|")(mode.Height)("|")
			(mode.RefreshRate.Numerator)("/")(mode.RefreshRate.Denominator)("]").str();
	}
	else
	{
		modestr = concat(modestr)("[")(mode.Width)("|")(mode.Height)("|")
			(mode.RefreshRate.Numerator / mode.RefreshRate.Denominator)("]").str();
	}
	return modestr;
}


RenderDevice::~RenderDevice(void)
{

}