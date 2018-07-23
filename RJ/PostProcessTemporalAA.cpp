#include <vector>
#include <tuple>
#include "PostProcessTemporalAA.h"
#include "DeferredRenderProcess.h"
#include "Logging.h"
#include "IntVector.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "RenderAssetsDX11.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "PipelineStateDX11.h"
#include "NoiseGenerator.h"

const std::string PostProcessTemporalAA::TX_NAME_REPROJECTION0 = "TemporalAA_Reprojection0_TX";
const std::string PostProcessTemporalAA::TX_NAME_REPROJECTION1 = "TemporalAA_Reprojection1_TX";
const std::string PostProcessTemporalAA::TX_NAME_FINAL = "TemporalAA_Final_TX";
const std::string PostProcessTemporalAA::RT_NAME_0 = "TemporalAA_RT0";
const std::string PostProcessTemporalAA::RT_NAME_1 = "TemporalAA_RT1";
const std::string PostProcessTemporalAA::RP_TEMPORAL = "TemporalAA_Pipeline";

const float PostProcessTemporalAA::DEFAULT_TEMPORAL_FEEDBACK_MIN = 0.88f;
const float PostProcessTemporalAA::DEFAULT_TEMPORAL_FEEDBACK_MAX = 0.97f;

PostProcessTemporalAA::PostProcessTemporalAA(void)
{
	// Only for default construction
}

PostProcessTemporalAA::PostProcessTemporalAA(DeferredRenderProcess * render_process)
	:
	PostProcessComponent("temporal-aa", "Temporal Anti-Aliasing"), 

	m_renderprocess(render_process), 

	m_vs_quad(NULL), 
	m_ps_temporal(NULL), 

	m_tx_reprojection(), 
	m_tx_final(NULL), 
	m_rt(), 
	m_pipeline_temporal(NULL), 

	m_cb_temporal(NULL), 
	m_temporal_feedback_min(DEFAULT_TEMPORAL_FEEDBACK_MIN), 
	m_temporal_feedback_max(DEFAULT_TEMPORAL_FEEDBACK_MAX), 

	m_reprojection_index(INITIAL_REPROJECTION_INDEX), 
	m_frame_reprojection_source(INITIAL_REPROJECTION_INDEX), 
	m_frame_reprojection_target(INITIAL_REPROJECTION_INDEX), 

	m_param_ps_temporal_deferred(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_temporal_buffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_temporal_tex_colour_buffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_temporal_tex_reproj_buffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_temporal_tex_depth_buffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_temporal_tex_velocity_buffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_temporal_tex_motion_buffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_temporal_tex_noise(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_temporal_noise_data(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	m_tx_reprojection[0] = m_tx_reprojection[1] = NULL;
	m_rt[0] = m_rt[1] = NULL;

	// Perform all one-time startup initialisation
	InitialiseShaders();
	InitialiseStandardBuffers();

	// Execute all repeatable initialisation
	PerformPostConfigInitialisation();
}

// Reinitialise based on a change to the effect configuration
void PostProcessTemporalAA::PerformPostConfigInitialisation(void)
{
	InitialiseTextureBuffers();
	InitialiseRenderTargets();
	InitialiseTemporalPipeline();
}

void PostProcessTemporalAA::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising post-process temporal anti-aliasing shaders\n";

	// Get a reference to all required shaders
	m_vs_quad = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::FullScreenQuadVertexShader);
	if (m_vs_quad == NULL) Game::Log << LOG_ERROR << "Cannot load post-process temporal anti-aliasing shader resources [vs_q]\n";

	m_ps_temporal = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::TemporalReprojection);
	if (m_ps_temporal == NULL) Game::Log << LOG_ERROR << "Cannot load post-process temporal anti-aliasing shader resources [ps]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_ps_temporal_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, DeferredRenderingParamBufferName);
	m_param_ps_temporal_buffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, TemporalAABufferName);
	m_param_ps_temporal_tex_colour_buffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, TAAColourBufferInputName);
	m_param_ps_temporal_tex_reproj_buffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, TAAHistoryBufferInputName);
	m_param_ps_temporal_tex_depth_buffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, TAADepthBufferInputName);
	m_param_ps_temporal_tex_velocity_buffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, TAAVelocityBufferInputName);
	m_param_ps_temporal_tex_motion_buffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, TAAMotionBlurFinalInputName);
	m_param_ps_temporal_tex_noise = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, NoiseTextureDataName);
	m_param_ps_temporal_noise_data = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_temporal, NoiseDataBufferName);
}

