#pragma once

#ifndef __iTakesDamageH__
#define __iTakesDamageH__

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "Damage.h"

class iTakesDamage
{

public:

	// Constructor
	iTakesDamage(void)					: m_maxhealth(1.0f), m_health(1.0f), m_is_invulnerable(false)		{ } 

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

	// Adjust the current health of this entity
	CMPINLINE bool						IncreaseHealth(Game::HitPoints increase)		{ return SetHealth(m_health + increase); }
	CMPINLINE bool						DecreaseHealth(Game::HitPoints decrease)		{ return SetHealth(m_health - decrease); }

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

	// Primary method called when the object takes a particular type of damage.  Calculates modified 
	// damage value (based on e.g. damage resistances) and applies to the object hitpoints.  Returns 
	// true if the object was destroyed by this damage
	bool								ApplyDamage(const Damage damage);

	// Primary method called when the object takes multiple types of damage.  Calculates modified 
	// damage value (based on e.g. damage resistances) and applies to the object hitpoints.  Damage
	// is applied in the order in which is was added to the damage set.  Returns true if the object
	// was destroyed by any of the damage in this damage set
	bool								ApplyDamage(const DamageSet & damage);

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

};


#endif