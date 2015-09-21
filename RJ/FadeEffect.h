#pragma once

#ifndef __FadeEffectH__
#define __FadeEffectH__

#include "CompilerSettings.h"
#include "GameVarsExtern.h"

class FadeEffect
{
public:

	// Default constructor/destructor
	FadeEffect(void);
	~FadeEffect(void);

	// Methods to indicate whether the effect is active and to retrieve the alpha value
	CMPINLINE bool			IsActive(void) const		{ return m_active; }
	CMPINLINE float			GetFadeAlpha(void) const	{ return m_fade; }
	CMPINLINE bool			AlphaIsActive(void) const	{ return m_alphaactive; }

	// Methods to directly change the fade effect for this object
	CMPINLINE void			Activate(void) 				{ m_active = true; }
	CMPINLINE void			Deactivate(void) 			{ m_active = false; }
	CMPINLINE void			SetFadeAlpha(float alpha)	
	{
		// Store the new alpha value
		m_fade = alpha; 

		// Set the alpha-active flag if this puts us in a non-standard state, i.e. if (m_fade < (1.0f - epsilon))
		m_alphaactive = (m_fade < ALPHA_THRESHOLD);
	}

	// Methods to begin a fade in or out from current alpha value over the specified period of time
	CMPINLINE void			FadeOut(float timeperiod)	{ InitialiseFade(timeperiod, m_fade, 0.0f); }
	CMPINLINE void			FadeIn(float timeperiod)	{ InitialiseFade(timeperiod, m_fade, 1.0f); }

	// Method to begin a fade in or out from current alpha value, to another alpha value, over the specified period of time
	CMPINLINE void			FadeToAlpha(float timeperiod, float targetalpha)	{ InitialiseFade(timeperiod, m_fade, targetalpha); }

	// Update method, called each frame (or less frequently if object is not visible; will still work).  Updates the fade effect if it is active
	CMPINLINE void Update(void)
	{
        // This method should only be called where m_active == true; safety check here
		if (!m_active) return;

		// Update progress through the fade effect.  If we are done then end the effect
		if (Game::ClockMs >= m_effectend)
		{
			// Set alpha to the target end value and deactivate the effect
			SetFadeAlpha(m_alphaend);
			m_active = false;
			return;
		}

		// If the effect is still active then set the alpha value proportionate to our progress through the effect time
		SetFadeAlpha(m_alphastart + (((float)(Game::ClockMs - m_effectstart) / m_effecttime) * m_alphachange));
	}


private:

	// Method to begin a fade effect from one alpha value to another over the specified period of time
	void					InitialiseFade(float timeperiod, float startalpha, float targetalpha);

	// Private member variables
	bool					m_active;					// Flag indicating whether the fade effect is active
	float					m_fade;						// Float alpha value that signifies the level of visibility (0.0 = full, 1.0 = none)
	bool					m_alphaactive;				// Flag indicating whether any non-standard alpha effect is active, i.e. if (m_fade > (1.0-epsilon))

	unsigned int			m_effectstart;				// The time that the effect was started (clock ms)
	float					m_effecttime;				// The time that this effect should run for (ms).  'float' to save casting each update
	unsigned int			m_effectend;				// The time that this effect will end (clock ms)
	float					m_alphastart;				// The alpha value that the effect began from
	float					m_alphaend;					// The alpha value that the effect should end at
	float					m_alphachange;				// The change in alpha that will be applied over the lifetime of the effect

	// Static constant value; (1.0f-epsilon) will be the threshold above which we consider no alpha effect to be active
	static const float		ALPHA_THRESHOLD;
};


#endif