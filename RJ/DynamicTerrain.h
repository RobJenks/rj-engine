#pragma once

#include "Terrain.h"
#include "DataEnabledObject.h"
#include "UsableObject.h"

class DynamicTerrain : public Terrain, public DataEnabledObject, public UsableObject
{
public:

	// Event raised when an entity tries to interact with this object
	virtual bool OnUsed(iObject *user);


protected:

	// Default constructor
	DynamicTerrain(void);


};
