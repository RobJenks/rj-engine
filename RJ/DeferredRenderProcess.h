#pragma once

#include "RenderProcessDX11.h"
#include "ManagedPtr.h"
#include "DeferredGBuffer.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"
#include "LightData.hlsl.h"
#include "ModelInstance.h"
class PipelineStateDX11;
class RenderTargetDX11;
class Model;

class DeferredRenderProcess : public RenderProcessDX11
{
public:

	// Possible debug rendering modes
	enum class DebugRenderMode { None = 0, Diffuse = 1, Specular = 2, Normal = 4, Depth = 8 };

	// Default constructor
	DeferredRenderProcess(void);

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	virtual void PerformPostDataLoadInitialisation(void);

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

	// Redirect an alternative render output to the primary render target Color0, and ultimately the backbuffer
	bool RepointBackbufferRenderTargetAttachment(const std::string & target);

	// Destructor
	~DeferredRenderProcess(void);


protected:

	// Record which set of data is active in the frame buffer to minimise rebinding and state changes
	enum class FrameBufferState				{ Unknown = 0, Normal, Fullscreen };
	FrameBufferState						m_frame_buffer_state;
	CMPINLINE FrameBufferState				GetFrameBufferState(void) const { return m_frame_buffer_state; }
	CMPINLINE void							SetFrameBufferState(FrameBufferState state) { m_frame_buffer_state = state; }


protected:

	// Primary stages in deferred rendering process
	void PopulateFrameBuffer(FrameBufferState state);
	void PopulateFrameBufferBufferForNormalRendering(void);
	void PopulateFrameBufferForFullscreenQuadRendering(void);
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
	ShaderDX11 *		m_ps_debug;

	// Render pipelines
	PipelineStateDX11 * m_pipeline_geometry;
	PipelineStateDX11 * m_pipeline_lighting_pass1;
	PipelineStateDX11 * m_pipeline_lighting_pass2;
	PipelineStateDX11 *	m_pipeline_lighting_directional;
	PipelineStateDX11 * m_pipeline_transparency;
	PipelineStateDX11 * m_pipeline_debug_rendering;

	// Additional render targets (in addition to the GBuffer and backbuffer itself)
	RenderTargetDX11 *	m_depth_only_rt;

	// Standard constant buffers; keep single instance for binding efficiency
	ManagedPtr<FrameDataBuffer>				m_cb_frame_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_frame;					// Compiled CB
	ManagedPtr<LightIndexBuffer>			m_cb_lightindex_data;		// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_lightindex;			// Compiled CB
	ManagedPtr<DeferredDebugBuffer>			m_cb_debug_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *					m_cb_debug;					// Compiled CB

	// Model buffers used for rendering light volumes
	Model *									m_model_sphere;
	Model *									m_model_cone;
	Model *									m_model_quad;
	XMMATRIX								m_transform_fullscreen_quad;
	XMMATRIX								m_transform_fullscreen_quad_farplane;

	// Indices of required shader parameters
	ShaderDX11::ShaderParameterIndex		m_param_vs_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_lightdata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_lightindexdata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_debug_debugdata;
	
	// Initialise components of the deferred rendering process
	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseStandardBuffers(void);
	void InitialiseGBufferResourceMappings(void);
	void InitialiseRenderVolumes(void);
	void InitialiseGeometryPipelines(void);
	void InitialiseDeferredLightingPipelines(void);
	void InitialiseDeferredLightingPass1Pipeline(void);
	void InitialiseDeferredLightingPass2Pipeline(void);
	void InitialiseDeferredDirectionalLightingPipeline(void);
	void InitialiseTransparentRenderingPipelines(void);
	void InitialiseDebugRenderingPipelines(void);

	// Bind shader resources required for the deferred lighting stage
	void BindDeferredLightingShaderResources(void);

	// Render a subset of the deferred lighting phase using the given pipeline and light render volume
	void RenderLightPipeline(PipelineStateDX11 *pipeline, Model *light_render_volume, const FXMMATRIX transform);


	// Perform debug rendering of GBuffer data, if enabled.  Returns a flag indicating whether debug rendering was performed
	bool GBufferDebugRendering(void);
	TextureDX11 * GetDebugTexture(DeferredRenderProcess::DebugRenderMode debug_mode);
	DebugRenderMode m_debug_render_mode;
	
};
