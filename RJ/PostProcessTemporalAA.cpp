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

const std::string PostProcessTemporalAA::TX_NAME_REPROJECTION0 = "TemporalAA_Reprojection0_TX";
const std::string PostProcessTemporalAA::TX_NAME_REPROJECTION1 = "TemporalAA_Reprojection0_TX";
const std::string PostProcessTemporalAA::TX_NAME_FINAL = "TemporalAA_Final_TX";
const std::string PostProcessTemporalAA::RT_NAME_0 = "TemporalAA_RT0";
const std::string PostProcessTemporalAA::RT_NAME_1 = "TemporalAA_RT1";


PostProcessTemporalAA::PostProcessTemporalAA(void)
{
	// Only for default construction
}

PostProcessTemporalAA::PostProcessTemporalAA(DeferredRenderProcess * render_process)
	:
	PostProcessComponent("temporal-aa", "Temporal Anti-Aliasing"), 

	m_vs_quad(NULL), 
	m_ps_temporal(NULL), 

	m_tx_reprojection(), 
	m_tx_final(NULL), 
	m_rt(), 

	m_param_ps_temporal_deferred(ShaderDX11::INVALID_SHADER_PARAMETER)
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

}



// Destructor
PostProcessTemporalAA::~PostProcessTemporalAA(void)
{

}


