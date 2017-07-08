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

// Applies a set of resistances and reduces damage amounts accordingly
void Damage::ApplyDamageResistance(const DamageResistanceSet & dr)
{
	// Look for resistance to this particular type of damage
	DamageResistanceSet::const_iterator it_end = dr.end();
	for (DamageResistanceSet::const_iterator it = dr.begin(); it != it_end; ++it)
	{
		if ((*it).Type == Type)
		{
			// We have resistance to this type.  First, if it is below our damage threshold then we can simply ignore the damage
			const DamageResistance & item = (*it);
			if (Amount <= item.Threshold) { Amount = 0.0f; return; }

			// If not, we can at least scale the damage based on our resistance
			Amount *= item.Modifier;

			// There is only one DR entry per damage type, so we can stop searching here
			break;
		}
	}
}

// Applies a set of resistances and reduces damage amounts accordingly
void DamageSet::ApplyDamageResistance(const DamageResistanceSet & dr)
{
	if (!dr.HasDamageResistance()) return;

	DamageSet::iterator it_end = end();
	for (DamageSet::iterator it = begin(); it != it_end; ++it)
	{
		(*it).ApplyDamageResistance(dr);
	}
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