// Initialise texture buffer resources based upon the current configuration.  Can be called multiple times based on 
// configuration changes, so will also handle release/refresh of existing resources if required
void PostProcessTemporalAA::InitialiseTextureBuffers(void)
{
	Texture::TextureFormat & primary_colour_buffer_format = Game::Engine->GetRenderDevice()->PrimaryRenderTargetColourBufferFormat();
	IntegralVector2<unsigned int> display_size = Game::Engine->GetRenderDevice()->GetDisplaySizeU();

	// Initialise textures; all colour buffers share the same (primary) format and dimensions; no downsampling possible
	std::vector<std::tuple<const std::string, const std::string, TextureDX11**, Texture::TextureFormat&, IntegralVector2<unsigned int>>> textures = {
		{ "reprojection buffer [0]", TX_NAME_REPROJECTION0, &(m_tx_reprojection[0]), primary_colour_buffer_format, display_size },
		{ "reprojection buffer [1]", TX_NAME_REPROJECTION1, &(m_tx_reprojection[1]), primary_colour_buffer_format, display_size },
		{ "final colour target", TX_NAME_FINAL, &(m_tx_final), primary_colour_buffer_format, display_size }
	};

	for (const auto & tex : textures)
	{
		std::string desc = std::get<0>(tex);
		std::string name = std::get<1>(tex);
		TextureDX11 **ppTex = std::get<2>(tex);
		Texture::TextureFormat & format = std::get<3>(tex);
		IntegralVector2<unsigned int> size = std::get<4>(tex);

		// Release any resource that already exists, otherwise log on first creation
		if (Game::Engine->GetAssets().AssetExists<TextureDX11>(name))
		{
			Game::Engine->GetAssets().DeleteAsset<TextureDX11>(name);
		}
		else
		{
			Game::Log << LOG_INFO << "Initialising temporal anti-aliasing " << desc << " texture resource\n";
		}

		// Create the new resource
		*ppTex = Game::Engine->GetAssets().CreateTexture2D(name, size.x, size.y, 1U, format);
		if (!(*ppTex)) Game::Log << LOG_ERROR << "Failed to create temporal anti-aliasing " << desc << " texture resource\n";
	}
}

// Initialise render targets based upon the current configuration.  Can be called multiple times based on 
// configuration changes, so will also handle release/refresh of existing resources if required
void PostProcessTemporalAA::InitialiseRenderTargets(void)
{
	IntegralVector2<unsigned int> display_size = Game::Engine->GetRenderDevice()->GetDisplaySizeU();

	// Initialise render targets.  Two RTs so that we can support read from one reproj texture during write to another, 
	// and to avoid binding & compiling RTs when reproj textures are flipped
	std::vector<std::tuple<std::string, std::string, RenderTargetDX11**, IntegralVector2<int>, std::array<TextureDX11*, 2U>>> rts = {
		{ "render target [0]", RT_NAME_0, &(m_rt[0]), display_size.Convert<int>(), { m_tx_reprojection[0], m_tx_final } },
		{ "render target [1]", RT_NAME_1, &(m_rt[1]), display_size.Convert<int>(), { m_tx_reprojection[1], m_tx_final } }
	};

	for (const auto & rt : rts)
	{
		std::string desc = std::get<0>(rt);
		std::string name = std::get<1>(rt);
		RenderTargetDX11 **ppRT = std::get<2>(rt);
		IntegralVector2<int> size = std::get<3>(rt);
		const std::array<TextureDX11*, 2U> & textures = std::get<4>(rt);

		// Release any resource that already exists, otherwise log on first creation
		if (Game::Engine->GetAssets().AssetExists<RenderTargetDX11>(name))
		{
			Game::Engine->GetAssets().DeleteAsset<RenderTargetDX11>(name);
		}
		else
		{
			Game::Log << LOG_INFO << "Initialising temporal anti-aliasing " << desc << " resource\n";
		}

		*ppRT = Game::Engine->GetAssets().CreateRenderTarget(name, size);
		if (*ppRT)
		{
			(*ppRT)->AttachTexture(RenderTarget::AttachmentPoint::Color0, textures[0]);
			(*ppRT)->AttachTexture(RenderTarget::AttachmentPoint::Color1, textures[1]);
		}
		else
		{
			Game::Log << LOG_ERROR << "Failed to create temporal anti-aliasing " << desc << " resource\n";
		}
	}
}

