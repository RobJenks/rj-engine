#pragma once

#include <string>
#include "RenderDevice.h"
#include "DX11_Core.h"
#include "Rendering.h"
#include "IntVector.h"
#include "ErrorCodes.h"
#include "CompilerSettings.h"

class RenderDeviceDX11 : public RenderDevice
{
public:

	RenderDeviceDX11(void);
	Result Initialise(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_depth);
	
	CMPINLINE Rendering::RenderDeviceType *			GetDevice() { return m_device; }
	CMPINLINE Rendering::RenderDeviceContextType *	GetDeviceContext() { return m_devicecontext; }
	
	Result											InitialisePrimaryGraphicsAdapter(INTVECTOR2 screen_size, bool vsync);
	
	~RenderDeviceDX11(void);
	

private:

	Rendering::RenderDeviceType *			m_device;
	Rendering::RenderDeviceContextType *	m_devicecontext;

	std::string								m_devicename;
	size_t									m_devicememory;
	D3D_DRIVER_TYPE							m_drivertype;		// Hardware (in almost all cases) or WARP (software backup)
	ID3D11Debug *							m_debuglayer;		// Only if required & initialised

	// TODO: Add central collection of sampler/pipeline definitions here (or elsewhere) to ensure maximum reuse

	// We will negotiate the highest possible supported feature level when attempting to initialise the render device
	static const D3D_FEATURE_LEVEL			SUPPORTED_FEATURE_LEVELS[];
};