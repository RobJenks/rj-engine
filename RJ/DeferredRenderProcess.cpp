#include <algorithm>
#include "DeferredRenderProcess.h"
#include "CoreEngine.h"
#include "LightingManagerObject.h"
#include "RenderDeviceDX11.h"
#include "PipelineStateDX11.h"
#include "ShaderDX11.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilState.h"
#include "BlendState.h"
#include "Model.h"
#include "ShadowManagerComponent.h"
#include "FrustumJitterProcess.h"
#include "PostProcessMotionBlur.h"
#include "PostProcessTemporalAA.h"
#include "CommonShaderConstantBufferDefinitions.hlsl.h"
#include "Data/Shaders/LightDataBuffers.hlsl"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"
#include "Data/Shaders/DeferredRenderingGBuffer.hlsl.h"

/* Info: known problem.  If an object is rendered in the deferred LIGHTING phase with textures assigned in its material, 
   those textures are bound over the top of the existing GBuffer textures (i.e. slots 0-3).  This can cause lighting to 
   only be rendered in a small upper-left corner square corresponding to the dimensions of the material texture.  If it 
   becomes necessary to allow textures light volume models in future, need to ensure the slots for material textures and 
   GBuffer textures do not overlap */


// Initialise static data
const float DeferredRenderProcess::DEBUG_DEFAULT_DEPTH_SCALING_EXPONENT = 6.0f;


// Constructor
DeferredRenderProcess::DeferredRenderProcess(void)
	:
	m_vs{ NULL, NULL },
	m_vs_quad(NULL),
	m_ps_geometry(NULL),
	m_ps_lighting{ NULL, NULL },
	m_ps_debug(NULL),
	m_def_dsv_tx(NULL),
	m_def_dsv_rt(NULL),
	m_light_dsv_tx(NULL),
	m_light_dsv_rt(NULL),
	m_colour_buffer(NULL),
	m_colour_rt(NULL),
	m_cb_frame(NULL),
	m_cb_lightindex(NULL),
	m_cb_deferred(NULL),

	m_pipeline_geometry(NULL),
	m_pipeline_lighting_pass1(NULL),
	m_pipeline_lighting_pass2{ NULL, NULL },
	m_pipeline_lighting_directional{ NULL, NULL },
	m_pipeline_transparency(NULL),
	m_pipeline_debug_rendering(NULL),

	m_param_vs_framedata{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER },
	m_param_vs_light_shadowmap_data(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_geometry_deferreddata(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_light_deferreddata{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER },
	m_param_ps_light_framedata{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER }, 
	m_param_ps_light_lightdata{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER },
	m_param_ps_light_lightindexdata{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER },
	m_param_ps_light_noisetexture{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER },
	m_param_ps_light_noisedata{ ShaderDX11::INVALID_SHADER_PARAMETER, ShaderDX11::INVALID_SHADER_PARAMETER },
	m_param_ps_light_shadowmap(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_light_shadowmap_data(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_debug_debugdata(ShaderDX11::INVALID_SHADER_PARAMETER),

	m_model_sphere(NULL),
	m_model_cone(NULL),
	m_model_quad(NULL),
	m_transform_fullscreen_quad(ID_MATRIX),
	m_transform_fullscreen_quad_farplane(ID_MATRIX),
	m_frame_buffer_state(FrameBufferState::Unknown),

	m_render_noise_method(NoiseGenerator::INVALID_NOISE_RESOURCE),

	m_velocity_k(2U),
	m_exposure(1.0f),
	m_motion_samples(16U),
	m_motion_max_sample_tap_distance(16U),

	m_post_processing_components({ 0 }),

	m_debug_render_active_view_count(0U),
	m_cb_debug(NULL), 
	m_debug_depth_scaling_exponent(DEBUG_DEFAULT_DEPTH_SCALING_EXPONENT), 
	m_debug_output_mode(DEF_DEBUG_RENDER_VIEWS)
{
	SetName( RenderProcess::Name<DeferredRenderProcess>() );

	InitialiseShaders();
	InitialiseRenderTargets();
	InitialiseGBuffer();
	InitialiseStandardBuffers();
	InitialiseGBufferResourceMappings();

	InitialiseGeometryPipelines();
	InitialiseDeferredLightingPipelines();
	InitialiseDeferredDirectionalLightingPipeline();
	InitialiseTransparentRenderingPipelines();
	InitialiseDebugRenderingPipelines();

	InitialiseShadowManager();
	InitialisePostProcessingComponents();
}

// Perform any initialisation that cannot be completed on construction, e.g. because it requires
// data that is read in from disk during the data load process
void DeferredRenderProcess::PerformPostDataLoadInitialisation(void)
{
	Game::Log << LOG_INFO << "Performing post-data load initialisation of deferred render process\n";

	// Can only be performed once model data is read from external data files
	InitialiseRenderVolumes();
	InitialiseRenderingDependencies();
}

// Respond to a change in shader configuration or a reload of shader bytecode
void DeferredRenderProcess::ShadersReloaded(void)
{
	Game::Log << LOG_INFO << "Reinitialising deferred render processes following shader reload\n";

	InitialiseShaders();
	InitialiseGBufferResourceMappings();

	// Also allow all post-processing components to account for the change
	for (auto * post : m_post_processing_components)
	{
		post->ShadersReloaded();
	}
}


void DeferredRenderProcess::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering shaders\n";

	// Get a reference to all required shaders
	m_vs[SM_DISABLED] = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::StandardVertexShader);
	if (m_vs[SM_DISABLED] == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [vs]\n";

	m_vs[SM_ENABLED] = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::StandardVertexShaderShadowMapped);
	if (m_vs[SM_ENABLED] == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [vs_sm]\n";

	m_vs_quad = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::FullScreenQuadVertexShader);
	if (m_vs_quad == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [vs_q]\n";

	m_ps_geometry = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredGeometryPixelShader);
	if (m_ps_geometry == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_g]\n";

	m_ps_lighting[SM_DISABLED] = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredLightingPixelShader);
	if (m_ps_lighting[SM_DISABLED] == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_l]\n";

	m_ps_lighting[SM_ENABLED] = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredLightingPixelShaderShadowMapped);
	if (m_ps_lighting[SM_ENABLED] == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering shader resources [ps_l_sm]\n";

	m_ps_debug = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::DeferredLightingDebug);
	if (m_ps_debug == NULL) Game::Log << LOG_ERROR << "Cannot load deferred rendering debug shader resources [ps_d]\n";

	InitialiseShaderParameterBindings();
}

void DeferredRenderProcess::InitialiseShaderParameterBindings(void)
{
	/* Determine indices into the shader parameter sets */

	/// Geometry-PS
	m_param_ps_geometry_deferreddata = AttemptRetrievalOfShaderParameter(m_ps_geometry, DeferredRenderingParamBufferName); 

	const SM_STATE SM_OPTIONS[2] = { SM_DISABLED, SM_ENABLED };
	for (SM_STATE SM : SM_OPTIONS)
	{
		/// VS
		m_param_vs_framedata[SM] = AttemptRetrievalOfShaderParameter(m_vs[SM], FrameDataBufferName);

		/// Lighting-PS
		m_param_ps_light_deferreddata[SM] = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM], DeferredRenderingParamBufferName);
		m_param_ps_light_framedata[SM] = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM], FrameDataBufferName);
		m_param_ps_light_lightdata[SM] = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM], LightBufferName);
		m_param_ps_light_lightindexdata[SM] = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM], LightIndexBufferName);
		m_param_ps_light_noisetexture[SM] = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM], NoiseTextureDataName);
		m_param_ps_light_noisedata[SM] = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM], NoiseDataBufferName);
	}

	/// SM-specific parameters
	m_param_vs_light_shadowmap_data = AttemptRetrievalOfShaderParameter(m_vs[SM_ENABLED], ShadowMappedLightBufferName);
	m_param_ps_light_shadowmap = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM_ENABLED], ShadowMapTextureName);
	m_param_ps_light_shadowmap_data = AttemptRetrievalOfShaderParameter(m_ps_lighting[SM_ENABLED], ShadowMappedLightBufferName);

