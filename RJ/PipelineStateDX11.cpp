#include "PipelineStateDX11.h"
#include "ShaderDX11.h"
#include "RenderTargetDX11.h"
#include "BlendStateDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilStateDX11.h"


PipelineStateDX11::PipelineStateDX11(void)
	:
	m_rendertarget(NULL)
{
	for (int i = 0; i < (int)Shader::Type::SHADER_TYPE_COUNT; ++i)
	{
		m_shaders[i] = NULL;
	}
}

PipelineStateDX11::~PipelineStateDX11()
{
}

void PipelineStateDX11::SetShader(Shader::Type type, ShaderDX11 * pShader)
{
	assert((int)type >= 0 && (int)type < (int)Shader::Type::SHADER_TYPE_COUNT);

	m_shaders[(int)type] = pShader;
}

ShaderDX11 * PipelineStateDX11::GetShader(Shader::Type type) const
{
	assert((int)type >= 0 && (int)type < (int)Shader::Type::SHADER_TYPE_COUNT);

	return m_shaders[(int)type];
}

const PipelineStateDX11::PipelineShadersDX11 & PipelineStateDX11::GetShaders() const
{
	return m_shaders;
}

void PipelineStateDX11::SetBlendState(const BlendStateDX11 & blendState)
{
	m_blendstate = blendState;
}

BlendStateDX11 & PipelineStateDX11::GetBlendState()
{
	return m_blendstate;
}

void PipelineStateDX11::SetRasterizerState(const RasterizerStateDX11 & rasterizerState)
{
	m_rasterizerstate = rasterizerState;
}

RasterizerStateDX11 & PipelineStateDX11::GetRasterizerState()
{
	return m_rasterizerstate;
}

void PipelineStateDX11::SetDepthStencilState(const DepthStencilStateDX11 & depthStencilState)
{
	m_depthstencilstate = depthStencilState;
}

DepthStencilStateDX11 & PipelineStateDX11::GetDepthStencilState()
{
	return m_depthstencilstate;
}

void PipelineStateDX11::SetRenderTarget(RenderTargetDX11 * renderTarget)
{
	m_rendertarget = renderTarget;
}

RenderTargetDX11 * PipelineStateDX11::GetRenderTarget() const
{
	return m_rendertarget;
}

void PipelineStateDX11::Bind()
{
	if (m_rendertarget)
	{
		m_rendertarget->Bind();
	}

	m_blendstate.Bind();
	m_rasterizerstate.Bind();
	m_depthstencilstate.Bind();

	for (auto * shader : m_shaders)
	{
		if (shader)
		{
			shader->Bind();
		}
	}
}

void PipelineStateDX11::Unbind()
{
	if (m_rendertarget)
	{
		m_rendertarget->Unbind();
	}

	for (auto * shader : m_shaders)
	{
		if (shader)
		{
			shader->Unbind();
		}
	}
}