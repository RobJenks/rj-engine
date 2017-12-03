#pragma once

#include <array>
#include "PipelineState.h"
#include "ShaderDX11.h"
#include "BlendStateDX11.h"
#include "RasterizerStateDX11.h"
#include "DepthStencilStateDX11.h"
class RenderTargetDX11;


class PipelineStateDX11 : public PipelineState
{
public:

	typedef std::array<ShaderDX11*, (int)Shader::Type::SHADER_TYPE_COUNT>	PipelineShadersDX11;

	PipelineStateDX11(void);
	~PipelineStateDX11();

	void SetShader(Shader::Type type, ShaderDX11 *pShader);
	ShaderDX11 * GetShader(Shader::Type type) const;
	const PipelineShadersDX11 & GetShaders() const;

	void SetBlendState(const BlendStateDX11 & blendState);
	BlendStateDX11 & GetBlendState();

	void SetRasterizerState(const RasterizerStateDX11 & rasterizerState);
	RasterizerStateDX11 & GetRasterizerState();

	void SetDepthStencilState(const DepthStencilStateDX11 & depthStencilState);
	DepthStencilStateDX11 & GetDepthStencilState();

	void SetRenderTarget(RenderTargetDX11 *renderTarget);
	RenderTargetDX11 * GetRenderTarget() const;

	void Bind();
	void Unbind();

private:

	// Store references to (at most) one shader per pipeline stage
	PipelineShadersDX11			m_shaders;

	// Key components of pipeline state
	BlendStateDX11 				m_blendstate;
	RasterizerStateDX11			m_rasterizerstate;
	DepthStencilStateDX11		m_depthstencilstate;

	RenderTargetDX11 *			m_rendertarget;

};