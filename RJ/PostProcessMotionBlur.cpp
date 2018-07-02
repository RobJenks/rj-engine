#include <tuple>
#include <vector>
#include "PostProcessMotionBlur.h"
#include "DeferredRenderProcess.h"
#include "DeferredGBuffer.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "RenderProcessDX11.h"
#include "NoiseGenerator.h"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"
#include "Data/Shaders/DeferredRenderingGBuffer.hlsl.h"
#include "Data/Shaders/motion_blur_resources.hlsl"
#include "Data/Shaders/noise_buffers.hlsl.h"

const std::string PostProcessMotionBlur::RT_NAME_TILEGEN = "MotionBlur_Tilegen_RT";
const std::string PostProcessMotionBlur::RT_NAME_NEIGHBOUR = "MotionBlur_Neighbour_RT";
const std::string PostProcessMotionBlur::RT_NAME_GATHER = "MotionBlur_Gather_RT";
const std::string PostProcessMotionBlur::TX_NAME_TILEGEN = "MotionBlur_Tilegen_TX";
const std::string PostProcessMotionBlur::TX_NAME_NEIGHBOUR = "MotionBlur_Neighbour_TX";
const std::string PostProcessMotionBlur::TX_NAME_GATHER = "MotionBlur_Gather_TX";
const std::string PostProcessMotionBlur::RP_NAME_TILEGEN = "MotionBlur_Tilegen_Pipeline";
const std::string PostProcessMotionBlur::RP_NAME_NEIGHBOUR = "MotionBlur_Neighbour_Pipeline";
const std::string PostProcessMotionBlur::RP_NAME_GATHER = "MotionBlur_Gather_Pipeline";


PostProcessMotionBlur::PostProcessMotionBlur(void)
{
	// Default constructor only provided to allow initial ptr creation with deferred initialisation
}

PostProcessMotionBlur::PostProcessMotionBlur(DeferredRenderProcess * render_process)
	:
	m_renderprocess(render_process), 

	m_vs_quad(NULL), 
	m_ps_tilegen(NULL), 
	m_ps_neighbourhood(NULL), 
	m_ps_gather(NULL), 

	m_rt_tilegen(NULL), 
	m_rt_neighbour(NULL), 
	m_rt_gather(NULL), 

	m_tx_tilegen(NULL), 
	m_tx_neighbour(NULL), 
	m_tx_gather(NULL), 

	m_pipeline_tilegen(NULL), 
	m_pipeline_neighbour(NULL), 
	m_pipeline_gather(NULL), 

	m_downsampled_fullscreen_transform(ID_MATRIX), 

	m_param_ps_tilegen_deferred(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_tilgen_velocitybuffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_neighbour_deferred(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_neighbour_tilebuffer(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_gather_deferred(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_gather_colour(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_gather_depth(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_gather_velocity(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_gather_vel_neighbourhood(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_gather_noise_tex(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_gather_noise_data(ShaderDX11::INVALID_SHADER_PARAMETER)
{
	assert(render_process != NULL);

	// Perform startup initialisation
	InitialiseShaders();
	InitialiseStandardBuffers();

	// Set default configuration
	SetTileScalingFactor(PostProcessMotionBlur::DEFAULT_TILE_SCALING_FACTOR);

	// Perform config-dependent initialisation
	PerformPostConfigInitialisation();
}

// Reinitialise based on a change to the effect configuration
void PostProcessMotionBlur::PerformPostConfigInitialisation(void)
{
	Game::Log << LOG_INFO << "Initialising post-process motion blur config-dependent assets\n";

	InitialiseRenderTargets();
	InitialiseRenderGeometry();

	InitialiseTileGenerationPipeline();
	InitialiseNeighbourhoodCalculationPipeline();
	InitialiseGatherPhasePipeline();
}

void PostProcessMotionBlur::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising post-process motion blur shaders\n";

	// Get a reference to all required shaders
	m_vs_quad = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::FullScreenQuadVertexShader);
	if (m_vs_quad == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [vs_q]\n";

	m_ps_tilegen = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::MotionBlurTileGen);
	if (m_ps_tilegen == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [ps_t]\n";

	m_ps_neighbourhood = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::MotionBlurNeighbourhood);
	if (m_ps_neighbourhood == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [ps_n]\n";

	m_ps_gather = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::MotionBlurGather);
	if (m_ps_gather == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [ps_g]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_ps_tilegen_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_tilegen, DeferredRenderingParamBufferName);
	m_param_ps_tilgen_velocitybuffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_tilegen, MotionBlurVelocityBufferInputName);
	m_param_ps_neighbour_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_neighbourhood, DeferredRenderingParamBufferName);
	m_param_ps_neighbour_tilebuffer = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_neighbourhood, MotionBlurVelocityTileBufferInputName);
	m_param_ps_gather_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, DeferredRenderingParamBufferName);
	m_param_ps_gather_colour = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, MotionBlurColourBufferInputName);
	m_param_ps_gather_depth = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, MotionBlurDepthBufferInputName);
	m_param_ps_gather_velocity = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, MotionBlurVelocityBufferInputName);
	m_param_ps_gather_vel_neighbourhood = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, MotionBlurVelocityNeighbourhoodInputName);
	m_param_ps_gather_noise_tex = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, NoiseTextureDataName);
	m_param_ps_gather_noise_data = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, NoiseDataBufferName);
}

