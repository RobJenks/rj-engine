#include "Logging.h"
#include "CoreEngine.h"
#include "RasterizerStateDX11.h"

RasterizerStateDX11::RasterizerStateDX11(void)
	: m_FrontFaceFillMode(FillMode::Solid)
	, m_BackFaceFillMode(FillMode::Solid)
	, m_CullMode(CullMode::Back)
	, m_FrontFace(FrontFace::Clockwise)
	, m_DepthBias(0.0f)
	, m_SlopeBias(0.0f)
	, m_BiasClamp(0.0f)
	, m_DepthClipEnabled(true)
	, m_ScissorEnabled(false)
	, m_MultisampleEnabled(false)
	, m_AntialiasedLineEnabled(false)
	, m_ConservativeRasterization(false)
	, m_ForcedSampleCount(0)
	, m_statedirty(true)
	, m_viewportsdirty(true)
	, m_rectsdirty(true)
{
}

RasterizerStateDX11::RasterizerStateDX11(const RasterizerStateDX11& copy)
	: m_d3d_rects(copy.m_d3d_rects)
	, m_d3d_viewports(copy.m_d3d_viewports)
	, m_FrontFaceFillMode(copy.m_FrontFaceFillMode)
	, m_BackFaceFillMode(copy.m_BackFaceFillMode)
	, m_CullMode(copy.m_CullMode)
	, m_FrontFace(copy.m_FrontFace)
	, m_DepthBias(copy.m_DepthBias)
	, m_SlopeBias(copy.m_SlopeBias)
	, m_BiasClamp(copy.m_BiasClamp)
	, m_DepthClipEnabled(copy.m_DepthClipEnabled)
	, m_ScissorEnabled(copy.m_ScissorEnabled)
	, m_MultisampleEnabled(copy.m_MultisampleEnabled)
	, m_AntialiasedLineEnabled(copy.m_AntialiasedLineEnabled)
	, m_ConservativeRasterization(copy.m_ConservativeRasterization)
	, m_ForcedSampleCount(copy.m_ForcedSampleCount)
	, m_scissorrects(copy.m_scissorrects)
	, m_viewports(copy.m_viewports)
	, m_statedirty(true)
	, m_viewportsdirty(false)
	, m_rectsdirty(false)
{}

RasterizerStateDX11::~RasterizerStateDX11()
{

}

const RasterizerStateDX11& RasterizerStateDX11::operator=(const RasterizerStateDX11& other)
{
	if (this != &other)
	{
		m_d3d_rects = other.m_d3d_rects;
		m_d3d_viewports = other.m_d3d_viewports;

		m_FrontFaceFillMode = other.m_FrontFaceFillMode;
		m_BackFaceFillMode = other.m_BackFaceFillMode;

		m_CullMode = other.m_CullMode;

		m_FrontFace = other.m_FrontFace;

		m_DepthBias = other.m_DepthBias;
		m_SlopeBias = other.m_SlopeBias;
		m_BiasClamp = other.m_BiasClamp;

		m_DepthClipEnabled = other.m_DepthClipEnabled;
		m_ScissorEnabled = other.m_ScissorEnabled;

		m_MultisampleEnabled = other.m_MultisampleEnabled;
		m_AntialiasedLineEnabled = other.m_AntialiasedLineEnabled;

		m_ConservativeRasterization = other.m_ConservativeRasterization;

		m_ForcedSampleCount = other.m_ForcedSampleCount;

		m_scissorrects = other.m_scissorrects;
		m_viewports = other.m_viewports;

		m_statedirty = true;
		m_viewportsdirty = false;
		m_rectsdirty = false;
	}

	return *this;
}

void RasterizerStateDX11::SetFillMode(FillMode frontFace, FillMode backFace)
{
	m_FrontFaceFillMode = frontFace;
	m_BackFaceFillMode = backFace;
}

void RasterizerStateDX11::GetFillMode(FillMode& frontFace, FillMode& backFace) const
{
	frontFace = m_FrontFaceFillMode;
	backFace = m_BackFaceFillMode;
}

void RasterizerStateDX11::SetCullMode(CullMode cullMode)
{
	m_CullMode = cullMode;
	m_statedirty = true;
}

RasterizerState::CullMode RasterizerStateDX11::GetCullMode() const
{
	return m_CullMode;
}

void RasterizerStateDX11::SetFrontFacing(FrontFace frontFace)
{
	m_FrontFace = frontFace;
	m_statedirty = true;
}

RasterizerState::FrontFace RasterizerStateDX11::GetFrontFacing() const
{
	return m_FrontFace;
}

