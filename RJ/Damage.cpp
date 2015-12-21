#include "Damage.h"

// Add a new damage resistance property for this entity
void DamageResistanceSet::AddDamageResistance(const DamageResistance & dr)
{
	// Validate parameter
	if (!dr.IsValid()) return;

	// Check whether we already have a resistance to this damage type; if so, combine with that resistance
	DamageResistanceSet::iterator it_end = end();
	for (DamageResistanceSet::iterator it = begin(); it != it_end; ++it)
	{
		if ((*it).Type == dr.Type)
		{
			(*it).Threshold += dr.Threshold;
			(*it).Modifier *= dr.Modifier;
			return;
		}
	}

	// This is a new damage resistance, so push onto the DR collection
	push_back(dr);
	m_has_damage_resistance = true;
}