#ifdef _DEBUG
	/// Debug
	m_param_ps_debug_debugdata = AttemptRetrievalOfShaderParameter(m_ps_debug, DeferredRendererDebugRenderingDataName);
#endif

}


void DeferredRenderProcess::InitialiseRenderTargets(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering render targets\n";

	UINTVECTOR2 displaysize = Game::Engine->GetRenderDevice()->GetDisplaySize().Convert<UINT>();

	// Depth-only render target will be attached to the primary RT depth/stencil buffer
	m_def_dsv_tx = Game::Engine->GetAssets().CreateTexture2D("Deferred_DSV", displaysize.x, displaysize.y);
	m_def_dsv_rt = Game::Engine->GetRenderDevice()->Assets.CreateRenderTarget("Deferred_DSV", displaysize.Convert<int>());
	m_def_dsv_rt->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, m_def_dsv_tx);

	// Colour render target will contain all colour buffer data; this is generally the primary render output before post-processing
	Texture::TextureFormat primary_colour_buffer_format = Game::Engine->GetRenderDevice()->PrimaryRenderTargetColourBufferFormat();
	m_colour_buffer = Game::Engine->GetRenderDevice()->Assets.CreateTexture2D("Deferred_Colour", displaysize.x, displaysize.y, 1, primary_colour_buffer_format);
	m_colour_rt = Game::Engine->GetRenderDevice()->Assets.CreateRenderTarget("Deferred_Colour", displaysize.Convert<int>());
	m_colour_rt->AttachTexture(RenderTarget::AttachmentPoint::Color0, m_colour_buffer);

	// Light-space DSV for cascaded shadow mapping
	m_light_dsv_tx = Game::Engine->GetAssets().CreateTexture2D("Deferred_Light_DSV", displaysize.x, displaysize.y);
	m_light_dsv_rt = Game::Engine->GetRenderDevice()->Assets.CreateRenderTarget("Deferred_Light_DSV", displaysize.Convert<int>());
	m_light_dsv_rt->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, m_light_dsv_tx);


	// Assert that all objects were created as expected
	std::vector<std::tuple<std::string, void**>> components = { 
		{ "depth-stencil buffer", (void**)&m_light_dsv_tx }, 
		{ "depth-stencil render target", (void**)&m_def_dsv_rt }, 
		{ "primary colour buffer", (void**)&m_colour_buffer }, 
		{ "primary colour render target", (void**)&m_colour_rt }, 
		{ "light-space DSV buffer", (void**)&m_light_dsv_tx }, 
		{ "light-space DSV render target", (void**)&m_light_dsv_rt }
	};

	// Verify each component in turn and report issues
	for (const auto & entry : components)
	{
		if (*(std::get<1>(entry)) == NULL)
		{
			Game::Log << LOG_ERROR << "Deferred renderer failed to initialise deferred " << std::get<0>(entry) << "\n";
		}
		else
		{
			Game::Log << LOG_INFO << "Initialised deferred " << std::get<0>(entry) << "\n";
		}
	}
}

void DeferredRenderProcess::InitialiseGBuffer(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering GBuffer\n";

	// Calling render process (us) must bind the GBuffer target light accumulation buffer on initialisation
	GBuffer.BindToTargetLightAccumulationBuffer(m_colour_buffer);
	Game::Log << LOG_INFO << "Initialised GBuffer light accumulation target buffer bindings\n";
}

void DeferredRenderProcess::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering standard buffer resources\n";

	auto & assets = Game::Engine->GetRenderDevice()->Assets;

	m_cb_frame = assets.CreateConstantBuffer<FrameDataBuffer>(FrameDataBufferName, m_cb_frame_data.RawPtr);
	m_cb_lightindex = assets.CreateConstantBuffer<LightIndexBuffer>(LightIndexBufferName, m_cb_lightindex_data.RawPtr);
	m_cb_deferred = assets.CreateConstantBuffer<DeferredRenderingParamBuffer>(DeferredRenderingParamBufferName, m_cb_deferred_data.RawPtr);
	m_cb_debug = assets.CreateConstantBuffer<DeferredRendererDebugRenderingData>(DeferredRendererDebugRenderingDataName, m_cb_debug_data.RawPtr);
}

void DeferredRenderProcess::InitialiseGBufferResourceMappings(void)
{
	// GBuffer textures will be bound as shader resource view to all lighting (not geometry) pixel shaders
	std::vector<ShaderDX11*> shaders = { m_ps_lighting[SM_DISABLED], m_ps_lighting[SM_ENABLED], m_ps_debug };

	// Mapping from resource names to GBuffer components
	std::vector<std::tuple<std::string, TextureDX11*>> resource_mapping = {
		{ GBufferDiffuseTextureName, GBuffer.DiffuseTexture },
		{ GBufferSpecularTextureName, GBuffer.SpecularTexture },
		{ GBufferNormalTextureName, GBuffer.NormalTexture },
		{ GBufferVelocityTextureName, GBuffer.VelocityTexture }, 
		{ GBufferDepthTextureName, GBuffer.DepthStencilTexture }
	};

	// Map all resources that can be assigned
	for (auto * shader : shaders)
	{
		for (auto & mapping : resource_mapping)
		{
			if (shader->HasParameter(std::get<0>(mapping)))
			{
				size_t index = shader->GetParameterIndexByName(std::get<0>(mapping));
				shader->SetParameterData(index, std::get<1>(mapping));
			}
		}
	}
}

void DeferredRenderProcess::InitialiseRenderVolumes(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering standard render volumes\n";

	// Load all required model geometry
	std::vector<std::tuple<std::string, std::string, Model**>> models = {
		{ "point light sphere volume", "unit_sphere_model", &m_model_sphere },
		{ "spot light cone volume", "spotlight_cone_model", &m_model_cone },
		{ "fullscreen quad", "unit_square_model", &m_model_quad }
	};

	for (auto & model : models)
	{
		Model *m = Model::GetModel(std::get<1>(model));
		if (!m)
		{
			Game::Log << LOG_ERROR << "Could not load " << std::get<0>(model) << " model (\"" << std::get<1>(model) << "\") during deferred render process initialisation\n";
		}

		*(std::get<2>(model)) = m;
	}

	// Also precalculate a fullscreen quad transform for rendering directional lights via the quad model
	auto displaysize = Game::Engine->GetRenderDevice()->GetDisplaySizeF();
	m_transform_fullscreen_quad = CalculateFullScreenQuadRenderingTransform(Game::Engine->GetRenderDevice()->GetDisplaySizeF());
}

// Calculate the transform for full-screen quad rendering with the specified dimensions
XMMATRIX DeferredRenderProcess::CalculateFullScreenQuadRenderingTransform(const XMFLOAT2 & dimensions)
{
	return XMMatrixMultiply(
		XMMatrixScaling(dimensions.x, dimensions.y, 1.0f),
		XMMatrixTranslation(0.0f, 0.0f, 10.0f)	// TODO (SM): Remove z translation?
	);
}

void DeferredRenderProcess::InitialiseRenderingDependencies(void)
{
	// TODO: Shouldn't just hardcode this here
	SetRenderNoiseGeneration("blnoise_hdr_rgba");
}

