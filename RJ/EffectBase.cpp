#include <string>
#include "Texture.h"
#include "Utility.h"
#include "EffectBase.h"

Result EffectBase::LoadTexture(Texture **texture, const char *filename)
{
	// Create the texture object
	(*texture) = new Texture();
	if(!(*texture)) return ErrorCodes::CouldNotCreateTextureObject;

	// Initialize the texture object
	Result result = (*texture)->Initialise(filename);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we loaded the texture successfully
	return ErrorCodes::NoError;
}

// Derives the effect model type from a string identifier.  Used for loading in from game XML data
EffectBase::EffectModelType EffectBase::DeriveEffectModel(const string modelname)
{
	// Convert to lowercase for case-insensitive comparison
	string s = StrLower(modelname);
	
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
	m_locale = NULL;
	m_code = "";
	m_effectmodel = EffectModelType::UnitSquare;
}

EffectBase::~EffectBase(void)
{
}
