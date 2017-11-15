#pragma once

#include <string>

enum PlayerInteractionType
{
	None = 0,					// No interaction is possible

	Normal,						// Single interaction, i.e. a press of the interaction key

	Extended					// Extended interaction, i.e. holding the interaction key while taking some 
								// other actions, e.g. moving the mouse
};

// Translate to an interaction type from its string representation
extern PlayerInteractionType TranslatePlayerInteractionTypeFromString(const std::string & interaction_type);

// Translate an interaction type to its string representation
extern std::string TranslatePlayerInteractionTypeToString(PlayerInteractionType interaction_type);