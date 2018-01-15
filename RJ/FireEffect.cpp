#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"

#include "FireEffect.h"


Result FireEffect::Initialise(void)
{
	// Nothing to initialise, for now
	return ErrorCodes::NoError;
}

// Loads the fire texture from file
Result FireEffect::SetFireTexture(const std::string & texture)
{
	m_firetexture = Game::Engine->GetAssets().GetTexture(texture);
	return ErrorCodes::NoError;
}

// Loads the noise texture from file
Result FireEffect::SetNoiseTexture(const std::string & texture)
{
	m_noisetexture = Game::Engine->GetAssets().GetTexture(texture);
	return ErrorCodes::NoError;
}

// Loads the alpha texture from file
Result FireEffect::SetAlphaTexture(const std::string & texture)
{
	m_alphatexture = Game::Engine->GetAssets().GetTexture(texture);
	return ErrorCodes::NoError;
}

ID3D11ShaderResourceView * FireEffect::GetFireTexture(void)
{
	return m_firetexture->GetShaderResourceView();
}

ID3D11ShaderResourceView * FireEffect::GetNoiseTexture(void)
{
	return m_noisetexture->GetShaderResourceView();
}

ID3D11ShaderResourceView * FireEffect::GetAlphaTexture(void)
{
	return m_alphatexture->GetShaderResourceView();
}


// Set other properties of the flame effect
void FireEffect::SetScrollSpeeds(const XMFLOAT3 &scrollspeed) { m_scrollspeeds = scrollspeed; }
void FireEffect::SetScaling(const XMFLOAT3 &scaling) { m_scaling = scaling; }
void FireEffect::SetDistortionParameters1(const XMFLOAT2 &distortion) { m_distortion1 = distortion; }
void FireEffect::SetDistortionParameters2(const XMFLOAT2 &distortion) { m_distortion2 = distortion; }
void FireEffect::SetDistortionParameters3(const XMFLOAT2 &distortion) { m_distortion3 = distortion; }
void FireEffect::SetDistortionScale(float dscale) { m_distortionscale = dscale; }
void FireEffect::SetDistortionBias(float dbias) { m_distortionbias = dbias; }


FireEffect::FireEffect(void)
{
	// Set values to 0/NULL prior to initialisation
	m_firetexture = 0;
	m_noisetexture = 0;
	m_alphatexture = 0;
	m_scrollspeeds = XMFLOAT3(0, 0, 0);
	m_scaling = XMFLOAT3(0, 0, 0);
	m_distortion1 = XMFLOAT2(0, 0); 
	m_distortion2 = XMFLOAT2(0, 0); 
	m_distortion3 = XMFLOAT2(0, 0); 
	m_distortionscale = 0.0f;
	m_distortionbias = 0.0f;
}

FireEffect::~FireEffect(void)
{
}
