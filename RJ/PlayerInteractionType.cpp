#include <string>
#include "Utility.h"
#include "PlayerInteractionType.h"

// Translate to an interaction type from its string representation
PlayerInteractionType TranslatePlayerInteractionTypeFromString(const std::string & interaction_type)
{
	std::string s = interaction_type;
	StrLowerC(s);

	if (s == "none")			return PlayerInteractionType::None;
	else if (s == "extended")	return PlayerInteractionType::Extended;
	else						return PlayerInteractionType::Normal;
}

// Translate an interaction type to its string representation
std::string TranslatePlayerInteractionTypeToString(PlayerInteractionType interaction_type)
{
	switch (interaction_type)
	{
		case PlayerInteractionType::None:		return "None";
		case PlayerInteractionType::Extended:	return "Extended";
		default:								return "Normal";
	}
}