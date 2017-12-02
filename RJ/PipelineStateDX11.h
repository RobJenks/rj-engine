#pragma once

#include "Shader.h"
#include "BlendStateDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilStateDX11.h"
class RenderTarget;


class PipelineStateDX11
{
public:



private:

	Shader						m_shader[(int)Shader::Type::SHADER_TYPE_COUNT];
	BlendStateDX11 				m_blendstate;
	RasterizerStateDX11			m_rasterizerstate;
	DepthStencilStateDX11		m_depthstencilstate;
	RenderTarget *				m_rendertarget;

};