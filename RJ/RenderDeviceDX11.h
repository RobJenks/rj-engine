#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "RenderDevice.h"
#include "Shader.h"
#include "DX11_Core.h"
#include "Rendering.h"
#include "IntVector.h"
#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "InputLayoutDesc.h"
#include "Texture.h"
#include "CPUGraphicsResourceAccess.h"
class ShaderDX11;
class SamplerStateDX11;
class RenderTargetDX11;
class MaterialDX11;
class PipelineStateDX11;
class TextureDX11;

class RenderDeviceDX11 : public RenderDevice
{
public:

	typedef std::unordered_map<std::string, std::unique_ptr<ShaderDX11>> ShaderCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<SamplerStateDX11>> SamplerStateCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<RenderTargetDX11>> RenderTargetCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<MaterialDX11>> MaterialCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<PipelineStateDX11>> PipelineStateCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<TextureDX11>> TextureCollection;


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

	CMPINLINE XMMATRIX								GetProjectionMatrix(void) const { return m_projection; }
	CMPINLINE XMMATRIX								GetOrthoMatrix(void) const { return m_orthographic; }
	

	void											SetDisplaySize(INTVECTOR2 display_size);
	void											SetFOV(float fov);
	void											SetDepthPlanes(float screen_near, float screen_far);
	void											SetSampleDesc(UINT count, UINT quality);

	void											RecalculateProjectionMatrix(void);
	void											RecalculateOrthographicMatrix(void);

	CMPINLINE const ShaderCollection &				GetShaders(void) const { return m_shaders; }
	CMPINLINE const SamplerStateCollection &		GetSamplerStates(void) const { return m_samplers; }
	CMPINLINE const RenderTargetCollection &		GetRenderTargets(void) const { return m_rendertargets; }
	CMPINLINE const MaterialCollection &			GetMaterials(void) const { return m_materials; }
	CMPINLINE const PipelineStateCollection &		GetPipelineStates(void) const { return m_pipelinestates; }
	CMPINLINE const TextureCollection &				GetTextures(void) const { return m_textures; }

	CMPINLINE ShaderDX11 *							GetShader(const std::string & name)			{ return GetAsset<ShaderDX11>(name, m_shaders); }
	CMPINLINE SamplerStateDX11 *					GetSamplerState(const std::string & name)	{ return GetAsset<SamplerStateDX11>(name, m_samplers); }
	CMPINLINE RenderTargetDX11 *					GetRenderTarget(const std::string & name)	{ return GetAsset<RenderTargetDX11>(name, m_rendertargets); }
	CMPINLINE MaterialDX11 *						GetMaterial(const std::string & name)		{ return GetAsset<MaterialDX11>(name, m_materials); }
	CMPINLINE PipelineStateDX11 *					GetPipelineState(const std::string & name)	{ return GetAsset<PipelineStateDX11>(name, m_pipelinestates); }
	CMPINLINE TextureDX11 *							GetTexture(const std::string & name)		{ return GetAsset<TextureDX11>(name, m_textures); }


	/* Methods to initiate each stage of the deferred rendering process per-frame */
	void											BeginDeferredRenderingFrame(void);
	// ...
	void											EndDeferredRenderinFrame(void);


	static DXGI_RATIONAL							QueryRefreshRateForDisplaySize(UINT screenwidth, UINT screenheight, bool vsync);
	
	// The number of samples to be taken for multi-sample textures
	static const uint8_t							TEXTURE_MULTISAMPLE_COUNT = 1U;
	
	~RenderDeviceDX11(void);
	

public:


	CMPINLINE SamplerStateDX11 *					CreateSamplerState(const std::string & name)	{ return CreateAsset<SamplerStateDX11>(name, m_samplers); }
	CMPINLINE RenderTargetDX11 *					CreateRenderTarget(const std::string & name)	{ return CreateAsset<RenderTargetDX11>(name, m_rendertargets); }
	CMPINLINE MaterialDX11 *						CreateMaterial(const std::string & name)		{ return CreateAsset<MaterialDX11>(name, m_materials); }
	CMPINLINE PipelineStateDX11 *					CreatePipelineState(const std::string & name)	{ return CreateAsset<PipelineStateDX11>(name, m_pipelinestates); }

	TextureDX11 *									CreateTexture(const std::string & name);
	TextureDX11 *									CreateTexture1D(const std::string & name, uint16_t width, uint16_t slices = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTexture2D(const std::string & name, uint16_t width, uint16_t height, uint16_t slices = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTexture3D(const std::string & name, uint16_t width, uint16_t height, uint16_t depth, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);
	TextureDX11 *									CreateTextureCube(const std::string & name, uint16_t size, uint16_t numCubes = 1, const Texture::TextureFormat& format = Texture::TextureFormat(), CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool gpuWrite = false);

private:

	Result											InitialiseExternalShaderResource(ShaderDX11 ** ppOutShader, Shader::Type shadertype, const std::string & fileName, 
														const std::string & entryPoint, const std::string & profile, const InputLayoutDesc *input_layout = NULL);

	TextureDX11 *									RegisterNewTexture(const std::string & name, std::unique_ptr<TextureDX11> texture);

	template <class T>
	T *												CreateAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData);

	template <class T>
	T *												GetAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData);

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
	float									m_aspectratio;
	float									m_screen_near;
	float									m_screen_far;
	XMMATRIX								m_projection;
	XMMATRIX								m_orthographic;
	DXGI_SAMPLE_DESC						m_sampledesc;

	ShaderCollection						m_shaders;
	SamplerStateCollection					m_samplers;
	RenderTargetCollection					m_rendertargets;
	MaterialCollection						m_materials;
	PipelineStateCollection					m_pipelinestates;
	TextureCollection						m_textures;

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




template <class T>
T *	RenderDeviceDX11::CreateAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData)
{
	// Compile-type asset name
	constexpr std::string type = STRING(T);
	constexpr size_t ix = type.find_last_of("DX11");
	constexpr if (ix != std::string::npos)
	{
		type = type.substr(0U, ix)
	}

	if (name.empty()) { Game::Log << LOG_ERROR << "Cannot initialise " << type << " definition with null identifier\n"; return NULL; }

	if (assetData.find(name) != assetData.end())
	{
		Game::Log << LOG_WARN << type << " definition for \"" << name << "\" already exists, cannot create duplicate\n";
		return NULL;
	}

	assetData[name] = std::make_unique<T>();
	return assetData[name].get();
}

template <class T>
T *	RenderDeviceDX11::GetAsset(const std::string & name, std::unordered_map<std::string, std::unique_ptr<T>> & assetData)
{
	auto it = assetData.find(name);
	return (it != assetData.end() ? it->second.get() : NULL);
}





