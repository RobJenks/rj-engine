#include "Logging.h"
#include "CoreEngine.h"
#include "BlendStateDX11.h"


BlendStateDX11::BlendStateDX11(void)
	: m_blendstate(NULL)
	, m_alphaToCoverageEnabled(false)
	, m_independentBlendEnabled(false)
	, m_samplemask(0xffffffff)
	, m_constblendfactor(1.0f, 1.0f, 1.0f, 1.0f)
	, m_isdirty(true)
{
}

BlendStateDX11::BlendStateDX11(const BlendStateDX11& copy)
	: m_blendstate(NULL)
	, m_blendmodes(copy.m_blendmodes)
	, m_alphaToCoverageEnabled(copy.m_alphaToCoverageEnabled)
	, m_independentBlendEnabled(copy.m_independentBlendEnabled)
	, m_samplemask(copy.m_samplemask)
	, m_constblendfactor(copy.m_constblendfactor)
	, m_isdirty(true)
{
}

BlendStateDX11::~BlendStateDX11()
{
	ReleaseIfExists(m_blendstate);
}

const BlendStateDX11& BlendStateDX11::operator=(const BlendStateDX11& other)
{
	if (this != &other)
	{
		m_blendmodes = other.m_blendmodes;
		m_alphaToCoverageEnabled = other.m_alphaToCoverageEnabled;
		m_independentBlendEnabled = other.m_independentBlendEnabled;
		m_samplemask = other.m_samplemask;
		m_constblendfactor = other.m_constblendfactor;

		m_isdirty = true;
	}

	return *this;
}

void BlendStateDX11::SetBlendMode(const BlendState::BlendMode& blendMode)
{
	m_blendmodes[0] = blendMode;
	m_isdirty = true;
}

void BlendStateDX11::SetBlendMode(const BlendState::BlendMode& blendMode, BlendStateDX11::TBlendModeArray::size_type index)
{
	assert(index < Rendering::MaxRenderTargets);

	m_blendmodes[index] = blendMode;
	m_isdirty = true;
}

void BlendStateDX11::SetBlendModes(const TBlendModeArray & blendModes)
{
	m_blendmodes = blendModes;
	m_isdirty = true;
}

const BlendStateDX11::TBlendModeArray & BlendStateDX11::GetBlendModes() const
{
	return m_blendmodes;
}

void BlendStateDX11::SetAlphaCoverage(bool enabled)
{
	if (m_alphaToCoverageEnabled != enabled)
	{
		m_alphaToCoverageEnabled = enabled;
		m_isdirty = true;
	}

}

bool BlendStateDX11::GetAlphaCoverage() const
{
	return m_alphaToCoverageEnabled;
}

void BlendStateDX11::SetIndependentBlend(bool enabled)
{
	if (m_alphaToCoverageEnabled != enabled)
	{
		m_alphaToCoverageEnabled = enabled;
		m_isdirty = true;
	}
}

bool BlendStateDX11::GetIndependentBlend() const
{
	return m_independentBlendEnabled;
}

void BlendStateDX11::SetConstBlendFactor(const XMFLOAT4 & constantBlendFactor)
{
	m_constblendfactor = constantBlendFactor;
	// No need to set the dirty flag as this value is not used to create the blend state object.
	// It is only used when activating the blend state of the output merger.
}

const XMFLOAT4& BlendStateDX11::GetConstBlendFactor() const
{
	return m_constblendfactor;
}

void BlendStateDX11::SetSampleMask(uint32_t sampleMask)
{
	m_samplemask = sampleMask;
	// No need to set the dirty flag as this value is not used to create the blend state object.
	// It is only used when activating the blend state of the output merger.
}

uint32_t BlendStateDX11::GetSampleMask() const
{
	return m_samplemask;
}

D3D11_BLEND BlendStateDX11::TranslateBlendFactor(BlendState::BlendFactor blendFactor) const
{
	D3D11_BLEND result = D3D11_BLEND_ONE;

	switch (blendFactor)
	{
	case BlendFactor::Zero:
		result = D3D11_BLEND_ZERO;
		break;
	case BlendFactor::One:
		result = D3D11_BLEND_ONE;
		break;
	case BlendFactor::SrcColor:
		result = D3D11_BLEND_SRC_COLOR;
		break;
	case BlendFactor::OneMinusSrcColor:
		result = D3D11_BLEND_INV_SRC_COLOR;
		break;
	case BlendFactor::DstColor:
		result = D3D11_BLEND_DEST_COLOR;
		break;
	case BlendFactor::OneMinusDstColor:
		result = D3D11_BLEND_INV_DEST_COLOR;
		break;
	case BlendFactor::SrcAlpha:
		result = D3D11_BLEND_SRC_ALPHA;
		break;
	case BlendFactor::OneMinusSrcAlpha:
		result = D3D11_BLEND_INV_SRC_ALPHA;
		break;
	case BlendFactor::DstAlpha:
		result = D3D11_BLEND_DEST_ALPHA;
		break;
	case BlendFactor::OneMinusDstAlpha:
		result = D3D11_BLEND_INV_DEST_ALPHA;
		break;
	case BlendFactor::SrcAlphaSat:
		result = D3D11_BLEND_SRC_ALPHA_SAT;
		break;
	case BlendFactor::ConstBlendFactor:
		result = D3D11_BLEND_BLEND_FACTOR;
		break;
	case BlendFactor::OneMinusBlendFactor:
		result = D3D11_BLEND_INV_BLEND_FACTOR;
		break;
	case BlendFactor::Src1Color:
		result = D3D11_BLEND_SRC1_COLOR;
		break;
	case BlendFactor::OneMinusSrc1Color:
		result = D3D11_BLEND_INV_SRC1_COLOR;
		break;
	case BlendFactor::Src1Alpha:
		result = D3D11_BLEND_INV_SRC1_ALPHA;
		break;
	case BlendFactor::OneMinusSrc1Alpha:
		result = D3D11_BLEND_INV_SRC1_ALPHA;
		break;
	default:
		Game::Log << LOG_ERROR << ("Unknown blend factor\n");
		break;
	}

	return result;
}

