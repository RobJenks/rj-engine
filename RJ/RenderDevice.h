#pragma once

#include <string>
#include "CompilerSettings.h"
#include "DX11_Core.h"

class RenderDevice
{
public:

	CMPINLINE std::string		GetRenderDeviceName(void) const { return m_renderdevice_name; }

	std::string					DXDisplayModeToString(const DXGI_MODE_DESC & mode);

protected:

	CMPINLINE void				SetRenderDeviceName(const std::string & name) { m_renderdevice_name = name; }

private:

	// String identifier for the render device implementation currently in use
	std::string					m_renderdevice_name;

};