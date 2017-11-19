#include <string>
#include "Utility.h"
#include "DynamicTerrainInteractionType.h"

// Translate to an interaction type from its string representation
DynamicTerrainInteractionType TranslateDynamicTerrainInteractionTypeFromString(const std::string & interaction_type)
{
	std::string s = interaction_type;
	StrLowerC(s);

	if (s == "none")			return DynamicTerrainInteractionType::None;
	else if (s == "extended")	return DynamicTerrainInteractionType::Extended;
	else						return DynamicTerrainInteractionType::Normal;
}

// Translate an interaction type to its string representation
std::string TranslateDynamicTerrainInteractionTypeToString(DynamicTerrainInteractionType interaction_type)
{
	switch (interaction_type)
	{
		case DynamicTerrainInteractionType::None:		return "None";
		case DynamicTerrainInteractionType::Extended:	return "Extended";
		default:								return "Normal";
	}
}