D3D11_BLEND_OP BlendStateDX11::TranslateBlendOp(BlendState::BlendOperation blendOperation) const
{
	D3D11_BLEND_OP result = D3D11_BLEND_OP_ADD;
	switch (blendOperation)
	{
	case BlendOperation::Add:
		result = D3D11_BLEND_OP_ADD;
		break;
	case BlendOperation::Subtract:
		result = D3D11_BLEND_OP_SUBTRACT;
		break;
	case BlendOperation::ReverseSubtract:
		result = D3D11_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendOperation::Min:
		result = D3D11_BLEND_OP_MIN;
		break;
	case BlendOperation::Max:
		result = D3D11_BLEND_OP_MAX;
		break;
	default:
		Game::Log << LOG_ERROR << ("Unknown blend operation\n");
		break;
	}

	return result;
}

UINT8 BlendStateDX11::TranslateWriteMask(bool red, bool green, bool blue, bool alpha) const
{
	UINT8 writeMask = 0;
	if (red)
	{
		writeMask |= D3D11_COLOR_WRITE_ENABLE_RED;
	}
	if (green)
	{
		writeMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	}
	if (blue)
	{
		writeMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	}
	if (alpha)
	{
		writeMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	}

	return writeMask;
}

D3D11_LOGIC_OP BlendStateDX11::TranslateLogicOperator(LogicOperator logicOp) const
{
	D3D11_LOGIC_OP result = D3D11_LOGIC_OP_NOOP;

	switch (logicOp)
	{
	case LogicOperator::None:
		result = D3D11_LOGIC_OP_NOOP;
		break;
	case LogicOperator::Clear:
		result = D3D11_LOGIC_OP_CLEAR;
		break;
	case LogicOperator::Set:
		result = D3D11_LOGIC_OP_SET;
		break;
	case LogicOperator::Copy:
		result = D3D11_LOGIC_OP_SET;
		break;
	case LogicOperator::CopyInverted:
		result = D3D11_LOGIC_OP_COPY_INVERTED;
		break;
	case LogicOperator::Invert:
		result = D3D11_LOGIC_OP_INVERT;
		break;
	case LogicOperator::And:
		result = D3D11_LOGIC_OP_AND;
		break;
	case LogicOperator::Nand:
		result = D3D11_LOGIC_OP_NAND;
		break;
	case LogicOperator::Or:
		result = D3D11_LOGIC_OP_OR;
		break;
	case LogicOperator::Nor:
		result = D3D11_LOGIC_OP_NOR;
		break;
	case LogicOperator::Xor:
		result = D3D11_LOGIC_OP_XOR;
		break;
	case LogicOperator::Equiv:
		result = D3D11_LOGIC_OP_EQUIV;
		break;
	case LogicOperator::AndReverse:
		result = D3D11_LOGIC_OP_AND_REVERSE;
		break;
	case LogicOperator::AndInverted:
		result = D3D11_LOGIC_OP_AND_INVERTED;
		break;
	case LogicOperator::OrReverse:
		result = D3D11_LOGIC_OP_OR_REVERSE;
		break;
	case LogicOperator::OrInverted:
		result = D3D11_LOGIC_OP_OR_INVERTED;
		break;
	default:
		break;
	}

	return result;
}

void BlendStateDX11::Bind()
{
	if (m_isdirty)
	{
		// (Re)create the blend state object.
		D3D11_BLEND_DESC1 blendDesc;

		blendDesc.AlphaToCoverageEnable = m_alphaToCoverageEnabled;
		blendDesc.IndependentBlendEnable = m_independentBlendEnabled;
		for (size_t i = 0; i < Rendering::MaxRenderTargets; ++i)
		{
			D3D11_RENDER_TARGET_BLEND_DESC1& rtBlendDesc = blendDesc.RenderTarget[i];
			BlendMode& blendMode = m_blendmodes[i];

			rtBlendDesc.BlendEnable = blendMode.BlendEnabled;
			rtBlendDesc.LogicOpEnable = blendMode.LogicOpEnabled;
			rtBlendDesc.SrcBlend = TranslateBlendFactor(blendMode.SrcFactor);
			rtBlendDesc.DestBlend = TranslateBlendFactor(blendMode.DstFactor);
			rtBlendDesc.BlendOp = TranslateBlendOp(blendMode.BlendOp);
			rtBlendDesc.SrcBlendAlpha = TranslateBlendFactor(blendMode.SrcAlphaFactor);
			rtBlendDesc.DestBlendAlpha = TranslateBlendFactor(blendMode.DstAlphaFactor);
			rtBlendDesc.BlendOpAlpha = TranslateBlendOp(blendMode.AlphaOp);
			rtBlendDesc.LogicOp = TranslateLogicOperator(blendMode.LogicOp);
			rtBlendDesc.RenderTargetWriteMask = TranslateWriteMask(blendMode.WriteRed, blendMode.WriteGreen, blendMode.WriteBlue, blendMode.WriteAlpha);
		}

		Game::Engine->GetDevice()->CreateBlendState1(&blendDesc, &m_blendstate);

		m_isdirty = false;
	}
	
	// Activate the blend state
	Game::Engine->GetDeviceContext()->OMSetBlendState(m_blendstate, ValuePtr(m_constblendfactor), m_samplemask);
}

