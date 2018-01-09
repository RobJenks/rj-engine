#include "DepthStencilStateDX11.h"
#include "Logging.h"
#include "Rendering.h"
#include "CoreEngine.h"

DepthStencilStateDX11::DepthStencilStateDX11(void)
	: 
	m_isdirty(true)
{
}

DepthStencilStateDX11::DepthStencilStateDX11(const DepthStencilStateDX11& copy)
	: m_depthmode(copy.m_depthmode)
	, m_stencilmode(copy.m_stencilmode)
	, m_isdirty(true)
{}

DepthStencilStateDX11::~DepthStencilStateDX11()
{

}

const DepthStencilStateDX11& DepthStencilStateDX11::operator=(const DepthStencilStateDX11& other)
{
	if (this != &other)
	{
		m_depthmode = other.m_depthmode;
		m_stencilmode = other.m_stencilmode;
		m_isdirty = true;
	}

	return *this;
}

void DepthStencilStateDX11::SetDepthMode(const DepthMode& depthMode)
{
	m_depthmode = depthMode;
	m_isdirty = true;
}

const DepthStencilState::DepthMode& DepthStencilStateDX11::GetDepthMode() const
{
	return m_depthmode;
}

void DepthStencilStateDX11::SetStencilMode(const StencilMode& stencilMode)
{
	m_stencilmode = stencilMode;
	m_isdirty = true;
}

const DepthStencilState::StencilMode& DepthStencilStateDX11::GetStencilMode() const
{
	return m_stencilmode;
}

D3D11_DEPTH_WRITE_MASK DepthStencilStateDX11::TranslateDepthWriteMask(DepthWrite depthWrite) const
{
	D3D11_DEPTH_WRITE_MASK result = D3D11_DEPTH_WRITE_MASK_ALL;

	switch (depthWrite)
	{
		case DepthWrite::Enable:
			result = D3D11_DEPTH_WRITE_MASK_ALL;
			break;
		case DepthWrite::Disable:
			result = D3D11_DEPTH_WRITE_MASK_ZERO;
			break;
		default:
			Game::Log << LOG_ERROR << "Unknown depth write mask (" << static_cast<int>(depthWrite) << ")\n";
			break;
	}

	return result;
}

D3D11_COMPARISON_FUNC DepthStencilStateDX11::TranslateCompareFunc(CompareFunction compareFunc) const
{
	D3D11_COMPARISON_FUNC result = D3D11_COMPARISON_LESS;

	switch (compareFunc)
	{
		case CompareFunction::Never:
			result = D3D11_COMPARISON_NEVER;
			break;
		case CompareFunction::Less:
			result = D3D11_COMPARISON_LESS;
			break;
		case CompareFunction::Equal:
			result = D3D11_COMPARISON_EQUAL;
			break;
		case CompareFunction::LessOrEqual:
			result = D3D11_COMPARISON_LESS_EQUAL;
			break;
		case CompareFunction::Greater:
			result = D3D11_COMPARISON_GREATER;
			break;
		case CompareFunction::NotEqual:
			result = D3D11_COMPARISON_NOT_EQUAL;
			break;
		case CompareFunction::GreaterOrEqual:
			result = D3D11_COMPARISON_GREATER_EQUAL;
			break;
		case CompareFunction::Always:
			result = D3D11_COMPARISON_ALWAYS;
			break;
		default:
			Game::Log << LOG_ERROR << "Unknown compare function (" << static_cast<int>(compareFunc) << ")\n";
			break;
	}

	return result;
}

D3D11_STENCIL_OP DepthStencilStateDX11::TranslateStencilOperation(StencilOperation stencilOperation) const
{
	D3D11_STENCIL_OP result = D3D11_STENCIL_OP_KEEP;

	switch (stencilOperation)
	{
		case StencilOperation::Keep:
			result = D3D11_STENCIL_OP_KEEP;
			break;
		case StencilOperation::Zero:
			result = D3D11_STENCIL_OP_ZERO;
			break;
		case StencilOperation::Reference:
			result = D3D11_STENCIL_OP_REPLACE;
			break;
		case StencilOperation::IncrementClamp:
			result = D3D11_STENCIL_OP_INCR_SAT;
			break;
		case StencilOperation::DecrementClamp:
			result = D3D11_STENCIL_OP_DECR_SAT;
			break;
		case StencilOperation::Invert:
			result = D3D11_STENCIL_OP_INVERT;
			break;
		case StencilOperation::IncrementWrap:
			result = D3D11_STENCIL_OP_INCR;
			break;
		case StencilOperation::DecrementWrap:
			result = D3D11_STENCIL_OP_DECR;
			break;
		default:
			Game::Log << LOG_ERROR << "Unknown stencil operation (" << static_cast<int>(stencilOperation) << ")\n";
			break;
	}

	return result;
}

D3D11_DEPTH_STENCILOP_DESC DepthStencilStateDX11::TranslateFaceOperation(FaceOperation faceOperation) const
{
	D3D11_DEPTH_STENCILOP_DESC result;

	result.StencilFailOp = TranslateStencilOperation(faceOperation.StencilFail);
	result.StencilDepthFailOp = TranslateStencilOperation(faceOperation.StencilPassDepthFail);
	result.StencilPassOp = TranslateStencilOperation(faceOperation.StencilDepthPass);
	result.StencilFunc = TranslateCompareFunc(faceOperation.StencilFunction);

	return result;
}

D3D11_DEPTH_STENCIL_DESC DepthStencilStateDX11::TranslateDepthStencilState(const DepthMode& depthMode, const StencilMode& stencilMode) const
{
	D3D11_DEPTH_STENCIL_DESC result;

	result.DepthEnable = depthMode.DepthEnable;
	result.DepthWriteMask = TranslateDepthWriteMask(depthMode.DepthWriteMask);
	result.DepthFunc = TranslateCompareFunc(depthMode.DepthFunction);
	result.StencilEnable = stencilMode.StencilEnabled;
	result.StencilReadMask = stencilMode.ReadMask;
	result.StencilWriteMask = stencilMode.WriteMask;
	result.FrontFace = TranslateFaceOperation(stencilMode.FrontFace);
	result.BackFace = TranslateFaceOperation(stencilMode.BackFace);

	return result;
}

void DepthStencilStateDX11::Bind()
{
	if (m_isdirty)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = TranslateDepthStencilState(m_depthmode, m_stencilmode);

		HRESULT result = Game::Engine->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &m_depthstencilstate);
		if (FAILED(result))
		{
			Game::Log << LOG_ERROR << "Failed to create depth stencil state during binding (hr=" << result << ")\n";
		}

		m_isdirty = false;
	}

	Game::Engine->GetDeviceContext()->OMSetDepthStencilState(m_depthstencilstate, m_stencilmode.StencilReference);
}