#include "DynamicTerrainDefinition.h"
#include "DynamicTerrain.h"


// Default constructor
DynamicTerrain::DynamicTerrain(void)
	: 
	m_dynamic_terrain_def(NULL)
{
	// Enable relevant flags on this terrain object
	m_isdynamic = true;
	m_dataenabled = true;
	m_usable = true;
}

// Static method to instantiate a new dynamic terrain object based upon its string definition code.  Shortcut
// to avoid calling through the definition object itself
DynamicTerrain * DynamicTerrain::Create(const std::string & code)
{
	const DynamicTerrainDefinition *def = D::DynamicTerrainDefinitions.Get(code);
	if (def != NULL)
	{
		return def->Create();
	}

	return NULL;
}


// Event raised when an entity tries to interact with this object
bool DynamicTerrain::OnUsed(iObject *user)
{
	// Default behaviour if none is set by subclasses
	return false;
}