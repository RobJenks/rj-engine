#pragma once

#include "RenderProcessDX11.h"
#include "ManagedPtr.h"
#include "DeferredGBuffer.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"
class PipelineStateDX11;
class RenderTargetDX11;
class Model;

class DeferredRenderProcess : public RenderProcessDX11
{
public:

	DeferredRenderProcess(void);

	// GBuffer holding all deferred rendering data and render targets
	DeferredGBuffer GBuffer;

	// Primary rendering method; executes all deferred rendering operations
	virtual void Render(void);

	// Begin the frame; clear per-frame RTs and other resources ready for rendering
	void BeginFrame(void);

	// Perform all rendering of the frame
	void RenderFrame(void);

	// End the frame, including presentation of swap chain to the primary display
	void EndFrame(void);


	~DeferredRenderProcess(void);


protected:

	// Primary stages in deferred rendering process
	void PopulateCommonConstantBuffers(void);
	void RenderGeometry(void);
	void PerformDeferredLighting(void);
	void RenderTransparency(void);

	// Retrieve standard buffer data
	CMPINLINE ConstantBufferDX11 *					GetCommonFrameDataBuffer(void) { return m_cb_frame; }
	CMPINLINE FrameDataBuffer *						GetCommonFrameDataBufferData(void) { return m_cb_frame_data.RawPtr; }

private:

	// Deferred rendering shaders
	ShaderDX11 *		m_vs;
	ShaderDX11 *		m_ps_geometry;
	ShaderDX11 *		m_ps_lighting;

	// Render pipelines
	PipelineStateDX11 * m_pipeline_geometry;
	PipelineStateDX11 * m_pipeline_lighting_pass1;
	PipelineStateDX11 * m_pipeline_lighting_pass2;
	PipelineStateDX11 *	m_pipeline_lighting_directional;
	PipelineStateDX11 * m_pipeline_transparency;

	// Additional render targets (in addition to the GBuffer and backbuffer itself)
	RenderTargetDX11 *	m_depth_only_rt;

	// Standard constant buffers; keep single instance for binding efficiency
	ManagedPtr<FrameDataBuffer>				m_cb_frame_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_frame;					// Compiled CB
	ManagedPtr<LightIndexBuffer>			m_cb_lightindex_data;		// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_lightindex;			// Compiled CB

	// Model buffers used for rendering light volumes
	Model *									m_model_sphere;
	Model *									m_model_cone;

	// Indices of required shader parameters
	ShaderDX11::ShaderParameterIndex		m_param_vs_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_lightdata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_lightindexdata;

	// Initialise components of the deferred rendering process
	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);
	void InitialiseRenderVolumes(void);
	void InitialiseGeometryPipelines(void);
	void InitialiseDeferredLightingPipelines(void);
	void InitialiseDeferredLightingPass1Pipeline(void);
	void InitialiseDeferredLightingPass2Pipeline(void);
	void InitialiseDeferredDirectionalLightingPipeline(void);
	void InitialiseTransparentRenderingPipelines(void);

	// Bind shader resources required for the deferred lighting stage
	void BindDeferredLightingShaderResources(void);

	// Render a subset of the deferred lighting phase using the given pipeline and light render volume
	void RenderLightPipeline(PipelineStateDX11 *pipeline, Model *light_render_volume);

};
