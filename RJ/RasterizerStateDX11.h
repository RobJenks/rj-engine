#pragma once

#include <array>
#include "RasterizerState.h"
#include "Rendering.h"
#include "DX11_Core.h"
#include "Rect.h"
#include "Viewport.h"


class RasterizerStateDX11 : public RasterizerState
{
public:

	typedef std::array<Rect, Rendering::MaxRenderTargets>			RectList;
	typedef std::array<D3D11_RECT, Rendering::MaxRenderTargets>		D3DRectList;
	typedef std::array<Viewport, Rendering::MaxRenderTargets>		ViewportList;
	typedef std::array<D3D11_VIEWPORT, Rendering::MaxRenderTargets>	D3DViewportList;

	RasterizerStateDX11(void);
	RasterizerStateDX11(const RasterizerStateDX11& copy);
	~RasterizerStateDX11();

	const RasterizerStateDX11& operator=(const RasterizerStateDX11& other);

	void SetFillMode(FillMode frontFace = FillMode::Solid, FillMode backFace = FillMode::Solid);
	void GetFillMode(FillMode& frontFace, FillMode& backFace) const;

	void SetCullMode(CullMode cullMode = CullMode::Back);
	CullMode GetCullMode() const;

	void SetFrontFacing(FrontFace frontFace = FrontFace::Clockwise);
	FrontFace GetFrontFacing() const;

	void SetDepthBias(float depthBias = 0.0f, float slopeBias = 0.0f, float biasClamp = 0.0f);
	void GetDepthBias(float& depthBias, float& slopeBias, float& biasClamp) const;

	void SetDepthClipEnabled(bool depthClipEnabled = true);
	bool GetDepthClipEnabled() const;

	void SetViewport(const Viewport& viewport);
	void SetViewport(const Viewport& viewport, ViewportList::size_type index);
	void SetViewports(const ViewportList & viewports);
	const ViewportList & GetViewports();

	void SetCompiledViewportDirect(const Viewport & viewport, const D3D11_VIEWPORT & compiled_viewport);
	void SetCompiledViewportDirect(const Viewport & viewport, const D3D11_VIEWPORT & compiled_viewport, ViewportList::size_type index);

	void SetScissorEnabled(bool scissorEnable = false);
	bool GetScissorEnabled() const;

	void SetScissorRect(const Rect& rect);
	void SetScissorRect(const Rect& rect, RectList::size_type index);
	void SetScissorRects(const RectList & rects);
	const RectList & GetScissorRects() const;

	void SetMultisampleEnabled(bool multisampleEnabled = false);
	bool GetMultisampleEnabled() const;

	void SetAntialiasedLineEnable(bool antialiasedLineEnabled);
	bool GetAntialiasedLineEnable() const;

	void SetForcedSampleCount(uint8_t sampleCount);
	uint8_t GetForcedSampleCount();

	void SetConservativeRasterizationEnabled(bool conservativeRasterizationEnabled = false);
	bool GetConservativeRasterizationEnabled() const;

	// Can only be invoked by the pipeline state
	void Bind();

private:

	D3D11_FILL_MODE TranslateFillMode(FillMode fillMode) const;
	D3D11_CULL_MODE TranslateCullMode(CullMode cullMode) const;
	bool TranslateFrontFace(FrontFace frontFace) const;

	D3DRectList TranslateRects(const RectList & rects) const;
	D3DViewportList TranslateViewports(const ViewportList & viewports) const;

private:

	ID3D11RasterizerState1 *	m_rasterstate;

	RectList					m_scissorrects;
	D3DRectList					m_d3d_rects;
	
	ViewportList				m_viewports;
	D3DViewportList				m_d3d_viewports;


	FillMode			m_FrontFaceFillMode;
	FillMode			m_BackFaceFillMode;

	CullMode			m_CullMode;

	FrontFace			m_FrontFace;

	float				m_DepthBias;
	float				m_SlopeBias;
	float				m_BiasClamp;

	bool				m_DepthClipEnabled;
	bool				m_ScissorEnabled;

	bool				m_MultisampleEnabled;
	bool				m_AntialiasedLineEnabled;

	bool				m_ConservativeRasterization;

	uint8_t				m_ForcedSampleCount;

	bool				m_statedirty, m_rectsdirty, m_viewportsdirty;
};



