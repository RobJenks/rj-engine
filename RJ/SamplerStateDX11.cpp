#include "Utility.h"
#include "Logging.h"
#include "DX11_Core.h"
#include "CoreEngine.h"
#include "SamplerStateDX11.h"


// Initialise static data
ID3D11SamplerState * const SamplerStateDX11::null_sampler_state[1] = { nullptr };


SamplerStateDX11::SamplerStateDX11(void)
	: m_MinFilter(MinFilter::MinNearest)
	, m_MagFilter(MagFilter::MagNearest)
	, m_MipFilter(MipFilter::MipNearest)
	, m_WrapModeU(WrapMode::Repeat)
	, m_WrapModeV(WrapMode::Repeat)
	, m_WrapModeW(WrapMode::Repeat)
	, m_CompareMode(CompareMode::None)
	, m_CompareFunc(CompareFunc::Always)
	, m_fLODBias(0.0f)
	, m_fMinLOD(0.0f)
	, m_fMaxLOD(D3D11_FLOAT32_MAX)
	, m_isAnisotropicFilteringEnabled(false)
	, m_AnisotropicFiltering(1)
	, m_isdirty(true)
{
	m_samplerstate[0] = nullptr;
}

SamplerStateDX11::~SamplerStateDX11()
{
}

void SamplerStateDX11::SetFilter(MinFilter minFilter, MagFilter magFilter, MipFilter mipFilter)
{
	m_MinFilter = minFilter;
	m_MagFilter = magFilter;
	m_MipFilter = mipFilter;
	m_isdirty = true;
}

void SamplerStateDX11::GetFilter(MinFilter& minFilter, MagFilter& magFilter, MipFilter& mipFilter) const
{
	minFilter = m_MinFilter;
	magFilter = m_MagFilter;
	mipFilter = m_MipFilter;
}

void SamplerStateDX11::SetWrapMode(WrapMode u, WrapMode v, WrapMode w)
{
	m_WrapModeU = u;
	m_WrapModeV = v;
	m_WrapModeW = w;
	m_isdirty = true;
}

void SamplerStateDX11::GetWrapMode(WrapMode& u, WrapMode& v, WrapMode& w) const
{
	u = m_WrapModeU;
	v = m_WrapModeV;
	w = m_WrapModeW;
}

void SamplerStateDX11::SetCompareMode(CompareMode compareMode)
{
	m_CompareMode = compareMode;
	m_isdirty = true;
}

SamplerState::CompareMode SamplerStateDX11::GetCompareMode(void) const
{
	return m_CompareMode;
}

void SamplerStateDX11::SetCompareFunction(CompareFunc compareFunc)
{
	m_CompareFunc = compareFunc;
	m_isdirty = true;
}

SamplerState::CompareFunc SamplerStateDX11::GetCompareFunc() const
{
	return m_CompareFunc;
}

void SamplerStateDX11::SetLODBias(float lodBias)
{
	m_fLODBias = lodBias;
	m_isdirty = true;
}

float SamplerStateDX11::GetLODBias() const
{
	return m_fLODBias;
}

void SamplerStateDX11::SetMinLOD(float minLOD)
{
	m_fMinLOD = minLOD;
	m_isdirty = true;
}

float SamplerStateDX11::GetMinLOD() const
{
	return m_fMinLOD;
}

void SamplerStateDX11::SetMaxLOD(float maxLOD)
{
	m_fMaxLOD = maxLOD;
	m_isdirty = true;
}

float SamplerStateDX11::GetMaxLOD() const
{
	return m_fMaxLOD;
}

void SamplerStateDX11::SetBorderColor(const XMFLOAT4 & borderColor)
{
	m_BorderColor = borderColor;
	m_isdirty = true;
}

XMFLOAT4 SamplerStateDX11::GetBorderColor() const
{
	return m_BorderColor;
}

void SamplerStateDX11::EnableAnisotropicFiltering(bool enabled)
{
	m_isAnisotropicFilteringEnabled = enabled;
	m_isdirty = true;
}

