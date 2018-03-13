#pragma once

#include <vector>
#include "Shader.h"
class TextureDX11;
class RenderTargetDX11;

// Manages all resources required for the geometry buffer in DX11 deferred rendering
class DeferredGBuffer
{
public:

	enum class GBufferTexture { Diffuse = 0, Specular = 1, Normal = 2, Depth = 3 };

	DeferredGBuffer(void);

	RenderTargetDX11 *				RenderTarget;

	TextureDX11 *					DiffuseTexture;
	TextureDX11 *					SpecularTexture;
	TextureDX11 *					NormalTexture;
	TextureDX11 *					DepthStencilTexture;

	TextureDX11 *					LookupTexture(GBufferTexture texture);
	
	void							Bind(Shader::Type shader_type);
	void							Unbind(Shader::Type shader_type);

	~DeferredGBuffer(void);


	
	static bool						IsDepthTexture(GBufferTexture texture);

private:

	std::vector<TextureDX11*>		m_pTextureResources;
	
};