void RasterizerStateDX11::SetDepthBias(float depthBias, float slopeBias, float biasClamp)
{
	m_DepthBias = depthBias;
	m_SlopeBias = slopeBias;
	m_BiasClamp = biasClamp;

	m_statedirty = true;
}

void RasterizerStateDX11::GetDepthBias(float& depthBias, float& slopeBias, float& biasClamp) const
{
	depthBias = m_DepthBias;
	slopeBias = m_SlopeBias;
	biasClamp = m_BiasClamp;
}

void RasterizerStateDX11::SetDepthClipEnabled(bool depthClipEnabled)
{
	m_DepthClipEnabled = depthClipEnabled;
	m_statedirty = true;
}

bool RasterizerStateDX11::GetDepthClipEnabled() const
{
	return m_DepthClipEnabled;
}

void RasterizerStateDX11::SetViewport(const Viewport& viewport)
{
	m_viewports[0] = viewport;
	m_viewportsdirty = true;
}

void RasterizerStateDX11::SetViewport(const Viewport& viewport, ViewportList::size_type index)
{
	m_viewports[index] = viewport;
	m_viewportsdirty = true;
}

void RasterizerStateDX11::SetViewports(const ViewportList & viewports)
{
	m_viewports = viewports;
	m_viewportsdirty = true;
}

const RasterizerStateDX11::ViewportList & RasterizerStateDX11::GetViewports()
{
	return m_viewports;
}

void RasterizerStateDX11::SetScissorEnabled(bool scissorEnable)
{
	m_ScissorEnabled = scissorEnable;
	m_statedirty = true;
}

bool RasterizerStateDX11::GetScissorEnabled() const
{
	return m_ScissorEnabled;
}

void RasterizerStateDX11::SetScissorRect(const Rect& rect)
{
	m_scissorrects[0] = rect;
	m_rectsdirty = true;
}

void RasterizerStateDX11::SetScissorRect(const Rect& rect, RectList::size_type index)
{
	m_scissorrects[index] = rect;
	m_rectsdirty = true;
}

void RasterizerStateDX11::SetScissorRects(const RectList & rects)
{
	m_scissorrects = rects;
	m_rectsdirty = true;
}

const RasterizerStateDX11::RectList & RasterizerStateDX11::GetScissorRects() const
{
	return m_scissorrects;
}

void RasterizerStateDX11::SetMultisampleEnabled(bool multisampleEnabled)
{
	m_MultisampleEnabled = multisampleEnabled;
	m_statedirty = true;
}

bool RasterizerStateDX11::GetMultisampleEnabled() const
{
	return m_MultisampleEnabled;
}

void RasterizerStateDX11::SetAntialiasedLineEnable(bool antialiasedLineEnabled)
{
	m_AntialiasedLineEnabled = antialiasedLineEnabled;
	m_statedirty = true;
}

bool RasterizerStateDX11::GetAntialiasedLineEnable() const
{
	return m_AntialiasedLineEnabled;
}

void RasterizerStateDX11::SetForcedSampleCount(uint8_t sampleCount)
{
	m_ForcedSampleCount = sampleCount;
	m_statedirty = true;
}

uint8_t RasterizerStateDX11::GetForcedSampleCount()
{
	return m_ForcedSampleCount;
}

void RasterizerStateDX11::SetConservativeRasterizationEnabled(bool conservativeRasterizationEnabled)
{
	m_ConservativeRasterization = conservativeRasterizationEnabled;
}

bool RasterizerStateDX11::GetConservativeRasterizationEnabled() const
{
	// Currently, this implementation always returns false
	// because conservative rasterization is only supported from DirectX 11.3 and 12.
	return false;
}

D3D11_FILL_MODE RasterizerStateDX11::TranslateFillMode(FillMode fillMode) const
{
	D3D11_FILL_MODE result = D3D11_FILL_SOLID;
	switch (fillMode)
	{
	case FillMode::Wireframe:
		result = D3D11_FILL_WIREFRAME;
		break;
	case FillMode::Solid:
		result = D3D11_FILL_SOLID;
		break;
	default:
		Game::Log << LOG_ERROR << "Unknown fill mode (" << static_cast<int>(fillMode) << ")\n";
		break;
	}

	return result;
}

