#pragma once

#include <string>

enum DynamicTerrainInteractionType
{
	None = 0,					// No interaction is possible

	Normal,						// Single interaction, e.g. a press of a button

	Extended					// Extended interaction, i.e. moving a control that has continuous range of 
								// values.  In the case of the player, this includes  holding the interaction 
								// key while taking some other actions, e.g. moving the mouse
};

// Translate to an interaction type from its string representation
extern DynamicTerrainInteractionType TranslateDynamicTerrainInteractionTypeFromString(const std::string & interaction_type);

// Translate an interaction type to its string representation
extern std::string TranslateDynamicTerrainInteractionTypeToString(DynamicTerrainInteractionType interaction_type);