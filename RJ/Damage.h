#pragma once

#ifndef __DamageH__
#define __DamageH__

#include <vector>

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
class Damage
{
public:
	DamageType		Type;
	float			Amount;

	Damage(void)							{ Type = DamageType::UnknownDamageType; Amount = 1.0f; }
	Damage(DamageType type, float amount)	{ Type = type; Amount = amount; }
};

// Definition of the 'damage' collection, i.e. an amount of (potentially) multiple different types of damage
typedef vector<Damage>			DamageSet;




#endif
