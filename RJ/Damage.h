#pragma once

#ifndef __DamageH__
#define __DamageH__

#include <vector>
#include "Modifier.h"

// Enumeration of the possible damage types
enum DamageType
{
	UnknownDamageType = 0,
	Any,
	Kinetic,
	Collision,
	Laser,
	Missile,
	Torpedo
};

// Class holding information on a specific amount of damage of a particular type
// This class has no special alignment requirements
class Damage
{
public:
	DamageType		Type;
	float			Amount;

	Damage(void)							{ Type = DamageType::UnknownDamageType; Amount = 1.0f; }
	Damage(DamageType type, float amount)	{ Type = type; Amount = amount; }
};

// Class holding information on resistance to a specific amount of damage of a particular type
// This class has no special alignment requirements
class DamageResistance
{
public:
	DamageType		Type;
	//Modifier		Value;

	DamageResistance(void)													{ Type = DamageType::UnknownDamageType; }
	//DamageResistance(DamageType type, ModifierType modifier, float value)	{ Type = type; Value = Modifier(modifier, value); }
};


// Definition of the 'damage' collection, i.e. an amount of (potentially) multiple different types of damage
typedef std::vector<Damage>					DamageSet;

// Definition of the 'damage resistance' collection, i.e. a resistance level to (potentially) multiple different types of damage
typedef std::vector<Damage>					DamageResistanceSet;




#endif
