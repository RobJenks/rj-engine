#include "GameVarsExtern.h"
#include "FadeEffect.h"

// Initialise static threshold value, above which we consider no alpha effect to be active
const float FadeEffect::ALPHA_THRESHOLD = (1.0f - Game::C_EPSILON);

// Default constructor
FadeEffect::FadeEffect(void)
{
	// Initialise values to their defaults
	m_active = m_alphaactive = false;
	m_fade = 1.0f;
	
	m_effectstart = m_effectend = 0;
	m_effecttime = 0.0f;
	m_alphastart = m_alphaend = m_alphachange = 0.0f;
}

// Method to begin a fade effect from current alpha value over the specified period of time
void FadeEffect::InitialiseFade(float timeperiod, float startalpha, float targetalpha)
{
	// Set the effect to active and initialise the timing parameters
	m_active = true;
	m_effectstart = Game::ClockMs;
	m_effecttime = (timeperiod * 1000.0f);
	m_effectend = (m_effectstart + (unsigned int)m_effecttime);

	// Set the alpha start/end values and calculate the change in alpha to be applied over the lifetime of the effect
	m_alphastart = startalpha;
	m_alphaend = targetalpha;
	m_alphachange = (m_alphaend - m_alphastart);
}


// Default destructor
FadeEffect::~FadeEffect(void)
{

}
