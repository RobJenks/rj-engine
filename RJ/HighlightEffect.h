#pragma once

#ifndef __HighlightEffectH__
#define __HighlightEffectH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "FastMath.h"
#include "GameVarsExtern.h"


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class HighlightEffect : public ALIGN16<HighlightEffect>
{
public:

	// Default constructor
	HighlightEffect(void)
	{
		m_active = false;
		m_colour = XMVectorReplicate(1.0f);
	}

	// Change the activation state of this effect
	CMPINLINE bool			IsActive(void)								{ return m_active; }
	CMPINLINE void			Activate(void)								{ m_active = true; }
	CMPINLINE void			Deactivate(void)							{ m_active = false; }
	CMPINLINE void			SetActiveState(bool active)					{ m_active = active; }

	// Retrieve or set the colour to be used in this highlight effect
	CMPINLINE XMVECTOR		GetColour(void)								{ return m_colour; }
	CMPINLINE void			SetColour(const FXMVECTOR colour)			{ m_colour = colour; }

	// Default destructor
	~HighlightEffect(void) { }


protected:

	bool					m_active;		// Flag indicating whether highlight effect is active
	AXMVECTOR				m_colour;		// Colour to be applied when highlighting

};





#endif