void PostProcessTemporalAA::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising post-process temporal anti-aliasing standard buffers\n";

	m_cb_temporal = Game::Engine->GetAssets().CreateConstantBuffer(TemporalAABufferName, m_cb_temporal_data.RawPtr);
}

void PostProcessTemporalAA::InitialiseTemporalPipeline(void)
{
	// Recreate existing pipeline state if it already exists
	if (Game::Engine->GetAssets().AssetExists<PipelineStateDX11>(RP_TEMPORAL))
	{
		Game::Engine->GetAssets().DeleteAsset<PipelineStateDX11>(RP_TEMPORAL);
	}
	else
	{
		Game::Log << LOG_INFO << "Initialising post-process temporal anti-aliasing render pipeline\n";
	}

	m_pipeline_temporal = Game::Engine->GetAssets().CreatePipelineState(RP_TEMPORAL);
	if (!m_pipeline_temporal)
	{
		Game::Log << LOG_ERROR << "Failed to initialise post-process temporal anti-aliasing render pipeline\n";
		return;
	}

	// General pipeline configuration
	m_pipeline_temporal->SetShader(Shader::Type::VertexShader, m_vs_quad);
	m_pipeline_temporal->SetShader(Shader::Type::PixelShader, m_ps_temporal);
	m_pipeline_temporal->SetRenderTarget(NULL);		// Will be set per-frame, alternating between RT[0] and [1]

	// Disable all depth/stencil operations and culling; this is full-screen ortho rendering only
	DepthStencilState::DepthMode depthMode(false, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Never);
	m_pipeline_temporal->GetDepthStencilState().SetDepthMode(depthMode);
	m_pipeline_temporal->GetRasterizerState().SetViewport(Game::Engine->GetRenderDevice()->GetPrimaryViewport());
	m_pipeline_temporal->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);

	// Disable all blending operations in the target buffer; target will be empty before the full-screen render anyway
	m_pipeline_temporal->GetBlendState().SetBlendMode(BlendState::BlendModes::NoBlend);
}

// Virtual event which can be handled by subclasses; triggered when component is activated or deactivated
void PostProcessTemporalAA::ActiveStateChanged(bool is_active)
{
	// Reset frame-dependent or temporal parameters if the component is being activated
	if (is_active)
	{
		Reset();
	}
}

// Reset any frame-dependent or temporal parameters, e.g. if the component is being activated
void PostProcessTemporalAA::Reset(void)
{
	m_reprojection_index = INITIAL_REPROJECTION_INDEX;
}


// Execute the post-process over the source buffer.  Returns a pointer to the final buffer
// following post-processing
TextureDX11 * PostProcessTemporalAA::Execute(TextureDX11 *source_colour, TextureDX11 *source_depth, TextureDX11 *source_vel, TextureDX11 *source_motion)
{
	assert(source_colour);
	assert(source_depth);
	assert(source_vel);
	assert(source_motion);

	// All rendering will be against a full-screen quad in orthographic projection space
	m_renderprocess->PopulateFrameBuffer(DeferredRenderProcess::FrameBufferState::Fullscreen);

	/*
		1. Populate the temporal AA buffer with all per-frame parameters
		2. Prepare reprojection render targets for the frame
		3. Temporal reprojection phase
	*/
	PopulateTemporalAABuffer();
	PrepareReprojectionBuffers(source_colour);
	ExecuteTemporalReprojectionPass(source_colour, source_depth, source_vel, source_motion);

	// Return the final result of the post-processing
	return m_tx_final;
}