// Geometry pipeline will render all opaque geomeetry to the GBuffer RT
void DeferredRenderProcess::InitialiseGeometryPipelines(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [g]\n";

	m_pipeline_geometry = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Geometry");
	m_pipeline_geometry->SetShader(Shader::Type::VertexShader, m_vs[SM_DISABLED]);
	m_pipeline_geometry->SetShader(Shader::Type::PixelShader, m_ps_geometry);
	m_pipeline_geometry->SetRenderTarget(GBuffer.RenderTarget);

}

// Multi-stage deferred lighting passes
void DeferredRenderProcess::InitialiseDeferredLightingPipelines(void)
{
	InitialiseDeferredLightingPass1Pipeline();
	InitialiseDeferredLightingPass2Pipeline();
}

// Lighting pass 1: determine lit pixels
void DeferredRenderProcess::InitialiseDeferredLightingPass1Pipeline(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [l1]\n";

	// First pass will only render depth information to an off-screen buffer
	m_pipeline_lighting_pass1 = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Lighting_Pass1");
	m_pipeline_lighting_pass1->SetShader(Shader::Type::VertexShader, m_vs[SM_DISABLED]);
	m_pipeline_lighting_pass1->SetRenderTarget(m_def_dsv_rt);

	// Perform culling of back faces
	m_pipeline_lighting_pass1->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Back);
	m_pipeline_lighting_pass1->GetRasterizerState().SetDepthClipEnabled(true);

	// Enable depth testing (pass if light is behind/greater than geometry depth), disable depth writes
	DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Greater);
	m_pipeline_lighting_pass1->GetDepthStencilState().SetDepthMode(depthMode);

	// Enable stencil operations, decrement 1 on pass
	DepthStencilState::StencilMode stencilMode(true);
	DepthStencilState::FaceOperation faceOperation;
	faceOperation.StencilDepthPass = DepthStencilState::StencilOperation::DecrementClamp;
	stencilMode.StencilReference = 1U;
	stencilMode.FrontFace = faceOperation;
	m_pipeline_lighting_pass1->GetDepthStencilState().SetStencilMode(stencilMode);
}

// Lighting pass 2: render lit pixels
void DeferredRenderProcess::InitialiseDeferredLightingPass2Pipeline(void)
{
	// We will initialise pipelines for both shadow-mapped and normal rendering
	static const SM_STATE SM_OPTIONS[2] = { SM_DISABLED, SM_ENABLED };
	static const std::string PIPELINE_ID[2] = { "l2", "l2_sm" };
	static const std::string PIPELINE_NAME[2] = { "Deferred_Lighting_Pass2", "Deferred_Lighting_Pass2_SM" };

	for (SM_STATE SM : SM_OPTIONS)
	{
		Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [" << PIPELINE_ID[SM] << "]\n";

		// Second pass will render lighting information to the primary RT itself
		m_pipeline_lighting_pass2[SM] = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState(PIPELINE_NAME[SM]);
		m_pipeline_lighting_pass2[SM]->SetShader(Shader::Type::VertexShader, m_vs[SM]);
		m_pipeline_lighting_pass2[SM]->SetShader(Shader::Type::PixelShader, m_ps_lighting[SM]);
		m_pipeline_lighting_pass2[SM]->SetRenderTarget(m_colour_rt);

		// Perform culling of front faces since we want to render only back faces of the light volume
		m_pipeline_lighting_pass2[SM]->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Front);
		m_pipeline_lighting_pass2[SM]->GetRasterizerState().SetDepthClipEnabled(false);

		// All light rendering will be additive
		m_pipeline_lighting_pass2[SM]->GetBlendState().SetBlendMode(BlendState::BlendModes::AdditiveBlend);

		// Enable depth testing (pass if in front of / greater than light volume back faces), disable depth writes
		DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::GreaterOrEqual);
		m_pipeline_lighting_pass2[SM]->GetDepthStencilState().SetDepthMode(depthMode);

		// Enable stencil operations, keep on pass ( == 1, i.e. not decremented/unmarked in pass1)
		DepthStencilState::StencilMode stencilMode(true);
		DepthStencilState::FaceOperation faceOperation;
		faceOperation.StencilFunction = DepthStencilState::CompareFunction::Equal;
		stencilMode.StencilReference = 1U;
		stencilMode.BackFace = faceOperation;
		m_pipeline_lighting_pass2[SM]->GetDepthStencilState().SetStencilMode(stencilMode);
	}
}

// Directional lights can be rendered in a single pass, rather than the default two-pass deferred lighting calculation
void DeferredRenderProcess::InitialiseDeferredDirectionalLightingPipeline(void)
{
	// We will initialise pipelines for both shadow-mapped and normal rendering
	static const SM_STATE SM_OPTIONS[2] = { SM_DISABLED, SM_ENABLED };
	static const std::string PIPELINE_ID[2] = { "ld", "ld_sm" };
	static const std::string PIPELINE_NAME[2] = { "Deferred_Lighting_Directional", "Deferred_Lighting_Directional_SM" };

	for (SM_STATE SM : SM_OPTIONS)
	{
		Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [" << PIPELINE_ID[SM] << "]\n";

		// Directional lighting pass will use a single full-screen quad at the far clip plane.  All pixels 
		// forward of the plane will be lit using the same deferred lighting PS
		m_pipeline_lighting_directional[SM] = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState(PIPELINE_NAME[SM]);
		m_pipeline_lighting_directional[SM]->SetShader(Shader::Type::VertexShader, m_vs[SM]);
		m_pipeline_lighting_directional[SM]->SetShader(Shader::Type::PixelShader, m_ps_lighting[SM]);
		m_pipeline_lighting_directional[SM]->SetRenderTarget(m_colour_rt);
		m_pipeline_lighting_directional[SM]->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Back);
		m_pipeline_lighting_directional[SM]->GetBlendState().SetBlendMode(BlendState::BlendModes::AdditiveBlend);

		// Enable depth testing (pass all pixels in front of the far plane), disable writes to the depth buffer
		DepthStencilState::DepthMode depthMode(true, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Greater);
		m_pipeline_lighting_directional[SM]->GetDepthStencilState().SetDepthMode(depthMode);
	}
	
	// Also initialise the fullscreen quad transform for rendering at the far plane
	// TODO: will not currently react to changes of the far depth plane distance
	auto displaysize = Game::Engine->GetRenderDevice()->GetDisplaySizeF();
	m_transform_fullscreen_quad_farplane = XMMatrixMultiply(
		XMMatrixScaling(displaysize.x, displaysize.y, 1.0f),
		XMMatrixTranslation(0.0f, 0.0f, Game::Engine->GetRenderDevice()->GetFarClipDistance())
	);

}

void DeferredRenderProcess::InitialiseTransparentRenderingPipelines(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [t]\n";

	// Use a standard pipeline state here; nothing specific to deferred rendering
	m_pipeline_transparency = Game::Engine->GetRenderDevice()->Assets.GetPipelineState("Transparency");
	if (!m_pipeline_transparency)
	{
		Game::Log << LOG_ERROR << "Could not initialise deferred lighting transparency pass; pipeline state not found\n";
	}
}

void DeferredRenderProcess::InitialiseDebugRenderingPipelines(void)
{
	// Debug pipeline is only active in debug builds
#ifndef _DEBUG
	Game::Log << LOG_INFO << "Skipping initialisation of deferred rendering pipeline [d]; not required\n";
	return;
#endif

	// Initialise pipeline
	Game::Log << LOG_INFO << "Initialising deferred rendering pipeline [d]\n";

	m_pipeline_debug_rendering = Game::Engine->GetRenderDevice()->Assets.CreatePipelineState("Deferred_Lighting_Debug");
	m_pipeline_debug_rendering->SetShader(Shader::Type::VertexShader, m_vs_quad);
	m_pipeline_debug_rendering->SetShader(Shader::Type::PixelShader, m_ps_debug);
	m_pipeline_debug_rendering->GetDepthStencilState().SetDepthMode(DepthStencilState::DepthMode(false));		// Disable all depth testing
	m_pipeline_debug_rendering->GetRasterizerState().SetCullMode(RasterizerState::CullMode::Back);
	m_pipeline_debug_rendering->SetRenderTarget(m_colour_rt);

	// Also initialise the debug rendering to a default starting state
	SetDebugRenderingState(std::vector<DebugRenderMode>(), DEF_DEBUG_RENDER_VIEWS);

	// Initialise the SRV unbinding resources to avoid repeated allocations at render-time
	m_debug_srv_unbind = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	
}

