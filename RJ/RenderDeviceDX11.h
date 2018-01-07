#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "ManagedPtr.h"
#include "IntVector.h"
#include "RenderDevice.h"
#include "RenderAssetsDX11.h"
#include "Shader.h"
#include "DX11_Core.h"
#include "Rendering.h"
#include "IntVector.h"
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "InputLayoutDesc.h"
#include "Texture.h"
#include "CPUGraphicsResourceAccess.h"
class RenderTargetDX11;


// Initialisation step with error logging and immediate-exit
#define PERFORM_INIT(initialisation, desc) \
{ \
Game::Log << LOG_INFO << "Initialising " << desc << "\n"; \
result = initialisation; \
if (result != ErrorCodes::NoError) \
{ \
	Game::Log << LOG_ERROR << "Rendering engine startup failed [" << result << "] during initialisation of " << desc << "\n"; \
	return result; \
} \
}


class RenderDeviceDX11 : public RenderDevice
{
public:

	RenderDeviceDX11(void);
	Result											Initialise(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_far);
	
	Result											InitialiseRenderDevice(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync);
	Result											InitialisePrimaryGraphicsAdapter(INTVECTOR2 screen_size, bool vsync);
	Result											InitialiseSwapChain(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync);
	Result											InitialisePrimaryRenderTarget(INTVECTOR2 screen_size);

	Result											InitialiseInputLayoutDefinitions(void);
	Result											InitialiseShaderResources(void);
	Result											InitialiseSamplerStateDefinitions(void);
	Result											InitialiseStandardRenderPipelines(void);


	CMPINLINE Rendering::RenderDeviceType *			GetDevice() { return m_device; }
	CMPINLINE Rendering::RenderDeviceContextType *	GetDeviceContext() { return m_devicecontext; }

	CMPINLINE RenderTargetDX11 *					GetPrimaryRenderTarget(void) { return m_rendertarget; }
	CMPINLINE ID3D11Texture2D * 					GetBackBuffer(void) { return m_backbuffer; }

	CMPINLINE float									GetFOV(void) const { return m_fov; }
	CMPINLINE float									GetAspectRatio(void) const { return m_aspectratio; }
	CMPINLINE float									GetTanOfHalfFOV(void) const { return m_halffovtan; }
	CMPINLINE float									GetNearClipDistance(void) const { return m_screen_near; }
	CMPINLINE float									GetFarClipDistance(void) const { return m_screen_far; }
	CMPINLINE INTVECTOR2							GetDisplaySize(void) const { return m_displaysize; }
	CMPINLINE XMFLOAT2								GetDisplaySizeF(void) const { return m_displaysize_f; }

	CMPINLINE XMMATRIX								GetProjectionMatrix(void) const { return m_projection; }
	CMPINLINE XMMATRIX								GetOrthoMatrix(void) const { return m_orthographic; }
	CMPINLINE XMMATRIX								GetInverseProjectionMatrix(void) const { return m_invproj; }
	
	// Rendering assets
	RenderAssetsDX11								Assets;


	void											SetDisplaySize(INTVECTOR2 display_size);
	void											SetFOV(float fov);
	void											SetDepthPlanes(float screen_near, float screen_far);
	void											SetSampleDesc(UINT count, UINT quality);

	void											RecalculateProjectionMatrix(void);
	void											RecalculateOrthographicMatrix(void);


	/* Methods to initiate each stage of the deferred rendering process per-frame */
	void											BeginDeferredRenderingFrame(void);
	// ...
	void											EndDeferredRenderinFrame(void);


	static DXGI_RATIONAL							QueryRefreshRateForDisplaySize(UINT screenwidth, UINT screenheight, bool vsync);
	
	// The number of samples to be taken for multi-sample textures
	static const uint8_t							TEXTURE_MULTISAMPLE_COUNT = 1U;
	
	~RenderDeviceDX11(void);



private:

	Rendering::RenderDeviceType *			m_device;
	Rendering::RenderDeviceContextType *	m_devicecontext;
	Rendering::SwapChainInterfaceType *		m_swapchain;
	ID3D11Texture2D *						m_backbuffer;
	RenderTargetDX11 *						m_rendertarget;

	std::string								m_devicename;
	size_t									m_devicememory;
	D3D_DRIVER_TYPE							m_drivertype;		// Hardware (in almost all cases) or WARP (software backup)
	ID3D11Debug *							m_debuglayer;		// Only if required & initialised

	float									m_fov;
	float									m_halffovtan;
	INTVECTOR2								m_displaysize;
	XMFLOAT2								m_displaysize_f;
	float									m_aspectratio;
	float									m_screen_near;
	float									m_screen_far;
	XMMATRIX								m_projection;
	XMMATRIX								m_invproj;
	XMMATRIX								m_orthographic;
	DXGI_SAMPLE_DESC						m_sampledesc;


	ShaderDX11 *							m_standard_vs;
	ShaderDX11 *							m_standard_ps;
	ShaderDX11 *							m_deferred_geometry_ps;
	ShaderDX11 *							m_deferred_lighting_ps;

	InputLayoutDesc							m_standard_input_layout;

	SamplerStateDX11 *						m_sampler_linearclamp;
	SamplerStateDX11 *						m_sampler_linearrepeat;
	

	// We will negotiate the highest possible supported feature level when attempting to initialise the render device
	static const D3D_FEATURE_LEVEL			SUPPORTED_FEATURE_LEVELS[];
};



