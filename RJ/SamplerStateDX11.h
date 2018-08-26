#pragma once

#include "DX11_Core.h"
#include "Shader.h"
#include "SamplerState.h"


class SamplerStateDX11 : public SamplerState
{
public:

	SamplerStateDX11(void);
	~SamplerStateDX11(void);

	void						SetFilter(MinFilter minFilter, MagFilter magFilter, MipFilter mipFilter);
	void						GetFilter(MinFilter& minFilter, MagFilter& magFilter, MipFilter& mipFilter) const;

	void						SetWrapMode(WrapMode u = WrapMode::Repeat, WrapMode v = WrapMode::Repeat, WrapMode w = WrapMode::Repeat);
	void						GetWrapMode(WrapMode& u, WrapMode& v, WrapMode& w) const;

	void						SetCompareMode(CompareMode compareMode);
	CompareMode					GetCompareMode(void) const;

	void						SetCompareFunction(CompareFunc compareFunc);
	CompareFunc					GetCompareFunc() const;

	void						SetLODBias(float lodBias);
	float						GetLODBias() const;

	void						SetMinLOD(float minLOD);
	float						GetMinLOD() const;

	void						SetMaxLOD(float maxLOD);
	float						GetMaxLOD() const;

	void						SetBorderColor(const XMFLOAT4 & borderColor);
	XMFLOAT4					GetBorderColor() const;

	void						EnableAnisotropicFiltering(bool enabled);
	bool						IsAnisotropicFilteringEnabled() const;

	void						SetMaxAnisotropy(uint8_t maxAnisotropy);
	uint8_t						GetMaxAnisotropy() const;


	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id);

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id);

private:

	D3D11_FILTER				TranslateFilter() const;
	D3D11_TEXTURE_ADDRESS_MODE	TranslateWrapMode(WrapMode wrapMode) const;
	D3D11_COMPARISON_FUNC		TranslateComparisonFunction(CompareFunc compareFunc) const;

private:

	ID3D11SamplerState *		m_samplerstate[1];

	MinFilter					m_MinFilter;
	MagFilter					m_MagFilter;
	MipFilter					m_MipFilter;
	WrapMode					m_WrapModeU, m_WrapModeV, m_WrapModeW;
	CompareMode					m_CompareMode;
	CompareFunc					m_CompareFunc;

	float						m_fLODBias;
	float						m_fMinLOD;
	float						m_fMaxLOD;

	XMFLOAT4					m_BorderColor;

	bool						m_isAnisotropicFilteringEnabled;
	uint8_t						m_AnisotropicFiltering;

	bool						m_isdirty;	// Allows sampler state changes to be reflected automatically in next Bind() call

	static ID3D11SamplerState * const null_sampler_state[1];
};