void DeferredRenderProcess::InitialiseShadowManager(void)
{
	Game::Log << LOG_INFO << "Initialising shadow manager components\n";

	m_shadow_manager = new ShadowManagerComponent(this);
}

void DeferredRenderProcess::InitialisePostProcessingComponents(void)
{
	Game::Log << LOG_INFO << "Initialising deferred rendering post-process components\n";

	// Screen-space pixel neighbourhood motion blur
	m_post_motionblur = new PostProcessMotionBlur(this);
	m_post_temporal_aa = new PostProcessTemporalAA(this);

	// Also maintain a collection of base post-processing components
	m_post_processing_components = 
	{ 
		m_post_motionblur.RawPtr, 
		m_post_temporal_aa.RawPtr 
	};
}

// Primary rendering method; executes all deferred rendering operations
void DeferredRenderProcess::Render(void)
{
	BeginFrame();

	RenderFrame();

	EndFrame();
}


// Begin the frame; clear per-frame RTs and other resources ready for rendering
void DeferredRenderProcess::BeginFrame(void)
{
	/*
		1. Initialise per-frame data
		2. Clear deferred and GBuffer render targets
		3. Delegate to child BeginFrame methods where applicable
	*/

	/* 1. Initialise per-frame data */

	// Reset frame buffer state to ensure re-initialisation at least once at the start of the frame
	PopulateFrameBuffer(FrameBufferState::Unknown);

	/* 2. Clear GBuffer RT */
	GBuffer.RenderTarget->Clear(ClearFlags::All, NULL_FLOAT4, 1.0f, 0U);

	/* 2. Clear all deferred render targets */
	m_colour_rt->Clear(ClearFlags::Colour, NULL_FLOAT4);

	/* 3. Perform shadow mapping frame initialisation */
	m_shadow_manager.RawPtr->BeginFrame();
}

// Perform all rendering of the frame
void DeferredRenderProcess::RenderFrame(void)
{
	/*
		0. Populate deferred rendering parameter buffer
		1. Render all opaque geometry
		2. Copy GBuffer depth/stencil to primary render target
		3a. Lighting pass 1: determine lit pixels (non-directional lights)
		3b. Lighting pass 2: render lit pixels (non-directional lights)
		3c: Lighting: render directional lights
		4. Render transparent objects
		5. Perform all post-processing
		6. [Debug only] If debug GBuffer rendering is enabled, overwrite all primary RT data with the debug output
		...
		N. Copy final prepared render buffer into the primary render target
	*/

	/* 0. Populate deferred rendering parameter buffer */
	PopulateDeferredRenderingParamBuffer();

	/* 1. Render opaque geometry */
	RenderGeometry();

	/* 2. Copy GBuffer depth/stencil to primary render target */
	Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->
		GetTexture(RenderTarget::AttachmentPoint::DepthStencil)->Copy(
			GBuffer.DepthStencilTexture);

	/* 3. Perform deferred lighting */
	PerformDeferredLighting();

	/* 4. Render transparent objects */
	RenderTransparency();

	/* 5. Perform all active post-process rendering and assign the 'final' colour buffer reference */
	PerformPostProcessing();
	

	/* 6. If debug rendering from the GBuffer is enabled, run the debug pipeline and overwrite all primary RT data */
#ifdef _DEBUG
	GBufferDebugRendering();
#endif

	/* N. Copy final prepared colour buffer into the primary render target */
	Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->
		GetTexture(RenderTarget::AttachmentPoint::Color0)->Copy(
			GetFinalColourBuffer());
}

// End the frame; perform any post-render cleanup for the render process
void DeferredRenderProcess::EndFrame(void)
{
	// Notify the core engine that an updated GBuffer is available for other render processes
	Game::Engine->NotifyGBufferUpdate(&GBuffer);
}

// TODO: Should make sure lights are sorted in future, so that directional lights come at the end of the light array, 
// to minimise the number of times we need to perform this state change and switch the frame buffer data
void DeferredRenderProcess::PopulateFrameBuffer(FrameBufferState state)
{
	// We only need to update the buffer data is the desired state has changed
	if (state == m_frame_buffer_state) return;

	// Populate the frame buffer based on the new state
	switch (state)
	{
		case FrameBufferState::Normal:
			PopulateFrameBufferBufferForNormalRendering();
			return;

		case FrameBufferState::Fullscreen:
			PopulateFrameBufferForFullscreenQuadRendering();
			return;

		default:
			SetFrameBufferState(FrameBufferState::Unknown);
			return;
	}
}

void DeferredRenderProcess::PopulateFrameBufferBufferForNormalRendering(void)
{
	// Store the new state
	SetFrameBufferState(FrameBufferState::Normal);

	/* Frame data buffer */

	m_cb_frame_data.RawPtr->View = Game::Engine->GetRenderViewMatrixF();
	m_cb_frame_data.RawPtr->Projection = Game::Engine->GetRenderProjectionMatrixF();
	m_cb_frame_data.RawPtr->ViewProjection = Game::Engine->GetRenderViewProjectionMatrixF();
	m_cb_frame_data.RawPtr->InvView = Game::Engine->GetRenderInverseViewMatrixF();
	m_cb_frame_data.RawPtr->InvProjection = Game::Engine->GetRenderInverseProjectionMatrixF();
	m_cb_frame_data.RawPtr->PriorFrameViewProjection = Game::Engine->GetPriorFrameViewProjectionMatrixF();
	m_cb_frame_data.RawPtr->ScreenDimensions = Game::Engine->GetRenderDevice()->GetDisplaySizeF();

	m_cb_frame_data.RawPtr->ProjectionUnjittered = Game::Engine->GetRenderProjectionMatrixUnjitteredF();
	m_cb_frame_data.RawPtr->PriorFrameViewProjectionUnjittered = Game::Engine->GetPriorFrameViewProjectionMatrixUnjitteredF();

	// Camera-specific, independent of normal/fullscreen rendering
	m_cb_frame_data.RawPtr->CameraInverseViewProjection = Game::Engine->GetRenderInverseViewProjectionMatrixF();

	m_cb_frame->Set(m_cb_frame_data.RawPtr);
}

void DeferredRenderProcess::PopulateFrameBufferForFullscreenQuadRendering(void)
{
	// Store the new state
	SetFrameBufferState(FrameBufferState::Fullscreen);

	// Frame data buffer
	m_cb_frame_data.RawPtr->View = ID_MATRIX_F;															// View matrix == identity
	m_cb_frame_data.RawPtr->Projection = Game::Engine->GetRenderOrthographicMatrixF();					// Proj matrix == orthographic
	m_cb_frame_data.RawPtr->ViewProjection = Game::Engine->GetRenderOrthographicMatrixF();				// ViewProj matrix == (orthographic * ID) == orthographic
	m_cb_frame_data.RawPtr->InvView = ID_MATRIX_F;														// Inv view == inv(ID) == ID
	m_cb_frame_data.RawPtr->InvProjection = Game::Engine->GetRenderInverseOrthographicMatrixF();		// Inv proj == inv orthographic
	m_cb_frame_data.RawPtr->PriorFrameViewProjection = Game::Engine->GetRenderOrthographicMatrixF();	// Prior ViewProj == ThisViewProj == orthographic
	m_cb_frame_data.RawPtr->ScreenDimensions = Game::Engine->GetRenderDevice()->GetDisplaySizeF();

	m_cb_frame_data.RawPtr->ProjectionUnjittered = Game::Engine->GetRenderProjectionMatrixUnjitteredF();
	m_cb_frame_data.RawPtr->PriorFrameViewProjectionUnjittered = Game::Engine->GetPriorFrameViewProjectionMatrixUnjitteredF();

	// Camera-specific, independent of normal/fullscreen rendering
	m_cb_frame_data.RawPtr->CameraInverseViewProjection = Game::Engine->GetRenderInverseViewProjectionMatrixF();

	m_cb_frame->Set(m_cb_frame_data.RawPtr);
}

