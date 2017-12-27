#include "RenderDevice.h"
#include "Utility.h"


RenderDevice::RenderDevice(void)
	:
	m_render_process(NULL)
{
}

// Perform rendering; will delegate to the currently-active render process
void RenderDevice::Render(void)
{
	assert(m_render_process != NULL);

	m_render_process->Render();
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