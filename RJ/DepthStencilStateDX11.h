#pragma once

#include "DepthStencilState.h"
#include "DX11_Core.h"

class DepthStencilStateDX11 : public DepthStencilState
{
public:

	DepthStencilStateDX11(void);
	DepthStencilStateDX11(const DepthStencilStateDX11 & copy);

	~DepthStencilStateDX11();

	const DepthStencilStateDX11& operator=(const DepthStencilStateDX11& other);

	void SetDepthMode(const DepthMode& depthMode);
	const DepthMode& GetDepthMode() const;

	void SetStencilMode(const StencilMode& stencilMode);
	const StencilMode& GetStencilMode() const;

	// Can only be called by the pipeline state.
	void Bind();

private:

	D3D11_DEPTH_WRITE_MASK TranslateDepthWriteMask(DepthWrite depthWrite) const;
	D3D11_COMPARISON_FUNC TranslateCompareFunc(CompareFunction compareFunc) const;
	D3D11_STENCIL_OP TranslateStencilOperation(StencilOperation stencilOperation) const;
	D3D11_DEPTH_STENCILOP_DESC TranslateFaceOperation(FaceOperation faceOperation) const;
	D3D11_DEPTH_STENCIL_DESC TranslateDepthStencilState(const DepthMode& depthMode, const StencilMode& stencilMode) const;

private:

	ID3D11DepthStencilState *				m_depthstencilstate;

	DepthMode								m_depthmode;
	StencilMode								m_stencilmode;

	bool m_isdirty;
};


