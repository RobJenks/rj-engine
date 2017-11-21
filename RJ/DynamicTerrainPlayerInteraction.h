#pragma once

#include "CompilerSettings.h"
#include "DynamicTerrainInteraction.h"

class DynamicTerrainPlayerInteraction : public DynamicTerrainInteraction
{
public:

	// Create a new interaction object
	CMPINLINE static DynamicTerrainInteraction				Normal(void) { return DynamicTerrainInteraction(DynamicTerrainInteractionType::Normal, 0.0f, true); }
	CMPINLINE static DynamicTerrainInteraction				Extended(float value) { return DynamicTerrainInteraction(DynamicTerrainInteractionType::Extended, value, true); }


};