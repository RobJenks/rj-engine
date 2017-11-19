#pragma once

#include "CompilerSettings.h"
#include "DynamicTerrainInteractionType.h"


class DynamicTerrainInteraction
{
public:

	// Constructors
	CMPINLINE DynamicTerrainInteraction(void) : m_type(DynamicTerrainInteractionType::None), m_value(0.0f) { }
	CMPINLINE DynamicTerrainInteraction(DynamicTerrainInteractionType type) : m_type(type), m_value(0.0f) { }
	CMPINLINE DynamicTerrainInteraction(DynamicTerrainInteractionType type, float value) : m_type(type), m_value(value) { }

	// Convenience method to create a new interaction of the specific type
	CMPINLINE static DynamicTerrainInteraction Normal(void) { return DynamicTerrainInteraction(DynamicTerrainInteractionType::Normal); }
	CMPINLINE static DynamicTerrainInteraction Extended(float value) { return DynamicTerrainInteraction(DynamicTerrainInteractionType::Extended, value); }

	// Return key parameters
	CMPINLINE DynamicTerrainInteractionType			GetType(void) const { return m_type; }
	CMPINLINE float									GetValue(void) const { return m_value; }



private:

	// The type of interaction being performed
	DynamicTerrainInteractionType					m_type;

	// In the case of a variable interaction, e.g. a continuous switch with range of values, we store
	// a payload of the current 'interaction value'.  This may differ between types of interaction
	float											m_value;



};