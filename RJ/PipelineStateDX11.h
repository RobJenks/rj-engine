#pragma once

#include <array>
#include "Shader.h"
#include "BlendStateDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilStateDX11.h"
class RenderTarget;


class PipelineStateDX11
{
public:



private:

	// Store references to (at most) one shader per pipeline stage
	std::array<Shader*, (int)Shader::Type::SHADER_TYPE_COUNT> m_shaders;

	BlendStateDX11 				m_blendstate;
	RasterizerStateDX11			m_rasterizerstate;
	DepthStencilStateDX11		m_depthstencilstate;
	RenderTarget *				m_rendertarget;

};