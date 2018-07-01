#include <tuple>
#include <vector>
#include "PostProcessMotionBlur.h"
#include "DeferredRenderProcess.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "RenderProcessDX11.h"
#include "Data/Shaders/DeferredRenderingBuffers.hlsl"

const std::string PostProcessMotionBlur::RT_NAME_TILEGEN = "MotionBlur_Tilegen_RT";
const std::string PostProcessMotionBlur::RT_NAME_NEIGHBOUR = "MotionBlur_Neighbour_RT";
const std::string PostProcessMotionBlur::RT_NAME_GATHER = "MotionBlur_Gather_RT";
const std::string PostProcessMotionBlur::TX_NAME_TILEGEN = "MotionBlur_Tilegen_TX";
const std::string PostProcessMotionBlur::TX_NAME_NEIGHBOUR = "MotionBlur_Neighbour_TX";
const std::string PostProcessMotionBlur::TX_NAME_GATHER = "MotionBlur_Gather_TX";


PostProcessMotionBlur::PostProcessMotionBlur(void)
{
	// Default constructor only provided to allow initial ptr creation with deferred initialisation
}

PostProcessMotionBlur::PostProcessMotionBlur(DeferredRenderProcess * render_process)
	:
	m_renderprocess(render_process), 

	m_vs(NULL), 
	m_ps_tilegen(NULL), 
	m_ps_neighbourhood(NULL), 
	m_ps_gather(NULL), 

	m_rt_tilegen(NULL), 
	m_rt_neighbour(NULL), 
	m_rt_gather(NULL), 

	m_tx_tilegen(NULL), 
	m_tx_neighbour(NULL), 
	m_tx_gather(NULL), 

	m_param_vs_framedata(ShaderDX11::INVALID_SHADER_PARAMETER),
	m_param_ps_tilegen_deferred(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_neighbour_deferred(ShaderDX11::INVALID_SHADER_PARAMETER), 
	m_param_ps_gather_deferred(ShaderDX11::INVALID_SHADER_PARAMETER)
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
}

void PostProcessMotionBlur::InitialiseShaders(void)
{
	Game::Log << LOG_INFO << "Initialising post-process motion blur shaders\n";

	// Get a reference to all required shaders
	m_vs = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::StandardVertexShader);
	if (m_vs == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [vs]\n";

	m_ps_tilegen = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::MotionBlurTileGen);
	if (m_ps_tilegen == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [ps_t]\n";

	m_ps_neighbourhood = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::MotionBlurNeighbourhood);
	if (m_ps_neighbourhood == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [ps_n]\n";

	m_ps_gather = Game::Engine->GetRenderDevice()->Assets.GetShader(Shaders::MotionBlurGather);
	if (m_ps_gather == NULL) Game::Log << LOG_ERROR << "Cannot load post-process motion blur shader resources [ps_g]\n";


	// Ensure we have valid indices into the shader parameter sets
	m_param_vs_framedata = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_vs, FrameDataBufferName);
	m_param_ps_tilegen_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_tilegen, DeferredRenderingParamBufferName);
	m_param_ps_neighbour_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_neighbourhood, DeferredRenderingParamBufferName);
	m_param_ps_gather_deferred = RenderProcessDX11::AttemptRetrievalOfShaderParameter(m_ps_gather, DeferredRenderingParamBufferName);
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

		*ppTex = Game::Engine->GetAssets().CreateTexture2D(txname, size.x, size.y, 1U, txformat);
		if (!(*ppTex)) Game::Log << LOG_ERROR << "Failed to create motion blur " << desc << " texture resource\n";

		// Render target
		if (Game::Engine->GetAssets().AssetExists<RenderTargetDX11>(rtname))
		{
			Game::Engine->GetAssets().DeleteAsset<RenderTargetDX11>(rtname);
		}

		*ppRT = Game::Engine->GetAssets().CreateRenderTarget(rtname, size.Convert<int>());
		if (!(*ppRT)) Game::Log << LOG_ERROR << "Failed to create motion blur " << desc << " render target\n";
	}
}

void PostProcessMotionBlur::InitialiseStandardBuffers(void)
{
	Game::Log << LOG_INFO << "Initialising post-process motion blur standard buffers\n";

}


// Tiled dimensions (per-dimension tile count, K) for this postprocess
void PostProcessMotionBlur::SetTileScalingFactor(unsigned int K)
{
	m_tilesize_k = max(1U, K);
	m_tiled_dimensions = IntegralVector2<unsigned int>(
		static_cast<unsigned int>(Game::ScreenWidth) / m_tilesize_k,
		static_cast<unsigned int>(Game::ScreenHeight) / m_tilesize_k
	);
}


// Destructor
PostProcessMotionBlur::~PostProcessMotionBlur(void)
{

}
