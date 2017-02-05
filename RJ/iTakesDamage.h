#pragma once

#ifndef __iTakesDamageH__
#define __iTakesDamageH__

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "FastMath.h"
#include "Damage.h"
#include "GameConsoleCommand.h"
#include "GamePhysicsEngine.h"

class iTakesDamage
{

public:

	// Constructor
	iTakesDamage(void)					: m_maxhealth(1.0f), m_health(1.0f), m_is_invulnerable(false)		{ } 

	// Destructor
	~iTakesDamage(void);

	// Current HP of the entity
	CMPINLINE Game::HitPoints			GetHealth(void) const							{ return m_health; }

	// Sets the current entity health.  Returns a flag indicating whether this change resulted in destruction of the entity (i.e. if HP <= 0)
	CMPINLINE bool						SetHealth(Game::HitPoints new_health)			
	{
		if (new_health > 0.0f)
		{
			m_health = min(new_health, m_maxhealth);
			return false;
		}
		else
		{
			m_health = 0.0f;
			DestroyObject();
			return true;
		}
	}

	// Adjust the current health of this entity.  Returns a flag indicating whether the change in health resulting
	// in destruction of the object
	CMPINLINE bool						IncreaseHealth(Game::HitPoints increase)		{ return SetHealth(m_health + increase); }
	CMPINLINE bool						DecreaseHealth(Game::HitPoints decrease)		{ return SetHealth(m_health - decrease); }
	CMPINLINE bool						SetHealthPercentage(float pc)					{ return SetHealth(m_maxhealth * pc); }

	// Maximum HP of the entity
	CMPINLINE Game::HitPoints			GetMaxHealth(void) const						{ return m_maxhealth; }
	CMPINLINE void						SetMaxHealth(Game::HitPoints max_health)		
	{ 
		m_maxhealth = max(1.0f, max_health); 
		if (m_health > m_maxhealth) SetHealth(m_maxhealth);
	}

	// Check or set whether this object is invulnerable, i.e. takes no damage of any kind
	CMPINLINE bool						IsInvulnerable(void) const						{ return m_is_invulnerable; }
	CMPINLINE void						SetInvulnerabilityFlag(bool is_invulnerable)	{ m_is_invulnerable = is_invulnerable; }

	// Returns a flag indicating whether this object has any damage resistance
	CMPINLINE bool						HasDamageResistance(void) const					{ return m_damageresistance.HasDamageResistance(); }

	// Add a new damage resistance property for this entity
	void								AddDamageResistance(const DamageResistance & dr){ m_damageresistance.AddDamageResistance(dr); }

	// Clear all damage resistances for this entity
	CMPINLINE void						RemoveAllDamageResistance(void)					{ m_damageresistance.RemoveAllDamageResistance(); }

	// Return the immutable set of damage resistance values for this object
	CMPINLINE const DamageResistanceSet &	GetDamageResistance(void) const				{ return m_damageresistance; }

	// Return a modifiable reference to the object damage resistance set
	CMPINLINE DamageResistanceSet &			DamageResistanceData(void)					{ return m_damageresistance; }

	// Primary method called when the object takes damage at a specified (object-local) position.  
	// Calculates modified damage value (based on e.g. damage resistances) and applies to the 
	// object hitpoints.  Damage is applied in the order in which is was added to the damage 
	// set.  Returns true if the object was destroyed by any of the damage in this damage set
	virtual bool						ApplyDamage(const DamageSet & damage, const GamePhysicsEngine::OBBIntersectionData & impact);
	CMPINLINE bool						ApplyDamage(const DamageSet & damage) { return ApplyDamage(damage, GamePhysicsEngine::OBBIntersectionData::NullValue); }

	// Simple method to apply a single component of damage.  Pass-through to the primary damage method
	// Returns true if the object is destroyed by this damage
	CMPINLINE bool						ApplyDamage(const Damage & damage, const GamePhysicsEngine::OBBIntersectionData & impact)
	{
		_tmp_damageset[0] = damage;
		return ApplyDamage(_tmp_damageset, impact);
	}
	CMPINLINE bool						ApplyDamage(const Damage & damage) { return ApplyDamage(damage, GamePhysicsEngine::OBBIntersectionData::NullValue); }

	// Simple method to apply an amount of damage.  Damage has no ("ANY") type and so is not affected
	// by any damage resistances except universal ("ALL") resistance.  Returns true if the object was destroyed by this damage
	CMPINLINE bool						ApplyDamage(float damage, const GamePhysicsEngine::OBBIntersectionData & impact)
	{ 
		return ApplyDamage(Damage(DamageType::ANY, damage), impact); 
	}
	CMPINLINE bool						ApplyDamage(float damage) { return ApplyDamage(damage, GamePhysicsEngine::OBBIntersectionData::NullValue); }

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void								ProcessDebugCommand(GameConsoleCommand & command);

	// Virtual method to be implemented by inheriting classes.  Destruction method triggered when object HP hits zero
	virtual void						DestroyObject(void) = 0;


protected:

	// Hit points
	Game::HitPoints						m_health;
	Game::HitPoints						m_maxhealth;

	// Entity may have resistances to one or more types of damage
	DamageResistanceSet					m_damageresistance;

	// Flag indicating whether the object is invulnerable (and therefore ignores all damage)
	bool								m_is_invulnerable;

	// Protected method called to apply a particular component of incoming damage.  Calculates modified 
	// damage value (based on e.g. damage resistances) and applies to the object hitpoints.  Returns 
	// true if the object was destroyed by this damage
	bool								ApplyDamageComponent(Damage damage);

	// Static local vector used to translate Damage > DamageSet without the need for an additional virtual call
	static DamageSet					_tmp_damageset;
};


#endif