#pragma once

#include <vector>
#include "Shader.h"
class TextureDX11;
class RenderTargetDX11;

// Manages all resources required for the geometry buffer in DX11 deferred rendering
class DeferredGBuffer
{
public:

	DeferredGBuffer(void);

	RenderTargetDX11 *				RenderTarget;

	TextureDX11 *					DiffuseTexture;
	TextureDX11 *					SpecularTexture;
	TextureDX11 *					NormalTexture;
	TextureDX11 *					DepthStencilTexture;

	void							Bind(Shader::Type shader_type);
	void							Unbind(Shader::Type shader_type);

	~DeferredGBuffer(void);

private:

	std::vector<TextureDX11*>		m_pTextureResources;
	
};