void PostProcessTemporalAA::PopulateTemporalAABuffer(void)
{
	m_cb_temporal_data.RawPtr->C_NearClip = Game::Engine->GetRenderDevice()->GetNearClipDistance();
	m_cb_temporal_data.RawPtr->C_FarClip = Game::Engine->GetRenderDevice()->GetFarClipDistance();
	m_cb_temporal_data.RawPtr->C_FeedbackMin = m_temporal_feedback_min;
	m_cb_temporal_data.RawPtr->C_FeedbackMax = m_temporal_feedback_max;

	m_cb_temporal->Set(m_cb_temporal_data.RawPtr);
}

void PostProcessTemporalAA::PrepareReprojectionBuffers(TextureDX11 *source_colour)
{
	// If this is the first frame since the component was activated, directly copy the source colour buffer
	// to populate the initial reprojection buffer
	if (m_reprojection_index == INITIAL_REPROJECTION_INDEX)
	{
		m_reprojection_index = 0;
		m_tx_reprojection[m_reprojection_index]->Copy(source_colour);
	}

	// Data will be read from reprojection_index this frame, and written to target_reproj
	m_frame_reprojection_source = m_reprojection_index;
	m_frame_reprojection_target = ((m_reprojection_index + 1) % 2);

	// Bind the relevant render target to the temporal pipeline
	m_pipeline_temporal->SetRenderTarget(m_rt[m_frame_reprojection_target]);

	// Cycle the source reprojection buffer to the current target, ready for next frame
	m_reprojection_index = m_frame_reprojection_target;
}

void PostProcessTemporalAA::ExecuteTemporalReprojectionPass(TextureDX11 *source_colour, TextureDX11 *source_depth, TextureDX11 *source_vel, TextureDX11 *source_motion)
{
	// Populate shader parameters
	auto ps = m_pipeline_temporal->GetShader(Shader::Type::PixelShader);
	ps->SetParameterData(m_param_ps_temporal_deferred, m_renderprocess->GetDeferredRenderingParameterBuffer());
	ps->SetParameterData(m_param_ps_temporal_buffer, m_cb_temporal);

	ps->SetParameterData(m_param_ps_temporal_tex_colour_buffer, source_colour);
	ps->SetParameterData(m_param_ps_temporal_tex_reproj_buffer, m_tx_reprojection[m_frame_reprojection_source]);
	ps->SetParameterData(m_param_ps_temporal_tex_depth_buffer, source_depth);
	ps->SetParameterData(m_param_ps_temporal_tex_velocity_buffer, source_vel);
	ps->SetParameterData(m_param_ps_temporal_tex_motion_buffer, source_motion);

	// We will inherit the noise generator configuration of the parent render process, no need for it to differ here
	auto noise_resource = Game::Engine->GetNoiseGenerator()->GetActiveNoiseResource();
	if (noise_resource) ps->SetParameterData(m_param_ps_temporal_tex_noise, noise_resource);
	ps->SetParameterData(m_param_ps_temporal_noise_data, Game::Engine->GetNoiseGenerator()->GetActiveNoiseBuffer());

	// Bind the pipeline and perform full-screen quad rendering
	m_pipeline_temporal->Bind();
	m_renderprocess->RenderFullScreenQuad();
	m_pipeline_temporal->Unbind();
}

// Response to a change in shader configuration or a reload of shader bytecode
void PostProcessTemporalAA::ShadersReloaded(void)
{
	InitialiseShaders();
}


// Destructor
PostProcessTemporalAA::~PostProcessTemporalAA(void)
{

}


