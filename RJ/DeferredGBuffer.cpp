#include <numeric>
#include "DeferredGBuffer.h"
#include "TextureDX11.h"
#include "RenderTargetDX11.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"
#include "Data/Shaders/DeferredRenderingGBuffer.hlsl.h"


DeferredGBuffer::DeferredGBuffer(void)
	:
	RenderTarget(NULL), 
	
	DiffuseTexture(NULL), 
	SpecularTexture(NULL), 
	NormalTexture(NULL), 
	DepthStencilTexture(NULL), 
	VelocityTexture(NULL)
{
	// First initialise all component texture resources
	Game::Log << LOG_INFO << "Initialising GBuffer texture resources\n";

	// Diffuse albedo buffer (Color1) 
	Texture::TextureFormat diffuseTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 8, 8, 0, 0);
	DiffuseTexture = Game::Engine->GetAssets().CreateTexture2D(GBufferDiffuseTextureName, Game::ScreenWidth, Game::ScreenHeight, 1U, diffuseTextureFormat);

	// Specular buffer (Color2)
	Texture::TextureFormat specularTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 8, 8, 0, 0);
	SpecularTexture = Game::Engine->GetAssets().CreateTexture2D(GBufferSpecularTextureName, Game::ScreenWidth, Game::ScreenHeight, 1U, specularTextureFormat);

	// Normal buffer (Color3)
	Texture::TextureFormat normalTextureFormat(
		Texture::Components::RGBA,
		Texture::Type::Float,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		32, 32, 32, 32, 0, 0);
	NormalTexture = Game::Engine->GetAssets().CreateTexture2D(GBufferNormalTextureName, Game::ScreenWidth, Game::ScreenHeight, 1U, normalTextureFormat);

	// Velocity buffer (Color4)
	Texture::TextureFormat velocityTextureFormat(
		Texture::Components::RG,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		8, 8, 0, 0, 0, 0
	);
	VelocityTexture = Game::Engine->GetRenderDevice()->Assets.CreateTexture2D(GBufferVelocityTextureName, Game::ScreenWidth, Game::ScreenHeight, 1U, velocityTextureFormat);

	// Depth/stencil buffer
	Texture::TextureFormat depthStencilTextureFormat(
		Texture::Components::DepthStencil,
		Texture::Type::UnsignedNormalized,
		RenderDeviceDX11::TEXTURE_MULTISAMPLE_COUNT,
		0, 0, 0, 0, 24, 8);
	DepthStencilTexture = Game::Engine->GetAssets().CreateTexture2D(GBufferDepthTextureName, Game::ScreenWidth, Game::ScreenHeight, 1U, depthStencilTextureFormat);


	// Verify all resources were created
	std::vector<TextureDX11*> pTextureResources({ DiffuseTexture, SpecularTexture, NormalTexture, DepthStencilTexture, VelocityTexture });
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
	// NOTE: The light accumulation buffer in Color0 should be bound by the calling render process.  It is left unbound
	// on GBuffer initialisation for this reason.  Expectation is that the caller must bind the light accumulation
	// target before performing a render cycle
	Game::Log << LOG_INFO << "Initialising GBuffer RT\n";
	
	RenderTarget = Game::Engine->GetAssets().CreateRenderTarget("GBufferRenderTarget", Game::Engine->GetRenderDevice()->GetDisplaySize());
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color0, NULL);				// Light accumulation buffer to be bound by caller
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color1, DiffuseTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color2, SpecularTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color3, NormalTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color4, VelocityTexture);
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil, DepthStencilTexture);

	Game::Log << LOG_INFO << "GBuffer initialisation complete\n";
}

// Bind an existing colour buffer as the light accumulation target for this GBuffer
void DeferredGBuffer::BindToTargetLightAccumulationBuffer(TextureDX11 *targetbuffer)
{
	RenderTarget->AttachTexture(RenderTarget::AttachmentPoint::Color0, targetbuffer);
}

// Unbind the light accumulation target for this render buffer.  Target should never be
// unbound when executing a render cycle
void DeferredGBuffer::UnbindTargetLightAccumulationBuffer(void)
{
	BindToTargetLightAccumulationBuffer(NULL);
}


void DeferredGBuffer::Bind(Shader::Type shader_type)
{
	DiffuseTexture->Bind(shader_type, 0U, ShaderParameter::Type::Texture);
	SpecularTexture->Bind(shader_type, 1U, ShaderParameter::Type::Texture);
	NormalTexture->Bind(shader_type, 2U, ShaderParameter::Type::Texture);
	VelocityTexture->Bind(shader_type, 3U, ShaderParameter::Type::Texture);
	DepthStencilTexture->Bind(shader_type, 4U, ShaderParameter::Type::Texture);
}

void DeferredGBuffer::Unbind(Shader::Type shader_type)
{
	DiffuseTexture->Unbind(shader_type, 0U, ShaderParameter::Type::Texture);
	SpecularTexture->Unbind(shader_type, 1U, ShaderParameter::Type::Texture);
	NormalTexture->Unbind(shader_type, 2U, ShaderParameter::Type::Texture);
	VelocityTexture->Unbind(shader_type, 3U, ShaderParameter::Type::Texture);
	DepthStencilTexture->Unbind(shader_type, 4U, ShaderParameter::Type::Texture);
}


DeferredGBuffer::~DeferredGBuffer(void)
{
}


TextureDX11 * DeferredGBuffer::LookupTexture(GBufferTexture texture)
{
	switch (texture)
	{
		case GBufferTexture::Diffuse:			return DiffuseTexture;
		case GBufferTexture::Specular:			return SpecularTexture;
		case GBufferTexture::Normal:			return NormalTexture;
		case GBufferTexture::Velocity:			return VelocityTexture;
		case GBufferTexture::Depth:				return DepthStencilTexture;

		default:								return NULL;
	}
}

bool DeferredGBuffer::IsDepthTexture(GBufferTexture texture)
{
	return (texture == GBufferTexture::Depth);
}