void DeferredRenderProcess::PopulateDeferredRenderingParamBuffer(void)
{
	auto displaysize = Game::Engine->GetRenderDevice()->GetDisplaySizeU();

	// General data
	m_cb_deferred_data.RawPtr->C_frametime = Game::TimeFactor;
	m_cb_deferred_data.RawPtr->C_buffersize = XMUINT2(displaysize.x, displaysize.y);
	m_cb_deferred_data.RawPtr->C_texelsize = XMFLOAT2(1.0f / static_cast<float>(displaysize.x), 1.0f / static_cast<float>(displaysize.y));

	// Velocity calculation data
	m_cb_deferred_data.RawPtr->C_k = m_velocity_k;
	m_cb_deferred_data.RawPtr->C_half_exposure = (0.5f * m_exposure);
	m_cb_deferred_data.RawPtr->C_half_frame_exposure = (0.5f * (m_exposure / (Game::TimeFactor + Game::C_EPSILON)));
	m_cb_deferred_data.RawPtr->C_motion_samples = m_motion_samples;
	m_cb_deferred_data.RawPtr->C_motion_max_sample_tap_distance = m_motion_max_sample_tap_distance;

	// Frustum jitter for current and prior frame (xy = current frame UV jitter, zw = prior frame UV jitter)
	auto frustum_jitter = Game::Engine->GetRenderDevice()->FrustumJitter();
	if (frustum_jitter->IsEnabled())
	{
		m_cb_deferred_data.RawPtr->C_Jitter = frustum_jitter->GetTwoFrameJitterVectorF();
	}
	else
	{
		m_cb_deferred_data.RawPtr->C_Jitter = NULL_FLOAT4;
	}

	// Commit all changes to the CB
	m_cb_deferred->Set(m_cb_deferred_data.RawPtr);
}

void DeferredRenderProcess::RenderGeometry(void)
{
	// Populate common buffer data
	PopulateFrameBuffer(FrameBufferState::Normal);

	// Bind required buffer resources to shader parameters
	m_pipeline_geometry->GetShader(Shader::Type::VertexShader)->SetParameterData(m_param_vs_framedata[SM_DISABLED], GetCommonFrameDataBuffer());
	m_pipeline_geometry->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_geometry_deferreddata, m_cb_deferred);

	// Bind the entire geometry rendering pipeline, including all shaders, render targets & states
	m_pipeline_geometry->Bind();

	// Render all non-transparent objects
	Game::Engine->ProcessRenderQueue<ShaderRenderPredicate::RenderGeometry, ModelRenderPredicate::RenderNonTransparent>(m_pipeline_geometry);

	// Unbind the geometry rendering pipeline
	// TODO: Avoid bind/unbind/bind/unbind/... ; in future, add more sensible transitions that can eliminate bind(null) calls [for unbinding] in between two normal binds
	m_pipeline_geometry->Unbind();
}

void DeferredRenderProcess::PerformDeferredLighting(void)
{
	// Bind required buffer resources to each pipeline
	BindDeferredLightingShaderResources();

	// Process each light in turn
	unsigned int light_count = static_cast<unsigned int>(Game::Engine->LightingManager->GetLightSourceCount());
	const auto * lights = Game::Engine->LightingManager->GetLightData();

	for (unsigned int i = 0U; i < light_count; ++i)
	{
		// Only process active lights
		const auto & light = lights[i];
		if (!CheckBit_Single(light.Flags, LIGHT_FLAG_ENABLED)) continue;

		// Update the light index buffer
		m_cb_lightindex_data.RawPtr->LightIndex = i;
		m_cb_lightindex->Set(m_cb_lightindex_data.RawPtr);

		// Clear the stencil buffer for rendering of the new light.  Only call for the first
		// pipeline since all three pipelines share the same render target
		m_pipeline_lighting_pass1->GetRenderTarget()->Clear(ClearFlags::Stencil, NULL_FLOAT4, 1.0f, 1);

		// Render based upon light type
		switch (light.Type)
		{
			case LightType::Point:
				RenderPointLight(i, light);
				break;

			case LightType::Spotlight:
				RenderSpotLight(i, light);
				break;

			case LightType::Directional:
				RenderDirectionalLight(i, light);
				break;
		}
	}
}


void DeferredRenderProcess::RenderPointLight(unsigned int light_index, const LightData & light)
{
	PopulateFrameBuffer(FrameBufferState::Normal);

	// Perform shadow mapping and bind SM resources to the pipeline, if applicable
	SM_STATE SM = GetShadowMappingState(light);
	PerformShadowMapping(light_index, light, m_pipeline_lighting_pass2[SM_ENABLED], 
		m_param_vs_light_shadowmap_data, m_param_ps_light_shadowmap, m_param_ps_light_shadowmap_data);

	// Two-pass render pipeline
	XMMATRIX transform = LightSource::CalculatePointLightTransform(light);
	RenderLightPipeline(m_pipeline_lighting_pass1, m_model_sphere, transform);
	RenderLightPipeline(m_pipeline_lighting_pass2[SM], m_model_sphere, transform);
}

void DeferredRenderProcess::RenderSpotLight(unsigned int light_index, const LightData & light)
{
	PopulateFrameBuffer(FrameBufferState::Normal);

	// Perform shadow mapping and bind SM resources to the pipeline, if applicable
	SM_STATE SM = GetShadowMappingState(light);
	PerformShadowMapping(light_index, light, m_pipeline_lighting_pass2[SM_ENABLED], 
		m_param_vs_light_shadowmap_data, m_param_ps_light_shadowmap, m_param_ps_light_shadowmap_data);

	// Two-pass render pipeline
	XMMATRIX transform = LightSource::CalculateSpotlightTransform(light);
	RenderLightPipeline(m_pipeline_lighting_pass1, m_model_cone, transform);
	RenderLightPipeline(m_pipeline_lighting_pass2[SM], m_model_cone, transform);
}

void DeferredRenderProcess::RenderDirectionalLight(unsigned int light_index, const LightData & light)
{
	PopulateFrameBuffer(FrameBufferState::Fullscreen);

	// Perform shadow mapping and bind SM resources to the pipeline, if applicable
	SM_STATE SM = GetShadowMappingState(light);
	PerformShadowMapping(light_index, light, m_pipeline_lighting_directional[SM_ENABLED], 
		m_param_vs_light_shadowmap_data, m_param_ps_light_shadowmap, m_param_ps_light_shadowmap_data);

	// Single-pass render pipeline
	RenderLightPipeline(m_pipeline_lighting_directional[SM], m_model_quad, m_transform_fullscreen_quad_farplane);
}

DeferredRenderProcess::SM_STATE DeferredRenderProcess::GetShadowMappingState(const LightData & light)
{
	return (CheckBit_Single(light.Flags, LIGHT_FLAG_SHADOW_MAP) ? SM_ENABLED : SM_DISABLED);
}

