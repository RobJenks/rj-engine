#include <numeric>
#include "DeferredGBuffer.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"

DeferredGBuffer::DeferredGBuffer(void)
	:
	RenderTarget(NULL), 
	
	DiffuseTexture(NULL), 
	SpecularTexture(NULL), 
	NormalTexture(NULL), 
	DepthStencilTexture(NULL)
{
	// First initialise all component texture resources
	Game::Log << LOG_INFO << "Initialising GBuffer texture resources\n";

	// Diffuse albedo buffer (Color1) 
	Texture::TextureFormat diffuseTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 8, 8, 0, 0);
	DiffuseTexture = Game::Engine->GetAssets().CreateTexture2D("GBuffer::Diffuse", Game::ScreenWidth, Game::ScreenHeight, 1, diffuseTextureFormat);

	// Specular buffer (Color2)
	Texture::TextureFormat specularTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 8, 8, 0, 0);
	SpecularTexture = Game::Engine->GetAssets().CreateTexture2D("GBuffer::Specular", Game::ScreenWidth, Game::ScreenHeight, 1, specularTextureFormat);

	// Normal buffer (Color3)
	Texture::TextureFormat normalTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::Float,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		32, 32, 32, 32, 0, 0);
	NormalTexture = Game::Engine->GetAssets().CreateTexture2D("GBuffer::Normal", Game::ScreenWidth, Game::ScreenHeight, 1, normalTextureFormat);

	// Depth/stencil buffer
	Texture::TextureFormat depthStencilTextureFormat(
		Texture::Components::DepthStencil,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		0, 0, 0, 0, 24, 8);
	DepthStencilTexture = Game::Engine->GetAssets().CreateTexture2D("GBuffer::DepthStencil", Game::ScreenWidth, Game::ScreenHeight, 1, depthStencilTextureFormat);

	// Verify all resources were created
	std::vector<TextureDX11*> pTextureResources({ DiffuseTexture, SpecularTexture, NormalTexture, DepthStencilTexture });
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
	
	RenderTarget = Game::Engine->GetAssets().CreateRenderTarget("GBufferRenderTarget", Game::Engine->GetRenderDevice()->GetDisplaySize());
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color0,
		Game::Engine->GetRenderDevice()->GetPrimaryRenderTarget()->GetTexture(RenderTarget::AttachmentPoint::Color0));
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color1, DiffuseTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color2, SpecularTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color3, NormalTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, DepthStencilTexture);

	Game::Log << LOG_INFO << "GBuffer initialisation complete\n";
}


void DeferredGBuffer::Bind(Shader::Type shader_type)
{
	DiffuseTexture->Bind(shader_type, 0U, ShaderParameter::Type::Texture);
	SpecularTexture->Bind(shader_type, 1U, ShaderParameter::Type::Texture);
	NormalTexture->Bind(shader_type, 2U, ShaderParameter::Type::Texture);
	DepthStencilTexture->Bind(shader_type, 3U, ShaderParameter::Type::Texture);
}

void DeferredGBuffer::Unbind(Shader::Type shader_type)
{
	DiffuseTexture->Unbind(shader_type, 0U, ShaderParameter::Type::Texture);
	SpecularTexture->Unbind(shader_type, 1U, ShaderParameter::Type::Texture);
	NormalTexture->Unbind(shader_type, 2U, ShaderParameter::Type::Texture);
	DepthStencilTexture->Unbind(shader_type, 3U, ShaderParameter::Type::Texture);
}


DeferredGBuffer::~DeferredGBuffer(void)
{
}