// Initialise render targets based upon the current configuration.  Can be called multiple times based on 
// configuration changes, so will also handle release/refresh of existing resources if required
void PostProcessMotionBlur::InitialiseRenderTargets(void)
{
	// Downsampled texture format for intermediate render stages (R8G8_UNORM)
	Texture::TextureFormat format_downsampled(Texture::Components::RG, Texture::Type::UnsignedNormalized, RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT, 8, 8, 0, 0, 0, 0);

	// Description, RT name, ppRT, Tex name, ppTex, TexFormat, Size
	const std::vector<std::tuple<std::string, std::string, RenderTargetDX11**, std::string, TextureDX11**, Texture::TextureFormat, IntegralVector2<unsigned int>>> rtconfig = {
		{ "tile generation", RT_NAME_TILEGEN, &m_rt_tilegen, TX_NAME_TILEGEN, &m_tx_tilegen, format_downsampled, m_tiled_dimensions }, 
		{ "neighbourhood calculation", RT_NAME_NEIGHBOUR, &m_rt_neighbour, TX_NAME_NEIGHBOUR, &m_tx_neighbour, format_downsampled, m_tiled_dimensions }, 
		{ "gather phase", RT_NAME_GATHER, &m_rt_gather, TX_NAME_GATHER, &m_tx_gather, 
			Game::Engine->GetRenderDevice()->PrimaryRenderTargetColourBufferFormat(), Game::Engine->GetRenderDevice()->GetDisplaySizeU() }
	};

	// Release (if applicable) and create RTs & resources based on this config
	for (const auto & entry : rtconfig)
	{
		const std::string & desc = std::get<0>(entry);
		const std::string & rtname = std::get<1>(entry);
		RenderTargetDX11 **ppRT = std::get<2>(entry);
		const std::string & txname = std::get<3>(entry);
		TextureDX11 ** ppTex = std::get<4>(entry);
		const Texture::TextureFormat & txformat = std::get<5>(entry);
		IntegralVector2<unsigned int> size = std::get<6>(entry);

		// Texture resource
		if (Game::Engine->GetAssets().AssetExists<TextureDX11>(txname))
		{
			Game::Engine->GetAssets().DeleteAsset<TextureDX11>(txname);
		}
		else
		{
			Game::Log << LOG_INFO << "Initialising motion blur " << desc << " texture resource\n";	// Only log on application startup, not re-initialisation
		}

		*ppTex = Game::Engine->GetAssets().CreateTexture2D(txname, size.x, size.y, 1U, txformat);
		if (!(*ppTex)) Game::Log << LOG_ERROR << "Failed to create motion blur " << desc << " texture resource\n";

		// Render target
		if (Game::Engine->GetAssets().AssetExists<RenderTargetDX11>(rtname))
		{
			Game::Engine->GetAssets().DeleteAsset<RenderTargetDX11>(rtname);
		}
		else
		{
			Game::Log << LOG_INFO << "Initialising motion blur " << desc << " render target\n";		// Only log on application startup, not re-initialisation
		}

		*ppRT = Game::Engine->GetAssets().CreateRenderTarget(rtname, size.Convert<int>());
		if (*ppRT)
		{
			(*ppRT)->AttachTexture(RenderTarget::AttachmentPoint::Color0, *ppTex);
		}
		else
		{
			Game::Log << LOG_ERROR << "Failed to create motion blur " << desc << " render target\n";
		}
	}
}

void PostProcessMotionBlur::InitialiseRenderGeometry(void)
{
	// Calculate full-screen quad rendering transform for the downsampled render targets
	m_downsampled_fullscreen_transform = m_renderprocess->CalculateFullScreenQuadRenderingTransform(m_tiled_dimensions.ToFloat());

}

void PostProcessMotionBlur::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising post-process motion blur standard buffers\n";

}