D3D11_CULL_MODE RasterizerStateDX11::TranslateCullMode(CullMode cullMode) const
{
	D3D11_CULL_MODE result = D3D11_CULL_BACK;
	switch (cullMode)
	{
	case CullMode::None:
		result = D3D11_CULL_NONE;
		break;
	case CullMode::Front:
		result = D3D11_CULL_FRONT;
		break;
	case CullMode::Back:
		result = D3D11_CULL_BACK;
		break;
	case CullMode::FrontAndBack:
		// This mode is not supported in DX11.
		break;
	default:
		Game::Log << LOG_ERROR << "Unknown cull mode (" << static_cast<int>(cullMode) << ")\n";
		break;
	}

	return result;
}

bool RasterizerStateDX11::TranslateFrontFace(FrontFace frontFace) const
{
	bool frontCounterClockwise = true;
	switch (frontFace)
	{
	case FrontFace::Clockwise:
		frontCounterClockwise = false;
		break;
	case FrontFace::CounterClockwise:
		frontCounterClockwise = true;
		break;
	default:
		Game::Log << LOG_ERROR << "Unknown front face winding order (" << static_cast<int>(frontFace) << ")\n";
		break;
	}

	return frontCounterClockwise;
}

RasterizerStateDX11::D3DRectList RasterizerStateDX11::TranslateRects(const RectList & rects) const
{
	D3DRectList result;
	for (size_t i = 0; i < Rendering::MaxRenderTargets; ++i)
	{
		D3D11_RECT& d3dRect = result[i];
		const Rect& rect = rects[i];

		d3dRect.top = static_cast<LONG>(rect.Y + 0.5f);
		d3dRect.bottom = static_cast<LONG>(rect.Y + rect.Height + 0.5f);
		d3dRect.left = static_cast<LONG>(rect.X + 0.5f);
		d3dRect.right = static_cast<LONG>(rect.X + rect.Width + 0.5f);
	}

	return result;
}

RasterizerStateDX11::D3DViewportList RasterizerStateDX11::TranslateViewports(const ViewportList & viewports) const
{
	D3DViewportList result;
	for (size_t i = 0; i < Rendering::MaxRenderTargets; ++i)
	{
		D3D11_VIEWPORT& d3dViewport = result[i];
		const Viewport& viewport = viewports[i];

		d3dViewport.TopLeftX = viewport.X;
		d3dViewport.TopLeftY = viewport.Y;
		d3dViewport.Width = viewport.Width;
		d3dViewport.Height = viewport.Height;
		d3dViewport.MinDepth = viewport.MinDepth;
		d3dViewport.MaxDepth = viewport.MaxDepth;
	}

	return result;
}

// Can only be invoked by the pipeline state
void RasterizerStateDX11::Bind()
{
	if (m_statedirty)
	{
		D3D11_RASTERIZER_DESC1 rasterizerDesc = {};

		rasterizerDesc.FillMode = TranslateFillMode(m_FrontFaceFillMode);
		rasterizerDesc.CullMode = TranslateCullMode(m_CullMode);
		rasterizerDesc.FrontCounterClockwise = TranslateFrontFace(m_FrontFace);
		rasterizerDesc.DepthBias = (m_DepthBias < 0.0f) ? static_cast<INT>(m_DepthBias - 0.5f) : static_cast<INT>(m_DepthBias + 0.5f);
		rasterizerDesc.DepthBiasClamp = m_BiasClamp;
		rasterizerDesc.SlopeScaledDepthBias = m_SlopeBias;
		rasterizerDesc.DepthClipEnable = m_DepthClipEnabled;
		rasterizerDesc.ScissorEnable = m_ScissorEnabled;
		rasterizerDesc.MultisampleEnable = m_MultisampleEnabled;
		rasterizerDesc.AntialiasedLineEnable = m_AntialiasedLineEnabled;
		rasterizerDesc.ForcedSampleCount = m_ForcedSampleCount;

		HRESULT result = Game::Engine->GetDevice()->CreateRasterizerState1(&rasterizerDesc, &m_rasterstate);
		if (FAILED(result))
		{
			Game::Log << LOG_ERROR << "Failed to create rasterizer state (hr=" << result << ")\n";
		}

		m_statedirty = false;
	}

	if (m_rectsdirty)
	{
		m_d3d_rects = TranslateRects(m_scissorrects);
		m_rectsdirty = false;
	}

	if (m_viewportsdirty)
	{
		m_d3d_viewports = TranslateViewports(m_viewports);
		m_viewportsdirty = false;
	}

	auto devicecontext = Game::Engine->GetDeviceContext();
	devicecontext->RSSetViewports((UINT)m_d3d_viewports.size(), m_d3d_viewports.data());
	devicecontext->RSSetScissorRects((UINT)m_d3d_rects.size(), m_d3d_rects.data());
	devicecontext->RSSetState(m_rasterstate);
}