#pragma once

#include <array>
#include "RenderProcessDX11.h"
#include "ManagedPtr.h"
#include "DeferredGBuffer.h"
#include "NoiseGenerator.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"
#include "Data/Shaders/DeferredRendererDebugRenderingData.hlsl"
#include "LightData.hlsl.h"
#include "ModelInstance.h"
#include "iAcceptsConsoleCommands.h"
class PipelineStateDX11;
class RenderTargetDX11;
class Model;
class ShadowManagerComponent;
class PostProcessComponent;
class PostProcessMotionBlur;
class PostProcessTemporalAA;

class DeferredRenderProcess : public RenderProcessDX11, public iAcceptsConsoleCommands
{
public:

	// Possible debug rendering modes
	enum class DebugRenderMode { None = 0, Diffuse, Specular, Normal, Velocity, Depth, 
								 MotionBlurTileGen, MotionBlurNeighbourhood, MotionBlurFinal, 
								 ShadowMap, 
								 /* ... */						 
								 Final};

	// Default constructor
	DeferredRenderProcess(void);

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	virtual void PerformPostDataLoadInitialisation(void);

	// Response to a change in shader configuration or a reload of shader bytecode
	virtual void ShadersReloaded(void);

	// GBuffer holding all deferred rendering data and render targets
	DeferredGBuffer GBuffer;

	// Primary rendering method; executes all deferred rendering operations
	virtual void Render(void);

	// Begin the frame; clear per-frame RTs and other resources ready for rendering
	void BeginFrame(void);

	// Perform all rendering of the frame
	void RenderFrame(void);

	// End the frame; perform any post-render cleanup for the render process
	void EndFrame(void);

	// Execute a full-screen quad rendering through the currently-bound pipeline, using the minimal
	// screen-space rendering vertex pipeline
	void RenderFullScreenQuad(void);

	// Retrieve standard buffer data
	CMPINLINE ConstantBufferDX11 * GetCommonFrameDataBuffer(void) const { return m_cb_frame; }
	CMPINLINE ConstantBufferDX11 * GetDeferredRenderingParameterBuffer(void) const { return m_cb_deferred; }

	// Populate the frame buffer for the given rendering mode
	enum class FrameBufferState { Unknown = 0, Normal, Fullscreen };
	void PopulateFrameBuffer(FrameBufferState state);

	// Calculate the transform for full-screen quad rendering with the specified dimensions
	XMMATRIX CalculateFullScreenQuadRenderingTransform(const XMFLOAT2 & dimensions);

	// Translate the name of a debug rendering mode to its internal value
	static DebugRenderMode TranslateDebugRenderMode(const std::string & mode);

	// Destructor
	~DeferredRenderProcess(void);


protected:

	// Record which set of data is active in the frame buffer to minimise rebinding and state changes
	FrameBufferState						m_frame_buffer_state;
	CMPINLINE FrameBufferState				GetFrameBufferState(void) const { return m_frame_buffer_state; }
	CMPINLINE void							SetFrameBufferState(FrameBufferState state) { m_frame_buffer_state = state; }

	// Return a pointer to the final colour buffer for this render process.  This may be the immediately-rendered colour
	// buffer, the post-processed colour buffer or the debug rendering output, depending on our current state
	TextureDX11 *							GetFinalColourBuffer(void);

protected:

	// Primary stages in deferred rendering process
	void PopulateFrameBufferBufferForNormalRendering(void);
	void PopulateFrameBufferForFullscreenQuadRendering(void);
	void PopulateDeferredRenderingParamBuffer(void);
	void RenderGeometry(void);
	void PerformDeferredLighting(void);
	void RenderTransparency(void);
	void PerformPostProcessing(void);

	// Post-processing phases
	TextureDX11 *		ExecutePostProcessMotionBlur(TextureDX11 *colour_buffer);
	TextureDX11 *		ExecutePostProcessTemporalAntiAliasing(TextureDX11 *colour_buffer);


	// Retrieve standard buffer data
	CMPINLINE FrameDataBuffer *						GetCommonFrameDataBufferData(void) { return m_cb_frame_data.RawPtr; }


	// Virtual inherited method to accept a command from the console
	bool ProcessConsoleCommand(GameConsoleCommand & command);
	
	bool ProcessPostProcessConsoleCommand(GameConsoleCommand & command);

private:

	// Deferred rendering shaders
	ShaderDX11 *		m_vs;
	ShaderDX11 *		m_vs_quad;
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
	TextureDX11 *		m_def_dsv_tx;			// Depth/stencil texture for view-space deferred rendering
	RenderTargetDX11 *	m_def_dsv_rt;			// RT for view-space deferred rendering with only depth/stencil bindings

	TextureDX11 *		m_light_dsv_tx;			// Depth/stencil texture for light-space rendering
	RenderTargetDX11 *	m_light_dsv_rt;			// RT for light-space rendering with only depth/stencil bindings


