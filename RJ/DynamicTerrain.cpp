#include "DynamicTerrain.h"


// Default constructor
DynamicTerrain::DynamicTerrain(void)
	:
	m_dynamic_terrain_code(NullString)
{
	// Enable relevant flags on this terrain object
	m_isdynamic = true;
	m_dataenabled = true;
	m_usable = true;
}

// Static method to instantiate a new dynamic terrain object based upon its string code
DynamicTerrain * DynamicTerrain::Create(const std::string & code)
{
	return D::DynamicTerrainDefinitions.Get(code);
}


// Event raised when an entity tries to interact with this object
bool DynamicTerrain::OnUsed(iObject *user)
{
	// Default behaviour if none is set by subclasses
	return false;
}