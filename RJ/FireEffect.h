#pragma once

#ifndef __FireEffectH__
#define __FireEffectH__

#include "EffectBase.h"
class TextureDX11;



// This class has no special alignment requirements
class FireEffect : public EffectBase
{
public:
	
	FireEffect(void);
	~FireEffect(void);

	Result Initialise(void);

	Result	SetFireTexture(const std::string & texture);
	Result	SetNoiseTexture(const std::string & texture);
	Result	SetAlphaTexture(const std::string & texture);
	void	SetScrollSpeeds(const XMFLOAT3 &scrollspeed);
	void	SetScaling(const XMFLOAT3 &scaling);
	void	SetDistortionParameters1(const XMFLOAT2 &distortion);
	void	SetDistortionParameters2(const XMFLOAT2 &distortion);
	void	SetDistortionParameters3(const XMFLOAT2 &distortion);
	void	SetDistortionScale(float dscale);
	void	SetDistortionBias(float dbias);

	ID3D11ShaderResourceView *				 GetFireTexture(void);
	ID3D11ShaderResourceView *				 GetNoiseTexture(void);
	ID3D11ShaderResourceView *				 GetAlphaTexture(void);
	CMPINLINE XMFLOAT3						 GetScrollSpeeds(void) { return m_scrollspeeds; }
	CMPINLINE XMFLOAT3						 GetScaling(void) { return m_scaling; }
	CMPINLINE XMFLOAT2						 GetDistortionParameters1(void) { return m_distortion1; }
	CMPINLINE XMFLOAT2						 GetDistortionParameters2(void) { return m_distortion2; }
	CMPINLINE XMFLOAT2						 GetDistortionParameters3(void) { return m_distortion3; }
	CMPINLINE float							 GetDistortionScale(void) { return m_distortionscale; }
	CMPINLINE float							 GetDistortionBias(void) { return m_distortionbias; }
	



private:
	TextureDX11 * 					 m_firetexture;
	TextureDX11 *					 m_noisetexture;
	TextureDX11 *					 m_alphatexture;
	XMFLOAT3						 m_scrollspeeds;
	XMFLOAT3						 m_scaling;
	XMFLOAT2						 m_distortion1; 
	XMFLOAT2						 m_distortion2; 
	XMFLOAT2						 m_distortion3; 
	float							 m_distortionscale;
	float							 m_distortionbias;


};



#endif