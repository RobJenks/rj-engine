#include "RenderDevice.h"
#include "Utility.h"
#include "Logging.h"


RenderDevice::RenderDevice(void)
	:
	m_render_process(NULL), 
	m_ui_render_process(NULL)
{
}

// Perform rendering; will delegate to the currently-active render process
void RenderDevice::Render(void)
{
	// We must have active render processes at all times
	assert(m_render_process != NULL);
	assert(m_ui_render_process != NULL);

	// Perform primary rendering of all scene geometry through the active render process
	GetActiveRenderProcess()->Render();

	// Perform UI and other orthographic/textured rendering
	GetActiveUIRenderProcess()->Render();
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