void PostProcessMotionBlur::InitialiseTileGenerationPipeline(void)
{
	// Recreate existing pipeline state if it exists
	if (Game::Engine->GetAssets().AssetExists<PipelineStateDX11>(RP_NAME_TILEGEN))
	{
		Game::Engine->GetAssets().DeleteAsset<PipelineStateDX11>(RP_NAME_TILEGEN);
	}
	else
	{
		Game::Log << LOG_INFO << "Initialising motion blur render pipeline [t]\n";		// Log on startup initialisation only
	}

	m_pipeline_tilegen = Game::Engine->GetAssets().CreatePipelineState(RP_NAME_TILEGEN);
	if (!m_pipeline_tilegen)
	{
		Game::Log << LOG_ERROR << "Failed to initialise motion blur tile generation render pipeline\n";
		return;
	}

	// Pipeline configuration
	m_pipeline_tilegen->SetShader(Shader::Type::VertexShader, m_vs_quad);
	m_pipeline_tilegen->SetShader(Shader::Type::PixelShader, m_ps_tilegen);
	m_pipeline_tilegen->SetRenderTarget(m_rt_tilegen);

	// Disable all depth/stencil operations
	DepthStencilState::DepthMode depthMode(false, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Never);
	m_pipeline_tilegen->GetDepthStencilState().SetDepthMode(depthMode);

	// Disable all culling and update raster state for downsampled rendering
	m_pipeline_tilegen->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline_tilegen->GetRasterizerState().SetViewport(m_downsampled_viewport);
	m_pipeline_tilegen->GetRasterizerState().SetMultisampleEnabled(true);

	// Disable all blending operations in the target buffer
	m_pipeline_tilegen->GetBlendState().SetBlendMode(BlendState::BlendModes::NoBlend);

}

void PostProcessMotionBlur::InitialiseNeighbourhoodCalculationPipeline(void)
{
	// Recreate existing pipeline state if it exists
	if (Game::Engine->GetAssets().AssetExists<PipelineStateDX11>(RP_NAME_NEIGHBOUR))
	{
		Game::Engine->GetAssets().DeleteAsset<PipelineStateDX11>(RP_NAME_NEIGHBOUR);
	}
	else
	{
		Game::Log << LOG_INFO << "Initialising motion blur render pipeline [n]\n";		// Log on startup initialisation only
	}

	m_pipeline_neighbour = Game::Engine->GetAssets().CreatePipelineState(RP_NAME_NEIGHBOUR);
	if (!m_pipeline_neighbour)
	{
		Game::Log << LOG_ERROR << "Failed to initialise motion blur neighbour determination render pipeline\n";
		return;
	}

	// Pipeline configuration
	m_pipeline_neighbour->SetShader(Shader::Type::VertexShader, m_vs_quad);
	m_pipeline_neighbour->SetShader(Shader::Type::PixelShader, m_ps_neighbourhood);
	m_pipeline_neighbour->SetRenderTarget(m_rt_neighbour);

	// Disable all depth/stencil operations
	DepthStencilState::DepthMode depthMode(false, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Never);
	m_pipeline_neighbour->GetDepthStencilState().SetDepthMode(depthMode);

	// Disable all culling and update raster state for downsampled rendering
	m_pipeline_neighbour->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline_neighbour->GetRasterizerState().SetViewport(m_downsampled_viewport);
	m_pipeline_neighbour->GetRasterizerState().SetMultisampleEnabled(true);

	// Disable all blending operations in the target buffer
	m_pipeline_neighbour->GetBlendState().SetBlendMode(BlendState::BlendModes::NoBlend);
}

void PostProcessMotionBlur::InitialiseGatherPhasePipeline(void)
{
	// Recreate existing pipeline state if it exists
	if (Game::Engine->GetAssets().AssetExists<PipelineStateDX11>(RP_NAME_GATHER))
	{
		Game::Engine->GetAssets().DeleteAsset<PipelineStateDX11>(RP_NAME_GATHER);
	}
	else
	{
		Game::Log << LOG_INFO << "Initialising motion blur render pipeline [g]\n";		// Log on startup initialisation only
	}

	m_pipeline_gather = Game::Engine->GetAssets().CreatePipelineState(RP_NAME_GATHER);
	if (!m_pipeline_gather)
	{
		Game::Log << LOG_ERROR << "Failed to initialise motion blur gather phase render pipeline\n";
		return;
	}

	// Pipeline configuration
	m_pipeline_gather->SetShader(Shader::Type::VertexShader, m_vs_quad);
	m_pipeline_gather->SetShader(Shader::Type::PixelShader, m_ps_gather);
	m_pipeline_gather->SetRenderTarget(m_rt_gather);

	// Disable all depth/stencil operations
	DepthStencilState::DepthMode depthMode(false, DepthStencilState::DepthWrite::Disable, DepthStencilState::CompareFunction::Never);
	m_pipeline_gather->GetDepthStencilState().SetDepthMode(depthMode);

	// Upsample back to backbuffer resolution for the final gather phase
	m_pipeline_gather->GetRasterizerState().SetViewport(Game::Engine->GetRenderDevice()->GetPrimaryViewport());
	m_pipeline_gather->GetRasterizerState().SetCullMode(RasterizerState::CullMode::None);
	m_pipeline_gather->GetRasterizerState().SetMultisampleEnabled(true);

	// Disable all blending operations in the target buffer
	m_pipeline_gather->GetBlendState().SetBlendMode(BlendState::BlendModes::NoBlend);
}