bool SamplerStateDX11::IsAnisotropicFilteringEnabled() const
{
	return m_isAnisotropicFilteringEnabled;
}


void SamplerStateDX11::SetMaxAnisotropy(uint8_t maxAnisotropy)
{
	m_AnisotropicFiltering = clamp(maxAnisotropy, (uint8_t)1, (uint8_t)16);
	m_isdirty = true;
}

uint8_t SamplerStateDX11::GetMaxAnisotropy() const
{
	return m_AnisotropicFiltering;
}

D3D11_FILTER SamplerStateDX11::TranslateFilter() const
{
	D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	if (m_isAnisotropicFilteringEnabled)
	{
		filter = (m_CompareMode == CompareMode::None) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_COMPARISON_ANISOTROPIC;
		return filter;
	}

	if (m_MinFilter == MinFilter::MinNearest && m_MagFilter == MagFilter::MagNearest && m_MipFilter == MipFilter::MipNearest)
	{
		filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	else if (m_MinFilter == MinFilter::MinNearest && m_MagFilter == MagFilter::MagNearest && m_MipFilter == MipFilter::MipLinear)
	{
		filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	}
	else if (m_MinFilter == MinFilter::MinNearest && m_MagFilter == MagFilter::MagLinear && m_MipFilter == MipFilter::MipNearest)
	{
		filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	}
	else if (m_MinFilter == MinFilter::MinNearest && m_MagFilter == MagFilter::MagLinear && m_MipFilter == MipFilter::MipLinear)
	{
		filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	}
	else if (m_MinFilter == MinFilter::MinLinear && m_MagFilter == MagFilter::MagNearest && m_MipFilter == MipFilter::MipNearest)
	{
		filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	}
	else if (m_MinFilter == MinFilter::MinLinear && m_MagFilter == MagFilter::MagNearest && m_MipFilter == MipFilter::MipLinear)
	{
		filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	}
	else if (m_MinFilter == MinFilter::MinLinear && m_MagFilter == MagFilter::MagLinear && m_MipFilter == MipFilter::MipNearest)
	{
		filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	}
	else if (m_MinFilter == MinFilter::MinLinear && m_MagFilter == MagFilter::MagLinear && m_MipFilter == MipFilter::MipLinear)
	{
		filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
	else
	{
		Game::Log << LOG_ERROR << ("Invalid texture filter modes.");
	}

	if (m_CompareMode != CompareMode::None)
	{
		*(reinterpret_cast<int*>(&filter)) += static_cast<int>(D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT);
	}

	return filter;
}

D3D11_TEXTURE_ADDRESS_MODE SamplerStateDX11::TranslateWrapMode(WrapMode wrapMode) const
{
	D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_WRAP;

	switch (wrapMode)
	{
	case WrapMode::Repeat:
		addressMode = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case WrapMode::Clamp:
		addressMode = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case WrapMode::Mirror:
		addressMode = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	case WrapMode::Border:
		addressMode = D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	}

	return addressMode;
}

D3D11_COMPARISON_FUNC SamplerStateDX11::TranslateComparisonFunction(CompareFunc compareFunc) const
{
	D3D11_COMPARISON_FUNC compareFuncD3D11 = D3D11_COMPARISON_ALWAYS;
	switch (compareFunc)
	{
	case CompareFunc::Never:
		compareFuncD3D11 = D3D11_COMPARISON_NEVER;
		break;
	case CompareFunc::Less:
		compareFuncD3D11 = D3D11_COMPARISON_LESS;
		break;
	case CompareFunc::Equal:
		compareFuncD3D11 = D3D11_COMPARISON_EQUAL;
		break;
	case CompareFunc::LessEqual:
		compareFuncD3D11 = D3D11_COMPARISON_LESS_EQUAL;
		break;
	case CompareFunc::Greater:
		compareFuncD3D11 = D3D11_COMPARISON_GREATER;
		break;
	case CompareFunc::NotEqual:
		compareFuncD3D11 = D3D11_COMPARISON_NOT_EQUAL;
		break;
	case CompareFunc::GreaterEqual:
		compareFuncD3D11 = D3D11_COMPARISON_GREATER_EQUAL;
		break;
	case CompareFunc::Always:
		compareFuncD3D11 = D3D11_COMPARISON_ALWAYS;
		break;
	}

	return compareFuncD3D11;
}

void SamplerStateDX11::Bind(Shader::Type shadertype, Shader::SlotID slot_id)
{
	if (m_isdirty || m_samplerstate[0] == nullptr)
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = TranslateFilter();
		samplerDesc.AddressU = TranslateWrapMode(m_WrapModeU);
		samplerDesc.AddressV = TranslateWrapMode(m_WrapModeV);
		samplerDesc.AddressW = TranslateWrapMode(m_WrapModeW);
		samplerDesc.MipLODBias = m_fLODBias;
		samplerDesc.MaxAnisotropy = m_AnisotropicFiltering;
		samplerDesc.ComparisonFunc = TranslateComparisonFunction(m_CompareFunc);
		samplerDesc.BorderColor[0] = m_BorderColor.x; // r
		samplerDesc.BorderColor[1] = m_BorderColor.y; // g
		samplerDesc.BorderColor[2] = m_BorderColor.z; // b
		samplerDesc.BorderColor[3] = m_BorderColor.w; // a
		samplerDesc.MinLOD = m_fMinLOD;
		samplerDesc.MaxLOD = m_fMaxLOD;

		Game::Engine->GetDevice()->CreateSamplerState(&samplerDesc, &m_samplerstate[0]);

		m_isdirty = false;
	}

	switch (shadertype)
	{
		case Shader::Type::VertexShader:
			Game::Engine->GetDeviceContext()->VSSetSamplers(slot_id, 1, m_samplerstate);
			break;
		case Shader::Type::HullShader:
			Game::Engine->GetDeviceContext()->HSSetSamplers(slot_id, 1, m_samplerstate);
			break;
		case Shader::Type::DomainShader:
			Game::Engine->GetDeviceContext()->DSSetSamplers(slot_id, 1, m_samplerstate);
			break;
		case Shader::Type::GeometryShader:
			Game::Engine->GetDeviceContext()->GSSetSamplers(slot_id, 1, m_samplerstate);
			break;
		case Shader::Type::PixelShader:
			Game::Engine->GetDeviceContext()->PSSetSamplers(slot_id, 1, m_samplerstate);
			break;
		case Shader::Type::ComputeShader:
			Game::Engine->GetDeviceContext()->CSSetSamplers(slot_id, 1, m_samplerstate);
			break;
	}
}

void SamplerStateDX11::Unbind(Shader::Type shadertype, Shader::SlotID slot_id)
{
	switch (shadertype)
	{
		case Shader::Type::VertexShader:
			Game::Engine->GetDeviceContext()->VSSetSamplers(slot_id, 1, null_sampler_state);
			break;
		case Shader::Type::HullShader:
			Game::Engine->GetDeviceContext()->HSSetSamplers(slot_id, 1, null_sampler_state);
			break;
		case Shader::Type::DomainShader:
			Game::Engine->GetDeviceContext()->DSSetSamplers(slot_id, 1, null_sampler_state);
			break;
		case Shader::Type::GeometryShader:
			Game::Engine->GetDeviceContext()->GSSetSamplers(slot_id, 1, null_sampler_state);
			break;
		case Shader::Type::PixelShader:
			Game::Engine->GetDeviceContext()->PSSetSamplers(slot_id, 1, null_sampler_state);
			break;
		case Shader::Type::ComputeShader:
			Game::Engine->GetDeviceContext()->CSSetSamplers(slot_id, 1, null_sampler_state);
			break;
	}
}