#pragma once

#include "RenderProcessDX11.h"
#include "DeferredGBuffer.h"
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
	void RenderGeometry(void);
	void PerformDeferredLighting(void);
	void RenderTransparency(void);


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

	// Initialise components of the deferred rendering process
	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseGeometryPipelines(void);
	void InitialiseDeferredLightingPipelines(void);
	void InitialiseDeferredLightingPass1Pipeline(void);
	void InitialiseDeferredLightingPass2Pipeline(void);
	void InitialiseDeferredDirectionalLightingPipeline(void);
	void InitialiseTransparentRenderingPipelines(void);



};
