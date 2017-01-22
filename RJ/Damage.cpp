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

// Static method to translate damage types to their string representation
std::string Damage::TranslateDamageTypeToString(DamageType type)
{
	switch (type)
	{
		case DamageType::EMP:			return "EMP"; 
		case DamageType::Explosive:		return "Explosive";
		case DamageType::Healing:		return "Healing";
		case DamageType::Kinetic:		return "Kinetic";
		case DamageType::Laser:			return "Laser";
		case DamageType::Plasma:		return "Plasma";
		default:						return "Any";
	}
}

// Static method to translate damage types from their string representation
DamageType Damage::TranslateDamageTypeFromString(std::string type)
{
	StrLowerC(type);
	if (type == "emp")					return DamageType::EMP;
	else if (type == "explosive")		return DamageType::Explosive;
	else if (type == "healing")			return DamageType::Healing;
	else if (type == "kinetic")			return DamageType::Kinetic;
	else if (type == "laser")			return DamageType::Laser;
	else if (type == "plasma")			return DamageType::Plasma;
	else								return DamageType::ANY;
}






