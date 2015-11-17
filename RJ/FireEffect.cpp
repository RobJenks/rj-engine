#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Texture.h"
#include "DXLocaliser.h"

#include "FireEffect.h"


Result FireEffect::Initialise(void)
{
	// Nothing to initialise, for now
	return ErrorCodes::NoError;
}

// Loads the fire texture from file
Result FireEffect::SetFireTexture(const char *texture)
{
	return LoadTexture(&m_firetexture, texture);
}

// Loads the noise texture from file
Result FireEffect::SetNoiseTexture(const char *texture)
{
	return LoadTexture(&m_noisetexture, texture);
}

// Loads the alpha texture from file
Result FireEffect::SetAlphaTexture(const char *texture)
{
	return LoadTexture(&m_alphatexture, texture);
}

void FireEffect::Shutdown(void)
{
	// Release the fire texture object
	if (m_firetexture)
	{
		m_firetexture->Shutdown();
		delete m_firetexture;
		m_firetexture = NULL;
	}
	
	// Release the alpha texture object
	if (m_alphatexture)
	{
		m_alphatexture->Shutdown();
		delete m_alphatexture;
		m_alphatexture = NULL;
	}
	
	// Release the noise texture object
	if (m_noisetexture)
	{
		m_noisetexture->Shutdown();
		delete m_noisetexture;
		m_noisetexture = NULL;
	}
}

// Set other properties of the flame effect
void FireEffect::SetScrollSpeeds(const XMFLOAT3 &scrollspeed) { m_scrollspeeds = scrollspeed; }
void FireEffect::SetScaling(const XMFLOAT3 &scaling) { m_scaling = scaling; }
void FireEffect::SetDistortionParameters1(const XMFLOAT2 &distortion) { m_distortion1 = distortion; }
void FireEffect::SetDistortionParameters2(const XMFLOAT2 &distortion) { m_distortion2 = distortion; }
void FireEffect::SetDistortionParameters3(const XMFLOAT2 &distortion) { m_distortion3 = distortion; }
void FireEffect::SetDistortionScale(float dscale) { m_distortionscale = dscale; }
void FireEffect::SetDistortionBias(float dbias) { m_distortionbias = dbias; }


FireEffect::FireEffect(const DXLocaliser *locale)
{
	// Store reference to the DX localiser instance
	m_locale = locale;

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
