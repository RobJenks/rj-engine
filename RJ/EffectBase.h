#pragma once

#ifndef __EffectBaseH__
#define __EffectBaseH__

#include "DX11_Core.h"

#include <string>
#include "ErrorCodes.h"
#include "CompilerSettings.h"

// This class has no special alignment requirements
class EffectBase
{
public:
	// Enumeration of the available effect models that can be used for rendering
	enum EffectModelType { UnitSquare = 0, UnitCone };

	// Returns/sets the uniquely identifying code for this effect
	CMPINLINE std::string GetCode(void) { return m_code; }
	CMPINLINE void SetCode(std::string code) { m_code = code; }

	// Returns/sets the effect model used in rendering this effect
	CMPINLINE EffectModelType GetEffectModel(void) { return m_effectmodel; }
	CMPINLINE void SetEffectModel(EffectModelType model) { m_effectmodel = model; }
	CMPINLINE void SetEffectModel(std::string modelname) { m_effectmodel = DeriveEffectModel(modelname); }
	
	// Derives the effect model type from a supplied string name
	EffectBase::EffectModelType DeriveEffectModel(std::string & modelname);

	// Constructors/destructors
	EffectBase(void);
	~EffectBase(void);

protected:
	std::string m_code;						// String ID that uniquely identifies the effect
	EffectModelType m_effectmodel;			// Determines the underlying effect model used to render this effect
	
};


#endif