void DeferredRenderProcess::PerformShadowMapping(unsigned int light_index, const LightData & light, PipelineStateDX11 *pipeline, 
												 ShaderDX11::ShaderParameterIndex param_vs_data, ShaderDX11::ShaderParameterIndex param_ps_sm, 
												 ShaderDX11::ShaderParameterIndex param_ps_data)
{
	if (GetShadowMappingState(light) == SM_ENABLED)
	{
		RenderShadowMap(light_index, light);
		BindShadowMapResources(pipeline, param_vs_data, param_ps_sm, param_ps_data);
	}
	else
	{
		UnbindShadowMapResources(pipeline, param_vs_data, param_ps_sm, param_ps_data);
	}
}

void DeferredRenderProcess::RenderShadowMap(unsigned int light_index, const LightData & light)
{
	m_shadow_manager.RawPtr->ExecuteLightSpaceRenderPass(light_index, light);
}

void DeferredRenderProcess::BindShadowMapResources(PipelineStateDX11 *pipeline, ShaderDX11::ShaderParameterIndex param_vs_data, 
												   ShaderDX11::ShaderParameterIndex param_ps_sm, ShaderDX11::ShaderParameterIndex param_ps_data)
{
	assert(pipeline);

	//pipeline->GetShader(Shader::Type::VertexShader)->SetParameterData(param_vs_data, m_shadow_manager.RawPtr->GetShadowMappedLightDataBuffer());
	pipeline->GetShader(Shader::Type::PixelShader)->SetParameterData(param_ps_data, m_shadow_manager.RawPtr->GetShadowMappedLightDataBuffer());
	pipeline->GetShader(Shader::Type::PixelShader)->SetParameterData(param_ps_sm, m_shadow_manager.RawPtr->GetActiveShadowMap());
}

void DeferredRenderProcess::UnbindShadowMapResources(PipelineStateDX11 *pipeline, ShaderDX11::ShaderParameterIndex param_vs_data,
													 ShaderDX11::ShaderParameterIndex param_ps_sm, ShaderDX11::ShaderParameterIndex param_ps_data)
{
	assert(pipeline);

	//pipeline->GetShader(Shader::Type::VertexShader)->SetParameterData(param_vs_data, static_cast<ConstantBufferDX11*>(NULL));
	pipeline->GetShader(Shader::Type::PixelShader)->SetParameterData(param_ps_data, static_cast<ConstantBufferDX11*>(NULL));
	pipeline->GetShader(Shader::Type::PixelShader)->SetParameterData(param_ps_sm, static_cast<TextureDX11*>(NULL));
}


// Bind shader resources required for the deferred lighting stage
void DeferredRenderProcess::BindDeferredLightingShaderResources(void)
{
	// TODO: Required for all pipelines when shader/buffer is the same?  E.g. all pipelines use the same frame data param in the same VS
	// TODO: Required every frame?  Only setting buffer pointer in shader.  May only be required on shader reload in case param indices change

	// Bind for both regular and shadow-mapped pipeline versions
	static const SM_STATE SM_OPTIONS[2] = { SM_DISABLED, SM_ENABLED };

	// Lighting pass 1 is VS-only and does not output fragments
	m_pipeline_lighting_pass1->GetShader(Shader::Type::VertexShader)->SetParameterData(m_param_vs_framedata[SM_DISABLED], GetCommonFrameDataBuffer());

	// Lighting pass 2 and Directional light pipelines both support shadow mapping
	for (SM_STATE SM : SM_OPTIONS)
	{
		// Lighting pass 2 uses both VS and PS
		m_pipeline_lighting_pass2[SM]->GetShader(Shader::Type::VertexShader)->SetParameterData(m_param_vs_framedata[SM], GetCommonFrameDataBuffer());
		m_pipeline_lighting_pass2[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_framedata[SM], GetCommonFrameDataBuffer());
		m_pipeline_lighting_pass2[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_lightindexdata[SM], m_cb_lightindex);
		m_pipeline_lighting_pass2[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_lightdata[SM], Game::Engine->LightingManager->GetLightDataBuffer());
		m_pipeline_lighting_pass2[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_noisedata[SM], Game::Engine->GetNoiseGenerator()->GetActiveNoiseBuffer());

		// Directional lighting pass uses both VS and PS
		m_pipeline_lighting_directional[SM]->GetShader(Shader::Type::VertexShader)->SetParameterData(m_param_vs_framedata[SM], GetCommonFrameDataBuffer());
		m_pipeline_lighting_directional[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_framedata[SM], GetCommonFrameDataBuffer());
		m_pipeline_lighting_directional[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_lightindexdata[SM], m_cb_lightindex);
		m_pipeline_lighting_directional[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_lightdata[SM], Game::Engine->LightingManager->GetLightDataBuffer());
		m_pipeline_lighting_directional[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_noisedata[SM], Game::Engine->GetNoiseGenerator()->GetActiveNoiseBuffer());
	}
	m_pipeline_lighting_pass2[SM_ENABLED]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_shadowmap, static_cast<TextureDX11*>(NULL));			// Set per shadow-casting light
	m_pipeline_lighting_directional[SM_ENABLED]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_shadowmap, static_cast<TextureDX11*>(NULL));	// Set per shadow-casting light

	// Bind the required noise resources to PS lighting shaders
	Game::Engine->GetNoiseGenerator()->BindNoiseResources(m_render_noise_method);
	TextureDX11 *noiseresource = Game::Engine->GetNoiseGenerator()->GetActiveNoiseResource();
	if (noiseresource)
	{
		for (SM_STATE SM : SM_OPTIONS)
		{
			m_pipeline_lighting_pass2[SM]->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_light_noisetexture[SM], noiseresource);
		}
	}
}

// Render a subset of the deferred lighting phase using the given pipeline and light render volume
void DeferredRenderProcess::RenderLightPipeline(PipelineStateDX11 *pipeline, Model *light_render_volume, const FXMMATRIX transform)
{
	// Simply render a single instance of the light volume within the bound pipeline
	// TODO: In general, optimise the sequence of bind/unbind calls; allow transition directly from Bind()->Bind() without Unbind() in the middle where possible
	pipeline->Bind();
	Game::Engine->RenderInstanced(*pipeline, *light_render_volume, NULL, RM_Instance(transform), 1U);
	pipeline->Unbind();
}


// TODO: Add a non-instanced rendering method to the CoreEngine and use it for rendering the single light volumes above

// Perform all transparent object rendering
void DeferredRenderProcess::RenderTransparency(void)
{

}

// Execute all active post-process rendering phases and populate the final deferred rendering output
void DeferredRenderProcess::PerformPostProcessing(void)
{
	// Points to the current latest colour buffer during the post-processing pipeline
	TextureDX11 * buffer = m_colour_buffer;

	/* Post-processing pipeline */

	buffer = ExecutePostProcessMotionBlur(buffer);
	buffer = ExecutePostProcessTemporalAntiAliasing(buffer);

	// Store a pointer to the final, fully-processed buffer
	m_final_colour_buffer = buffer;
}

// Screen-space per-pixel subsampled motion blur
TextureDX11 * DeferredRenderProcess::ExecutePostProcessMotionBlur(TextureDX11 *colour_buffer)
{
	// Leave buffer unmodified if this post-process is not active
	if (!m_post_motionblur.RawPtr->IsActive()) return colour_buffer;

	// Post-process and return the modified colour buffer
	return m_post_motionblur.RawPtr->Execute(colour_buffer, GBuffer.VelocityTexture);
}

