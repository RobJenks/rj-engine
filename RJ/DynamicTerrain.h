#pragma once

#include "Terrain.h"
#include "DataEnabledObject.h"
#include "UsableObject.h"

class DynamicTerrain : public Terrain, public DataEnabledObject, public UsableObject
{
public:

	// Default constructor
	DynamicTerrain(void);


private:


};
