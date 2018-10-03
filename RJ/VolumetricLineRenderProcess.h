#pragma once

#include "VolumetricLineRenderingCommonData.hlsl.h"
#include "RenderProcessDX11.h"
#include "ManagedPtr.h"
#include "ShaderDX11.h"
class RenderTargetDX11;
class ConstantBufferDX11;
class PipelineStateDX11;

class VolumetricLineRenderProcess : public RenderProcessDX11
{
public:

	// Constructor
	VolumetricLineRenderProcess(void);

	// Virtual render method; must be implemented by all derived render processess
	void					Render(void);

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	void					PerformPostDataLoadInitialisation(void);

	// Response to a change in shader configuration or a reload of shader bytecode
	void					ShadersReloaded(void);

	// Allow redirect of render process output to an alternative render target.  Default is the 
	// primary colour buffer
	void					SetRenderTarget(RenderTargetDX11 *rendertarget);

private:

	// Initialisation methods
	void InitialiseShaders(void);
	void InitialiseStandardBuffers(void);
	void InitialisePipelines(void);
	void InitialiseShaderResourceBindings(void);

	void PopulateFrameBuffer(void);

private:

	// Shaders
	ShaderDX11 * m_vs;
	ShaderDX11 * m_gs;
	ShaderDX11 * m_ps;

	// Shader resources
	ManagedPtr<VolumetricLineRenderingFrameBuffer>	m_cb_framebuffer_data;
	ConstantBufferDX11 *							m_cb_framebuffer;

	// Pipeline resources
	PipelineStateDX11 *					m_pipeline;
	

	// Shader parameter indices
	ShaderDX11::ShaderParameterIndex	m_param_vs_framebuffer;
	ShaderDX11::ShaderParameterIndex	m_param_gs_framebuffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_framebuffer;
	ShaderDX11::ShaderParameterIndex	m_param_ps_depth_texture;

};