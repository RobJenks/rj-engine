#pragma once

#include "CompilerSettings.h"
#include "DynamicTerrainInteractionType.h"


class DynamicTerrainInteraction
{
public:

	// Convenience method to create a new interaction of the specific type
	CMPINLINE static DynamicTerrainInteraction Normal(void) { return DynamicTerrainInteraction(DynamicTerrainInteractionType::Normal); }
	CMPINLINE static DynamicTerrainInteraction Extended(float value) { return DynamicTerrainInteraction(DynamicTerrainInteractionType::Extended, value); }

	// Return key parameters
	CMPINLINE DynamicTerrainInteractionType			GetType(void) const { return m_type; }
	CMPINLINE float									GetValue(void) const { return m_value; }
	CMPINLINE bool									IsPlayerInteraction(void) const { return m_player_interaction; }


	// Constructors.  Should not be used directly; interaction objects should be instantiated via static factory methods
	CMPINLINE DynamicTerrainInteraction(DynamicTerrainInteractionType type) : m_type(type), m_value(0.0f), m_player_interaction(false) { }
	CMPINLINE DynamicTerrainInteraction(DynamicTerrainInteractionType type, float value) : m_type(type), m_value(value), m_player_interaction(false) { }
	CMPINLINE DynamicTerrainInteraction(DynamicTerrainInteractionType type, float value, bool is_player_interaction)
		: m_type(type), m_value(value), m_player_interaction(is_player_interaction) { }

protected:

	// The type of interaction being performed
	DynamicTerrainInteractionType					m_type;

	// In the case of a variable interaction, e.g. a continuous switch with range of values, we store
	// a payload of the current 'interaction value'.  This may differ between types of interaction
	float											m_value;

	// Flag which indicates whether this is a player interaction or not
	bool											m_player_interaction;


};