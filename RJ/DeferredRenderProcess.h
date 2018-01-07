#pragma once

#include "RenderProcessDX11.h"
#include "DeferredGBuffer.h"
#include "Data\Shaders\Common\CommonShaderConstantBufferDefinitions.hlsl.h"
class PipelineStateDX11;
class RenderTargetDX11;

class DeferredRenderProcess : public RenderProcessDX11
{
public:

	DeferredRenderProcess(void);

	// GBuffer holding all deferred rendering data and render targets
	DeferredGBuffer GBuffer;

	// Primary rendering method; executes all deferred rendering operations
	virtual void Render(void);


	DeferredRenderProcess(void);


protected:

	// Primary stages in deferred rendering process
	void PopulateCommonConstantBuffers(void);
	void RenderGeometry(void);
	void PerformDeferredLighting(void);
	void RenderTransparency(void);

	// Retrieve standard buffer data
	ConstantBufferDX11 *							GetCommonFrameDataBuffer(void) { return m_cb_frame; }
	FrameDataBuffer *								GetCommonFrameDataBufferData(void) { return m_cb_frame_data.RawPtr; }
	ConstantBufferDX11 *							GetCommonMaterialBuffer(void) { return m_cb_material; }
	MaterialBuffer *								GetCommonMaterialBufferData(void) { return m_cb_material_data.RawPtr; }

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
	ManagedPtr<MaterialBuffer>				m_cb_material_data;
	ConstantBufferDX11 *					m_cb_material;

	// Indices of required shader parameters
	ShaderDX11::ShaderParameterIndex		m_param_vs_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_geom_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_geom_materialdata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_materialdata;

	// Initialise components of the deferred rendering process
	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);
	void InitialiseGeometryPipelines(void);
	void InitialiseDeferredLightingPipelines(void);
	void InitialiseDeferredLightingPass1Pipeline(void);
	void InitialiseDeferredLightingPass2Pipeline(void);
	void InitialiseDeferredDirectionalLightingPipeline(void);
	void InitialiseTransparentRenderingPipelines(void);



};
