#pragma once

#ifndef __DamageH__
#define __DamageH__

#include <vector>
#include "CompilerSettings.h"
#include "GameVarsExtern.h"

// Forward declarations
class DamageResistanceSet;

// Enumeration of the possible damage types
enum DamageType
{
	Kinetic = 0,			// Covers kinetic weaponry as well as collision damage
	Laser,					// Standard laser damage
	Explosive,				// Damage from explosives (missiles etc. also have a kinetic component)
	EMP,					// Damage from EMP weapons (shields and electronics are overly-susceptible, i.e. -ve resistance, to EMP)
	Plasma,					// Damage from plasma weapons
	Healing,				// Special type for repair/healing.  Should be applied as -ve damage of this type, and nothing should have resistance to the type
	ANY						// Damage of any type.  Only applicable for damage resistance, if we have blanket resistance against any type of damage
};

// Class holding information on a specific amount of damage of a particular type
// This class has no special alignment requirements
class Damage
{
public:
	DamageType			Type;
	Game::HitPoints		Amount;

	Damage(void)							: Type(DamageType::Kinetic), Amount(1.0f) { }
	Damage(DamageType type, float amount)	: Type(type), Amount(amount) { }

	// Validation method that determines whether this damage is valid
	CMPINLINE bool		IsValid(void) const		{ return ((int)Type >= 0 && (int)Type < (int)DamageType::ANY); }

	// Applies a set of resistances and reduces damage amounts accordingly
	void				ApplyDamageResistance(const DamageResistanceSet & dr);

	// Static methods to translate damage types to/from their string representation
	static std::string	TranslateDamageTypeToString(DamageType type);
	static DamageType	TranslateDamageTypeFromString(std::string type);
};

// Class holding information on resistance to a specific amount of damage of a particular type
// This class has no special alignment requirements
class DamageResistance
{
public:
	DamageType			Type;				// Resistance to a particular kind of damage (or ANY)
	Game::HitPoints		Threshold;			// Threshold below which we ignore this type of damage entirely
	float				Modifier;			// Multiplicative modifier to all damage of this type received

	// Constructors
	DamageResistance(void) : Type(DamageType::Kinetic), Threshold(0.0f), Modifier(1.0f) { }
	DamageResistance(DamageType type, float modifier) : Type(type), Threshold(0.0f), Modifier(modifier) { }
	DamageResistance(DamageType type, float modifier, Game::HitPoints threshold) : Type(type), Threshold(threshold), Modifier(modifier) { }

	// Validation method that determines whether this resistance is valid
	CMPINLINE bool		IsValid(void) const		{ return ((int)Type >= 0 && (int)Type < (int)DamageType::ANY); }
};


// Definition of the 'damage' collection, i.e. an amount of (potentially) multiple different types of damage
class DamageSet : public std::vector<Damage>
{
public:
	// Base default constructor
	DamageSet(void) : std::vector<Damage>() { }

	// Implement base constructor to initialise with data
	DamageSet(DamageSet::size_type _Count, const DamageSet::value_type & _Val)
		: std::vector<Damage>(_Count, _Val) { }

	// Applies a set of resistances and reduces damage amounts accordingly
	void ApplyDamageResistance(const DamageResistanceSet & dr);

};

// Definition of the 'damage resistance' collection, i.e. a resistance level to (potentially) multiple different types of damage
class DamageResistanceSet : public std::vector<DamageResistance>
{
public:
	
	// Default constructor
	DamageResistanceSet(void)			: m_has_damage_resistance(false) { }

	// Add a new damage resistance property; if a resistance of the same type already exists it will 
	// be added to the same entry.  This means that there is always at most one entry per damage type
	void								AddDamageResistance(const DamageResistance & dr);

	// Clear all damage resistances for this entity
	CMPINLINE void						RemoveAllDamageResistance(void)
	{
		clear();
		m_has_damage_resistance = false;
	}

	// Returns a flag indicating whether this object has any damage resistance
	CMPINLINE bool						HasDamageResistance(void) const				{ return m_has_damage_resistance; }


protected:

	// Flag indicating whether we have any damage resistance entries, for efficiency when testing damage.  Most objects will not have DR
	bool								m_has_damage_resistance;
};




#endif












