#include <string>
#include "Utility.h"
#include "EffectBase.h"


// Derives the effect model type from a string identifier.  Used for loading in from game XML data
EffectBase::EffectModelType EffectBase::DeriveEffectModel(const std::string & modelname)
{
	// Convert to lowercase for case-insensitive comparison
	std::string s = StrLower(modelname);
	
	// Now determine the effect model type this string is referring to
	if (s == "unit_cone")
		return EffectModelType::UnitCone;
	/* elseif .... */
		/* other options.... */
	else 
		/* Default option is the unit square */
		return EffectModelType::UnitSquare;
}

EffectBase::EffectBase(void)
{
	// Set variables to NULL prior to initialisation
	m_code = "";
	m_effectmodel = EffectModelType::UnitSquare;
}

EffectBase::~EffectBase(void)
{
}