// Temporal anti-aliasing with screen-space motion blur contribution
TextureDX11 * DeferredRenderProcess::ExecutePostProcessTemporalAntiAliasing(TextureDX11 *colour_buffer)
{
	// Leave buffer unmodified if this post-process is not active
	if (!m_post_temporal_aa.RawPtr->IsActive()) return colour_buffer;

	// Temporal reprojection will transition to motion blur at high pixel velocities if motion blur is 
	// active, otherwise it will fall back to regular colour buffer data
	TextureDX11 *motion_buffer = (m_post_motionblur.RawPtr->IsActive() ? m_post_motionblur.RawPtr->GetRenderedOutput() : colour_buffer);

	// Post-process and return the modified colour buffer
	return m_post_temporal_aa.RawPtr->Execute(colour_buffer, GBuffer.DepthStencilTexture, GBuffer.VelocityTexture, motion_buffer);
}

// Set the class of render noise generation used during the render process
void DeferredRenderProcess::SetRenderNoiseGeneration(const std::string & code)
{
	m_render_noise_method = Game::Engine->GetNoiseGenerator()->GetResourceID(code);
}

// Returns the list of supported debug render modes, mapped (StringCode -> Mode)
const std::vector<std::pair<std::string, DeferredRenderProcess::DebugRenderMode>> DeferredRenderProcess::SupportedDebugRenderModes = 
{
	{ "diffuse", DebugRenderMode::Diffuse }, 
	{ "specular", DebugRenderMode::Specular }, 
	{ "normal", DebugRenderMode::Normal }, 
	{ "velocity", DebugRenderMode::Velocity }, 
	{ "depth", DebugRenderMode::Depth }, 
	{ "motion_tilegen", DebugRenderMode::MotionBlurTileGen }, 
	{ "motion_neighbourhood", DebugRenderMode::MotionBlurNeighbourhood }, 
	{ "motion_final", DebugRenderMode::MotionBlurFinal }, 
	{ "shadowmap", DebugRenderMode::ShadowMap }, 
	{ "final", DebugRenderMode::Final }
};

// Translate the name of a debug rendering mode to its internal value
DeferredRenderProcess::DebugRenderMode DeferredRenderProcess::TranslateDebugRenderMode(const std::string & mode)
{
	// Collection of supported debug render modes
	const auto & modes = SupportedDebugRenderModes;
	auto type = StrLower(mode);

	// Attempt to match a supported render mode, or disable rendering if not recognised
	auto it = std::find_if(modes.begin(), modes.end(), [&type](const std::pair<std::string, DeferredRenderProcess::DebugRenderMode> & entry)
	{
		return (entry.first == type);
	});

	return (it != modes.end() ? it->second : DebugRenderMode::None);
}

int DeferredRenderProcess::GetHlslDebugMode(DebugRenderMode render_mode) const
{
	switch (render_mode)
	{
		case DebugRenderMode::None:
			return DEF_DEBUG_STATE_DISABLED;

		case DebugRenderMode::Depth:
		case DebugRenderMode::ShadowMap:
			return DEF_DEBUG_STATE_ENABLED_DEPTH;

		case DebugRenderMode::Velocity:
			return DEF_DEBUG_STATE_ENABLED_VELOCITY;

		default:
			return DEF_DEBUG_STATE_ENABLED_NORMAL;
	}
}

TextureDX11 * DeferredRenderProcess::GetDebugTexture(DeferredRenderProcess::DebugRenderMode debug_mode)
{
	switch (debug_mode)
	{
		// GBuffer textures
		case DebugRenderMode::Diffuse:					return GBuffer.DiffuseTexture;
		case DebugRenderMode::Specular:					return GBuffer.SpecularTexture;
		case DebugRenderMode::Normal:					return GBuffer.NormalTexture;
		case DebugRenderMode::Velocity:					return GBuffer.VelocityTexture;
		case DebugRenderMode::Depth:					return GBuffer.DepthStencilTexture;

		// Other textures
		case DebugRenderMode::MotionBlurTileGen:		return (m_post_motionblur.RawPtr ? m_post_motionblur.RawPtr->GetTileGenerationPhaseResult() : NULL);
		case DebugRenderMode::MotionBlurNeighbourhood:	return (m_post_motionblur.RawPtr ? m_post_motionblur.RawPtr->GetNeighbourhoodDeterminationResult() : NULL);
		case DebugRenderMode::MotionBlurFinal:			return (m_post_motionblur.RawPtr ? m_post_motionblur.RawPtr->GetRenderedOutput() : NULL);
		case DebugRenderMode::ShadowMap:				return (m_shadow_manager.RawPtr->GetDebugShadowMapBufferCapture());

		// Final backbuffer output 
		case DebugRenderMode::Final:					return m_final_colour_buffer;

		// Unknown texture
		default:										return NULL;
	}
}

void DeferredRenderProcess::SetDebugRenderingState(const std::vector<DebugRenderMode> & render_modes, unsigned int output_mode)
{
	// Maximum of 16 views (4x4)
	unsigned int viewcount = (std::min)(static_cast<unsigned int>(render_modes.size()), 16U);

	// Store a copy of this data for per-frame updates
	m_debug_render_modes.clear();
	m_debug_render_modes.insert(m_debug_render_modes.begin(), render_modes.begin(), (render_modes.begin() + viewcount));

	m_debug_output_mode = output_mode;

	// Update the debug shader buffer based on this new information
	UpdateDebugRenderingBuffers();
}

void DeferredRenderProcess::UpdateDebugRenderingBuffers(void)
{
	// Set general buffer data
	auto displaysize = Game::Engine->GetRenderDevice()->GetDisplaySizeU();
	size_t viewcount = m_debug_render_modes.size();
	assert(viewcount <= 16U);

	m_cb_debug_data.RawPtr->C_debug_buffer_size = XMUINT2(displaysize.x, displaysize.y);
	m_cb_debug_data.RawPtr->C_debug_view_count = static_cast<unsigned int>(viewcount);
	m_cb_debug_data.RawPtr->C_debug_render_mode = m_debug_output_mode;
	m_cb_debug_data.RawPtr->C_debug_depth_scaling_exponent = GetDebugDepthRenderingExponent();


	// Vector will hold pointers to the relevant texture resources.  Allocate here but reassign resources
	// per-frame in the debug rendering method, since resource locations may change
	m_debug_srvs.clear();
	m_debug_srvs.insert(m_debug_srvs.begin(), viewcount, NULL);

	// Set each CB parameter in turn, or null it out if not specified
	int last_valid_view = -1;
	for (unsigned int i = 0U; i < 16U; ++i)
	{
		if (i >= viewcount || m_debug_render_modes[i] == DebugRenderMode::None)
		{
			m_cb_debug_data.RawPtr->C_debug_view[i].state = DEF_DEBUG_STATE_DISABLED;
		}
		else
		{
			last_valid_view = static_cast<int>(i);
			m_cb_debug_data.RawPtr->C_debug_view[i].state = GetHlslDebugMode(m_debug_render_modes[i]);
		}
	}

	// Store the count of active render views; if > 0, this will indicate that debug rendering is active
	// Adjust the render view count downwards to the earliest subset of valid modes
	m_debug_render_active_view_count = static_cast<unsigned int>(last_valid_view + 1);

	// Compile changes into the debug rendering buffer
	m_cb_debug->Set(m_cb_debug_data.RawPtr);
}

std::vector<DeferredRenderProcess::DebugRenderMode> DeferredRenderProcess::ProcessDebugRenderModeString(const std::vector<std::string> & render_modes, 
																										unsigned int & outDebugRenderType)
{
	std::vector<DebugRenderMode> modes;

	// Default values
	outDebugRenderType = DEF_DEBUG_RENDER_VIEWS;

	// Process each component
	std::for_each(render_modes.begin(), render_modes.end(), [&modes, &outDebugRenderType](const std::string & mode)
	{ 
		if (!mode.empty()) 
		{
			if (mode == "-views") outDebugRenderType = DEF_DEBUG_RENDER_VIEWS;
			else if (mode == "-composite") outDebugRenderType = DEF_DEBUG_RENDER_COMPOSITE;
			else
			{
				modes.push_back(TranslateDebugRenderMode(mode));
			}
		}
	});

	return modes;
}