// Tiled dimensions (per-dimension tile count, K) for this postprocess
void PostProcessMotionBlur::SetTileScalingFactor(unsigned int K)
{
	m_tilesize_k = max(1U, K);

	m_tiled_dimensions = IntegralVector2<unsigned int>(
		static_cast<unsigned int>(Game::ScreenWidth) / m_tilesize_k,
		static_cast<unsigned int>(Game::ScreenHeight) / m_tilesize_k
	);

	m_downsampled_viewport = Viewport(0.0f, 0.0f, static_cast<float>(m_tiled_dimensions.x), static_cast<float>(m_tiled_dimensions.y));
}

// Execute the post-process over the source buffer.  Returns a pointer to the final buffer
// following post-processing
TextureDX11 * PostProcessMotionBlur::Execute(TextureDX11 *source_colour, TextureDX11 *source_vel)
{
	assert(source_colour);
	assert(source_vel);

	// All rendering will be against a full-screen quad in orthographic projection space
	m_renderprocess->PopulateFrameBuffer(DeferredRenderProcess::FrameBufferState::Fullscreen);

	/* 
		1. Velocity-space tile generation
		2. Velocity-space neighbourhood determination
		3. Colour sampling and gather to final output
	*/
	ExecuteTileGenerationPass(source_vel);
	ExecuteNeighbourhoodDeterminationPass(m_tx_tilegen);
	ExecuteGatherPass(source_colour, m_renderprocess->GBuffer.DepthStencilTexture, source_vel, m_tx_neighbour);

	// Return the final result of the post-processing
	return NULL;
}

// 1. Velocity-space tile generation
void PostProcessMotionBlur::ExecuteTileGenerationPass(TextureDX11 *source_vel)
{
	assert(source_vel);

	// Populate shader parameters
	m_pipeline_tilegen->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_tilgen_velocitybuffer).Set(source_vel);
	m_pipeline_tilegen->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_tilegen_deferred).Set(m_renderprocess->GetDeferredRenderingParameterBuffer());
	
	// Bind the pipeline and perform full-screen quad rendering on the downsampled buffer
	m_pipeline_tilegen->Bind();
	m_renderprocess->RenderFullScreenQuad();
	m_pipeline_tilegen->Unbind();
}

// 2. Velocity-space neighbourhood determination
void PostProcessMotionBlur::ExecuteNeighbourhoodDeterminationPass(TextureDX11 *velocity_tile_data)
{
	assert(velocity_tile_data);

	// Populate shader parameters
	m_pipeline_neighbour->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_neighbour_tilebuffer).Set(velocity_tile_data);

	// TODO: disabled for now, but a CB will be required when we are correctly passing downsampled texture dimensions rather than testing in the PS
	//m_pipeline_neighbour->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_neighbour_deferred).Set(m_renderprocess->GetDeferredRenderingParameterBuffer());

	// Bind the pipeline and perform full-screen quad rendering on the downsampled buffer
	m_pipeline_neighbour->Bind();
	m_renderprocess->RenderFullScreenQuad();
	m_pipeline_neighbour->Unbind();
}

// 3. Colour sampling and gather to final output
void PostProcessMotionBlur::ExecuteGatherPass(	TextureDX11 *source_colour, TextureDX11 *source_depth, TextureDX11 *source_vel,
												TextureDX11 *velocity_neighbourhood_buffer)
{
	assert(source_colour);
	assert(source_depth);
	assert(source_vel);
	assert(velocity_neighbourhood_buffer);

	// Populate shader parameters
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_deferred).Set(m_renderprocess->GetDeferredRenderingParameterBuffer());
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_colour).Set(source_colour);
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_depth).Set(source_depth);
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_velocity).Set(source_vel);
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_vel_neighbourhood).Set(velocity_neighbourhood_buffer);

	// We will inherit the noise generator configuration of the parent render process, no need for it to differ here
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_noise_tex).Set(Game::Engine->GetNoiseGenerator()->GetActiveNoiseResource());
	m_pipeline_gather->GetShader(Shader::Type::PixelShader)->GetParameter(m_param_ps_gather_noise_data).Set(Game::Engine->GetNoiseGenerator()->GetActiveNoiseBuffer());

	// Bind the pipeline and upsample motion blur intermediate data back up to combine via gather with the colour buffer
	m_pipeline_gather->Bind();
	m_renderprocess->RenderFullScreenQuad();
	m_pipeline_gather->Unbind();
}



// Destructor
PostProcessMotionBlur::~PostProcessMotionBlur(void)
{

}
