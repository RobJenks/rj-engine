#include <numeric>
#include "DeferredGBuffer.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"

DeferredGBuffer::DeferredGBuffer(void)
	:
	RenderTarget(NULL), 
	DepthOnlyRenderTarget(NULL), 
	ColourOnlyRenderTarget(NULL), 

	DiffuseTexture(NULL), 
	SpecularTexture(NULL), 
	NormalTexture(NULL), 
	DepthTexture(NULL)
{
	// First initialise all component texture resources
	Game::Log << LOG_INFO << "Initialising GBuffer texture resources\n";

	// Diffuse albedo buffer (Color1) 
	Texture::TextureFormat diffuseTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 8, 8, 0, 0);
	DiffuseTexture = Game::Engine->GetRenderDevice()->CreateTexture2D("GBuffer::Diffuse", Game::ScreenWidth, Game::ScreenHeight, 1, diffuseTextureFormat);

	// Specular buffer (Color2)
	Texture::TextureFormat specularTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 8, 8, 0, 0);
	SpecularTexture = Game::Engine->GetRenderDevice()->CreateTexture2D("GBuffer::Specular", Game::ScreenWidth, Game::ScreenHeight, 1, specularTextureFormat);

	// Normal buffer (Color3)
	Texture::TextureFormat normalTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::Float,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		32, 32, 32, 32, 0, 0);
	NormalTexture = Game::Engine->GetRenderDevice()->CreateTexture2D("GBuffer::Normal", Game::ScreenWidth, Game::ScreenHeight, 1, normalTextureFormat);

	// Depth/stencil buffer
	Texture::TextureFormat depthStencilTextureFormat(
		Texture::Components::DepthStencil,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		0, 0, 0, 0, 24, 8);
	DepthTexture = Game::Engine->GetRenderDevice()->CreateTexture2D("GBuffer::Depth", Game::ScreenWidth, Game::ScreenHeight, 1, depthStencilTextureFormat);

	// Verify all resources were created
	std::vector<TextureDX11*> pTextureResources({ DiffuseTexture, SpecularTexture, NormalTexture, DepthTexture });
	unsigned int loaded = std::accumulate(pTextureResources.begin(), pTextureResources.end(), 0, []
		(unsigned int accum, TextureDX11 *item) { return (item ? accum + 1 : accum); });

	if (loaded == pTextureResources.size())
	{
		Game::Log << LOG_INFO << loaded << " of " << pTextureResources.size() << " GBuffer texture resources loaded successfully\n";
	}
	else
	{
		Game::Log << LOG_ERROR << "Only " << loaded << " of " << pTextureResources.size() << " GBuffer texture resources loaded successfully, cannot proceed\n";
		return;
	}


	// Now initialise the GBuffer render target
	// The light accumulation buffer in Color0 can be bound directly to the primary colour RT (i.e. backbuffer)
	// since we do not currently need any separation between final GBuffer colour accumulation and the backbuffer itself
	Game::Log << LOG_INFO << "Initialising GBuffer RT\n";

	RenderTarget = Game::Engine->GetRenderDevice()->CreateRenderTarget();
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color0,
		Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->GetTexture(RenderTarget::AttachmentPoint::Color0));
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color1, DiffuseTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color2, SpecularTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color3, NormalTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, DepthTexture);

	Game::Log << LOG_INFO << "GBuffer initialisation complete\n";
}



DeferredGBuffer::~DeferredGBuffer(void)
{
}