	RenderTargetDX11 *	m_colour_rt;			// Colour buffer RT after all render process rendering, but before any post-processing.  Owned by this render process.
	TextureDX11 *		m_colour_buffer;		// Colour buffer after all render process rendering, but before any post-processing.  Owned by this render process.

	// Pointer to the final colour buffer following all rendering and post-processing.  This buffer may be 
	// owned by the render process or it may be owned by the final post-processing component that worked 
	// upon it.  We maintain only a pointer since it will be direct-copied to the backbuffer immediately 
	// before presentation 
	TextureDX11 *		m_final_colour_buffer;

	// Standard constant buffers; keep single instance for binding efficiency
	ManagedPtr<FrameDataBuffer>					m_cb_frame_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *						m_cb_frame;					// Compiled CB
	ManagedPtr<LightIndexBuffer>				m_cb_lightindex_data;		// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *						m_cb_lightindex;			// Compiled CB
	ManagedPtr<DeferredRenderingParamBuffer>	m_cb_deferred_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *						m_cb_deferred;				// Compiled CB

	// Shadow mapping components
	ManagedPtr<ShadowManagerComponent>			m_shadow_manager;

	// Post-processing components
	ManagedPtr<PostProcessMotionBlur>			m_post_motionblur;
	ManagedPtr<PostProcessTemporalAA>			m_post_temporal_aa;
	/* ... */
	std::array<PostProcessComponent*, 2U>		m_post_processing_components;

	// Model buffers used for rendering light volumes
	Model *									m_model_sphere;
	Model *									m_model_cone;
	Model *									m_model_quad;
	XMMATRIX								m_transform_fullscreen_quad;
	XMMATRIX								m_transform_fullscreen_quad_farplane;

	// ID of the render noise generation method used in this render process
	NoiseGenerator::NoiseResourceID			m_render_noise_method;

	// Velocity calculation parameters
	unsigned int							m_velocity_k;
	float									m_exposure;
	unsigned int							m_motion_samples;
	unsigned int							m_motion_max_sample_tap_distance;

	// Indices of required shader parameters
	ShaderDX11::ShaderParameterIndex		m_param_vs_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_geometry_deferreddata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_deferreddata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_framedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_lightdata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_lightindexdata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_noisetexture;
	ShaderDX11::ShaderParameterIndex		m_param_ps_light_noisedata;
	ShaderDX11::ShaderParameterIndex		m_param_ps_debug_debugdata;
	
	// Initialise components of the deferred rendering process
	void InitialiseShaders(void);
	void InitialiseRenderTargets(void);
	void InitialiseGBuffer(void); 
	void InitialiseStandardBuffers(void);
	void InitialiseGBufferResourceMappings(void);
	void InitialiseRenderVolumes(void);
	void InitialiseRenderingDependencies(void);
	void InitialiseGeometryPipelines(void);
	void InitialiseDeferredLightingPipelines(void);
	void InitialiseDeferredLightingPass1Pipeline(void);
	void InitialiseDeferredLightingPass2Pipeline(void);
	void InitialiseDeferredDirectionalLightingPipeline(void);
	void InitialiseTransparentRenderingPipelines(void);
	void InitialiseDebugRenderingPipelines(void);
	void InitialiseShadowManager(void);
	void InitialisePostProcessingComponents(void);

	// Bind shader resources required for the deferred lighting stage
	void BindDeferredLightingShaderResources(void);

	// Render a subset of the deferred lighting phase using the given pipeline and light render volume
	void RenderLightPipeline(PipelineStateDX11 *pipeline, Model *light_render_volume, const FXMMATRIX transform);

	// Set the class of render noise generation used during the render process
	void SetRenderNoiseGeneration(const std::string & code);

	// Perform debug rendering of GBuffer data, if enabled.  Returns a flag indicating whether debug rendering was performed
	bool											GBufferDebugRendering(void);
	TextureDX11 *									GetDebugTexture(DeferredRenderProcess::DebugRenderMode debug_mode);
	int												GetHlslDebugMode(DebugRenderMode render_mode) const;
	void											SetDebugRenderingState(const std::vector<DebugRenderMode> & render_modes, unsigned int output_mode);
	bool											DebugRenderingIsEnabled(void) const { return (m_debug_render_active_view_count != 0U); }
	std::vector<DebugRenderMode>					ProcessDebugRenderModeString(const std::vector<std::string> & render_modes, unsigned int & outDebugRenderType);
	
	
	// Returns the list of supported debug render modes, mapped (StringCode -> Mode)
	static const std::vector<std::pair<std::string, DeferredRenderProcess::DebugRenderMode>> SupportedDebugRenderModes;
	

	unsigned int									m_debug_render_active_view_count;
	std::vector<DebugRenderMode>					m_debug_render_modes;
	std::vector<ID3D11ShaderResourceView*>			m_debug_srvs;
	std::vector<ID3D11ShaderResourceView*>			m_debug_srv_unbind;
	ManagedPtr<DeferredRendererDebugRenderingData>	m_cb_debug_data;
	ConstantBufferDX11 *							m_cb_debug;
	
};
