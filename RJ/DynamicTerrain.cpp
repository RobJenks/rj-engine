#include "DynamicTerrain.h"


// Default constructor
DynamicTerrain::DynamicTerrain(void)
{
	// Enable relevant flags on this terrain object
	m_isdynamic = true;
	m_dataenabled = true;
	m_usable = true;
}


// Event raised when an entity tries to interact with this object
bool DynamicTerrain::OnUsed(iObject *user)
{
	// Default behaviour if none is set by subclasses
	return true;
}