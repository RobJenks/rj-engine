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
class ShaderDX11;
class SamplerStateDX11;
class RenderTargetDX11;
class MaterialDX11;
class PipelineStateDX11;

class RenderDeviceDX11 : public RenderDevice
{
public:

	typedef std::unordered_map<std::string, std::unique_ptr<ShaderDX11>> ShaderCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<SamplerStateDX11>> SamplerStateCollection;
	typedef std::vector<std::unique_ptr<RenderTargetDX11>> RenderTargetCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<MaterialDX11>> MaterialCollection;
	typedef std::unordered_map<std::string, std::unique_ptr<PipelineStateDX11>> PipelineStateCollection;


	RenderDeviceDX11(void);
	Result											Initialise(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_depth);
	
	Result											InitialiseRenderDevice(HWND hwnd, INTVECTOR2 screen_size, bool full_screen, bool vsync, float screen_near, float screen_depth);
	Result											InitialisePrimaryGraphicsAdapter(INTVECTOR2 screen_size, bool vsync);

	Result											InitialiseInputLayoutDefinitions(void);
	Result											InitialiseShaderResources(void);
	Result											InitialiseSamplerStateDefinitions(void);
	Result											InitialiseDeferredRenderingResources(void);


	CMPINLINE Rendering::RenderDeviceType *			GetDevice() { return m_device; }
	CMPINLINE Rendering::RenderDeviceContextType *	GetDeviceContext() { return m_devicecontext; }

	CMPINLINE const ShaderCollection &				GetShaders(void) const { return m_shaders; }
	CMPINLINE const SamplerStateCollection &		GetSamplerStates(void) const { return m_samplers; }
	CMPINLINE const RenderTargetCollection &		GetRenderTargets(void) const { return m_rendertargets; }
	CMPINLINE const MaterialCollection &			GetMaterials(void) const { return m_materials; }
	CMPINLINE const PipelineStateCollection &		GetPipelineStates(void) const { return m_pipelinestates; }


	/* Methods to initiate each stage of the deferred rendering process per-frame */
	void											BeginDeferredRenderingFrame(void);
	// ...
	void											EndDeferredRenderinFrame(void);
	
	
	~RenderDeviceDX11(void);
	

private:

	Result											InitialiseExternalShaderResource(ShaderDX11 ** ppOutShader, Shader::Type shadertype, const std::string & fileName, 
														const std::string & entryPoint, const std::string & profile, const InputLayoutDesc *input_layout = NULL);
	
	SamplerStateDX11 *								CreateSamplerState(const std::string & name);
	RenderTargetDX11 *								CreateRenderTarget(void);
	MaterialDX11 *									CreateMaterial(const std::string & name);
	PipelineStateDX11 *								CreatePipelineState(const std::string & name);

private:

	Rendering::RenderDeviceType *			m_device;
	Rendering::RenderDeviceContextType *	m_devicecontext;

	std::string								m_devicename;
	size_t									m_devicememory;
	D3D_DRIVER_TYPE							m_drivertype;		// Hardware (in almost all cases) or WARP (software backup)
	ID3D11Debug *							m_debuglayer;		// Only if required & initialised

	ShaderCollection						m_shaders;
	SamplerStateCollection					m_samplers;
	RenderTargetCollection					m_rendertargets;
	MaterialCollection						m_materials;
	PipelineStateCollection					m_pipelinestates;

	ShaderDX11 *							m_deferred_vs;
	ShaderDX11 *							m_deferred_geometry_ps;

	InputLayoutDesc							m_standard_input_layout;

	SamplerStateDX11 *						m_sampler_linearclamp;
	SamplerStateDX11 *						m_sampler_linearrepeat;


	// We will negotiate the highest possible supported feature level when attempting to initialise the render device
	static const D3D_FEATURE_LEVEL			SUPPORTED_FEATURE_LEVELS[];
};