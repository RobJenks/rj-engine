#pragma once

#ifndef __FireEffectH__
#define __FireEffectH__

#include "EffectBase.h"
#include "Texture.h"
class DXLocaliser;

class FireEffect : public EffectBase
{
public:
	
	FireEffect(const DXLocaliser *locale);
	~FireEffect(void);

	Result Initialise(void);

	void Shutdown(void);

	Result	SetFireTexture(const char *texture);
	Result	SetNoiseTexture(const char *texture);
	Result	SetAlphaTexture(const char *texture);
	void	SetScrollSpeeds(const D3DXVECTOR3 &scrollspeed);
	void	SetScaling(const D3DXVECTOR3 &scaling);
	void	SetDistortionParameters1(const D3DXVECTOR2 &distortion);
	void	SetDistortionParameters2(const D3DXVECTOR2 &distortion);
	void	SetDistortionParameters3(const D3DXVECTOR2 &distortion);
	void	SetDistortionScale(float dscale);
	void	SetDistortionBias(float dbias);

	CMPINLINE ID3D11ShaderResourceView		*GetFireTexture(void) { return m_firetexture->GetTexture(); }
	CMPINLINE ID3D11ShaderResourceView		*GetNoiseTexture(void) { return m_noisetexture->GetTexture(); }
	CMPINLINE ID3D11ShaderResourceView		*GetAlphaTexture(void) { return m_alphatexture->GetTexture(); }
	CMPINLINE D3DXVECTOR3					 GetScrollSpeeds(void) { return m_scrollspeeds; }
	CMPINLINE D3DXVECTOR3					 GetScaling(void) { return m_scaling; }
	CMPINLINE D3DXVECTOR2					 GetDistortionParameters1(void) { return m_distortion1; }
	CMPINLINE D3DXVECTOR2					 GetDistortionParameters2(void) { return m_distortion2; }
	CMPINLINE D3DXVECTOR2					 GetDistortionParameters3(void) { return m_distortion3; }
	CMPINLINE float							 GetDistortionScale(void) { return m_distortionscale; }
	CMPINLINE float							 GetDistortionBias(void) { return m_distortionbias; }
	



private:
	Texture							*m_firetexture;
	Texture							*m_noisetexture;
	Texture							*m_alphatexture;
	D3DXVECTOR3						 m_scrollspeeds;
	D3DXVECTOR3						 m_scaling;
	D3DXVECTOR2						 m_distortion1; 
	D3DXVECTOR2						 m_distortion2; 
	D3DXVECTOR2						 m_distortion3; 
	float							 m_distortionscale;
	float							 m_distortionbias;


};



#endif