// Perform debug rendering of GBuffer data, if enabled.  Returns a flag indicating whether debug rendering was performed
bool DeferredRenderProcess::GBufferDebugRendering(void)
{
	// Normal case: exit immediately since debug rendering is not enabled
	if (!DebugRenderingIsEnabled()) return false;

	// Bind the texture(s) to be debug-rendered 
	for (unsigned int i = 0U; i < m_debug_render_active_view_count; ++i)
	{
		TextureDX11 *texture = GetDebugTexture(m_debug_render_modes[i]);
		m_debug_srvs[i] = (texture ? texture->GetBindingSRV() : NULL);
	}
	TextureDX11::BindSRVMultiple(Shader::Type::PixelShader, 0U, m_debug_srvs);

	// Debug views are generated via full-screen rendering
	// TODO: No longer required?
	PopulateFrameBuffer(FrameBufferState::Fullscreen);

	// Bind shader parameters to the debug pipeline
	m_pipeline_debug_rendering->GetShader(Shader::Type::PixelShader)->SetParameterData(m_param_ps_debug_debugdata, m_cb_debug);

	// Bind the debug pipeline
	m_pipeline_debug_rendering->Bind();

	// Render a full-screen quad through the debug pipeline.  Debug texture will be rendered directly to this quad
	RenderFullScreenQuad();

	// Unbind the debug pipeline following rendering
	m_pipeline_debug_rendering->Unbind();

	// Unbind the SRV resources that were directly bound to the PS
	TextureDX11::BindSRVMultiple(Shader::Type::PixelShader, 0U, m_debug_srv_unbind);

	return true;
}

void DeferredRenderProcess::SetDebugDepthRenderingExponent(float exp)
{
	m_debug_depth_scaling_exponent = exp;
	UpdateDebugRenderingBuffers();
}

// Return a pointer to the final colour buffer for this render process.  This may be the immediately-rendered colour
// buffer, the post-processed colour buffer or the debug rendering output, depending on our current state
TextureDX11 * DeferredRenderProcess::GetFinalColourBuffer(void)
{
	// If debug rendering is enabled we redirect to the base colour buffer, which has been overwritten with debug data
#	ifdef _DEBUG
		if (DebugRenderingIsEnabled()) return m_colour_buffer;
#	endif

	// Otherwise, we always return the final fully-processed colour buffer
	return m_final_colour_buffer;
}

// Execute a full-screen quad rendering through the currently-bound pipeline, using the minimal
// screen-space rendering vertex pipeline
void DeferredRenderProcess::RenderFullScreenQuad(void)
{
	Game::Engine->RenderFullScreenQuad();
}

// Virtual inherited method to accept a command from the console
bool DeferredRenderProcess::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "backbuffer_attach" || command.InputCommand == "rt_attach" || command.InputCommand == "rta")
	{
		if (command.Parameter(0) == "-h" || command.Parameter(0) == "-help" || command.Parameter(0) == "-modes")
		{
			std::string modes;
			for (const auto & mode : DeferredRenderProcess::SupportedDebugRenderModes) modes += ((modes.empty() ? "" : ", ") + mode.first);

			command.SetSuccessOutput("Supported debug rendering modes: " + modes);
			return true;
		}

		unsigned int output_mode = DEF_DEBUG_RENDER_VIEWS;
		std::vector<DebugRenderMode> render_modes = ProcessDebugRenderModeString(command.InputParameters, output_mode);
		SetDebugRenderingState(render_modes, output_mode);

		if (!DebugRenderingIsEnabled())
		{
			command.SetSuccessOutput("Disabled debug rendering mode");
		}
		else
		{
			command.SetSuccessOutput(concat("Enabled debug rendering with ")(m_debug_render_active_view_count)(" redirected render target buffers").str());
		}

		return true;
	}
	else if (command.InputCommand == "debug_depth_exp")
	{
		if (command.ParameterCount() >= 1U)
		{
			float exp = command.ParameterAsFloat(0);
			if (exp <= 0.0f) {
				command.SetFailureOutput("Debug depth rendering exponent must be positive and non-zero");
			}
			else {
				SetDebugDepthRenderingExponent(exp);
				command.SetSuccessOutput(concat("Debug depth rendering exponent updated to ")(GetDebugDepthRenderingExponent()).str());
			}
		}
		else
		{
			command.SetSuccessOutput(concat("Debug depth rendering exponent is ")(GetDebugDepthRenderingExponent()).str());
		}
		return true;
	}
	else if (command.InputCommand == "defrend_setnoise")
	{
		SetRenderNoiseGeneration(command.Parameter(0));
		if (m_render_noise_method == NoiseGenerator::INVALID_NOISE_RESOURCE)
		{
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::InvalidNoiseGenerationMethod,
				concat("Invalid noise generation method \"")(command.Parameter(0))("\"; noise disabled").str()); 
		}
		else
		{
			command.SetSuccessOutput(concat("Updated deferred rendering noise generation method to \"")(command.Parameter(0))("\"").str());
		}
		return true;
	}
	else if (command.InputCommand == "defrend_getnoise")
	{
		std::string method = Game::Engine->GetNoiseGenerator()->DetermineNoiseGenerationMethodName(m_render_noise_method);
		command.SetSuccessOutput(concat("Deferred rendering noise generation method = \"")(method.empty() ? "<null>" : method)("\" (")(m_render_noise_method)(")").str());
		return true;
	}
	else if (command.InputCommand == "post")
	{
		return ProcessPostProcessConsoleCommand(command);
	}

	// We did not recognise the command
	return false;
}

bool DeferredRenderProcess::ProcessPostProcessConsoleCommand(GameConsoleCommand & command)
{
	// Commands are of the form "post <component> <command> [params...]"
	// Param[0] = component
	// Param[1] = command
	// Param[2..N-1] = params
	if (command.ParameterCount() < 2)
	{
		command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::ConsoleCommandHasInvalidSyntax,
			"Cannot execute post-process render command; syntax is \"post <component> <command> [params...]\"");
		return true;
	}

	// Attempt to locate a matching component before proceeding
	for (const auto component : m_post_processing_components)
	{
		if (component->GetCode() == command.Parameter(0))
		{
			// Process allowed commands
			if (command.Parameter(1) == "status")
			{
				command.SetSuccessOutput(concat("Post-processing component \"")(component->GetDescription())("\" (\"")(component->GetCode())("\") is ")
					(component->IsActive() ? "enabled" : "disabled").str());
				return true;
			}
			else if (command.Parameter(1) == "enable" || command.Parameter(1) == "disable")
			{
				bool enable = (command.Parameter(1) == "enable");
				if (component->IsActive() == enable)
				{
					command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::PostProcessAlreadyInDesiredState, 
						concat("Post-processing component \"")(component->GetDescription())("\" (\"")
						(component->GetCode())("\") is already ")(enable ? "enabled" : "disabled").str());
					return true;
				}
				else
				{
					component->SetActive(enable);
					command.SetSuccessOutput(concat("Post-processing component \"")(component->GetDescription())("\" (\"")(component->GetCode())("\") is now ")
						(component->IsActive() ? "enabled" : "disabled").str());
					return true;
				}
			}
			else
			{
				command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::PostProcessCommandNotSupported,
					concat("Command \"")(command.Parameter(0))("\" is not supported by post-processing render components").str());
				return true;
			}
		}
	}

	// We could not locate a matching component
	command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::PostProcessComponentDoesNotExist,
		concat("Cannot execute post-process render command; no component exists with code \"")(command.Parameter(0))("\"").str());
	return true;
}

DeferredRenderProcess::~DeferredRenderProcess(void)
{

}

