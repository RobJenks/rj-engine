#pragma once

#ifndef __EffectBaseH__
#define __EffectBaseH__

#include "DX11_Core.h"

#include <string>
#include "ErrorCodes.h"
#include "CompilerSettings.h"
class Texture;
class DXLocaliser;
using namespace std;

class EffectBase
{
public:
	// Enumeration of the available effect models that can be used for rendering
	enum EffectModelType { UnitSquare = 0, UnitCone };

	// Utility function for loading in effect texture data
	Result					LoadTexture(Texture **texture, const char *filename);

	// Returns/sets the uniquely identifying code for this effect
	CMPINLINE string GetCode(void) { return m_code; }
	CMPINLINE void SetCode(string code) { m_code = code; }

	// Returns/sets the effect model used in rendering this effect
	CMPINLINE EffectModelType GetEffectModel(void) { return m_effectmodel; }
	CMPINLINE void SetEffectModel(EffectModelType model) { m_effectmodel = model; }
	CMPINLINE void SetEffectModel(string modelname) { m_effectmodel = DeriveEffectModel(modelname); }
	
	// Derives the effect model type from a supplied string name
	EffectBase::EffectModelType DeriveEffectModel(string modelname);

	// Constructors/destructors
	EffectBase(void);
	~EffectBase(void);

protected:
	const DXLocaliser *m_locale;			// Pointer to the DX locale, for localising e.g. shader calls
	string m_code;							// String ID that uniquely identifies the effect
	EffectModelType m_effectmodel;			// Determines the underlying effect model used to render this effect
	
};


#endif