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


// Initialisation steps with error logging and immediate-exit
#define PERFORM_INIT_STEP(action_name, desc_name, expression, desc) \
{ \
Game::Log << LOG_INFO << action_name << " " << desc << "\n"; \
result = expression; \
if (result != ErrorCodes::NoError) \
{ \
	Game::Log << LOG_ERROR << "Rendering engine startup failed [" << result << "] during " << desc_name << " of " << desc << "\n"; \
	return result; \
} \
}

#define PERFORM_INIT(initialisation, desc) PERFORM_INIT_STEP("Initialising", "initialisation", initialisation, desc);
#define PERFORM_VALIDATION(validation, desc) PERFORM_INIT_STEP("Validating", "validation", validation, desc);



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
	Result											InitialisePreAssignableShaderParameters(void);
	Result											InitialiseSamplerStateDefinitions(void);
	Result											InitialiseStandardRenderPipelines(void);

	Result											ValidateShaders(void);

	// Perform any late initialisation that requires access to loaded game data
	void											PerformPostDataLoadInitialisation(void);

	// Verify the render device is in a good state and report errors if not
	bool											VerifyState(void);

	CMPINLINE Rendering::RenderDeviceType *			GetDevice() { return m_device; }
	CMPINLINE Rendering::RenderDeviceContextType *	GetDeviceContext() { return m_devicecontext; }

	CMPINLINE RenderTargetDX11 *					GetPrimaryRenderTarget(void) { return m_rendertarget; }
	CMPINLINE ID3D11Texture2D * 					GetBackBuffer(void) { return m_backbuffer; }

	CMPINLINE float									GetFOV(void) const { return m_fov; }
	CMPINLINE float									GetAspectRatio(void) const { return m_aspectratio; }
	CMPINLINE bool									IsVsyncEnabled(void) const { return m_vsync; }
	CMPINLINE float									GetTanOfHalfFOV(void) const { return m_halffovtan; }
	CMPINLINE float									GetNearClipDistance(void) const { return m_screen_near; }
	CMPINLINE float									GetFarClipDistance(void) const { return m_screen_far; }
	CMPINLINE INTVECTOR2							GetDisplaySize(void) const { return m_displaysize; }
	CMPINLINE UINTVECTOR2							GetDisplaySizeU(void) const { return m_displaysize_u; }
	CMPINLINE XMFLOAT2								GetDisplaySizeF(void) const { return m_displaysize_f; }

	CMPINLINE XMMATRIX								GetProjectionMatrix(void) const { return m_projection; }
	CMPINLINE XMMATRIX								GetOrthoMatrix(void) const { return m_orthographic; }
	CMPINLINE XMMATRIX								GetInverseProjectionMatrix(void) const { return m_invproj; }
	
	// Rendering assets
	RenderAssetsDX11								Assets;

	// Direct access to common or core engine assets
	const MaterialDX11 *							NullMaterial(void) const { return m_material_null; }
	const MaterialDX11 *							DefaultMaterial(void) const { return m_material_null; }

	// Return configuration for the primary render target buffers
	Texture::TextureFormat							PrimaryRenderTargetColourBufferFormat(void) const;
	Texture::TextureFormat							PrimaryRenderTargetDepthStencilBufferFormat(void) const;

	// Viewport configuration
	void											SetDisplaySize(INTVECTOR2 display_size);
	void											SetVsyncEnabled(bool vsync_enabled);
	void											SetFOV(float fov);
	void											SetDepthPlanes(float screen_near, float screen_far);
	void											SetSampleDesc(UINT count, UINT quality);

	void											RecalculateProjectionMatrix(void);
	void											RecalculateOrthographicMatrix(void);


	// Return a reference to the primary viewport
	CMPINLINE const Viewport &						GetPrimaryViewport(void) const { return m_viewport; }

	// Update the primary viewport; this will propogate to all resources which have already been 
	// created and associated with the default viewport
	void											UpdatePrimaryViewportSize(XMFLOAT2 size);


	// Present backbuffer to the primary display by cycling the swap chain
	HRESULT											PresentFrame(void);

	// Attempt to hot-load all shaders and recompile them in-place
	void											ReloadAllShaders(void);

	// Attempt to reload material data from disk
	Result											ReloadMaterial(MaterialDX11 * material);
	Result											ReloadMaterial(const std::string & material);
	void											ReloadAllMaterials(void);

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
	ID3D11InfoQueue *						m_debuginfoqueue;	// Only if required & initialised

	float									m_fov;
	float									m_halffovtan;
	INTVECTOR2								m_displaysize;
	UINTVECTOR2								m_displaysize_u;
	XMFLOAT2								m_displaysize_f;
	Viewport								m_viewport;			// Primary, full-screen viewport
	float									m_aspectratio;
	bool									m_vsync;
	UINT									m_sync_interval;
	float									m_screen_near;
	float									m_screen_far;
	XMMATRIX								m_projection;
	XMMATRIX								m_invproj;
	XMMATRIX								m_orthographic;
	DXGI_SAMPLE_DESC						m_sampledesc;


	ShaderDX11 *							m_standard_vs;
	ShaderDX11 *							m_standard_ps;
	ShaderDX11 *							m_quad_vs;
	ShaderDX11 *							m_deferred_geometry_ps;
	ShaderDX11 *							m_deferred_lighting_ps;
	ShaderDX11 *							m_deferred_debug_ps;
	ShaderDX11 *							m_texture_vs;
	ShaderDX11 *							m_texture_ps;
	ShaderDX11 *							m_sdf_decal_direct_vs;
	ShaderDX11 *							m_sdf_decal_deferred_vs;
	ShaderDX11 *							m_sdf_decal_direct_ps;
	ShaderDX11 *							m_sdf_decal_deferred_ps;
	ShaderDX11 *							m_post_motionblur_tilegen_ps;
	ShaderDX11 *							m_post_motionblur_neighbour_ps;
	ShaderDX11 *							m_post_motionblur_gather_ps;

	InputLayoutDesc							m_standard_input_layout;
	InputLayoutDesc							m_fullscreen_quad_input_layout;

	SamplerStateDX11 *						m_sampler_linearclamp;
	SamplerStateDX11 *						m_sampler_linearrepeat;
	SamplerStateDX11 *						m_sampler_pointclamp;
	SamplerStateDX11 *						m_sampler_pointrepeat;

	const MaterialDX11 *					m_material_null;
	const MaterialDX11 *					m_material_default;

	// We will negotiate the highest possible supported feature level when attempting to initialise the render device
	static const D3D_FEATURE_LEVEL			SUPPORTED_FEATURE_LEVELS[];
};



