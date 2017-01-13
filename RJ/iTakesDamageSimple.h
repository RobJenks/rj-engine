#pragma once

#ifndef __iTakesDamageSimpleH__
#define __iTakesDamageSimpleH__

#include "GameVarsExtern.h"
#include "Damage.h"

class iTakesDamageSimple
{
public:

	// Constructor
	CMPINLINE iTakesDamageSimple(void) : m_maxhealth(1.0f), m_health(1.0f), m_invulnerable(false), m_destroyed(false) { }

	// Destructor
	~iTakesDamageSimple(void);

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
	CMPINLINE bool						IsInvulnerable(void) const						{ return m_invulnerable; }
	CMPINLINE void						SetInvulnerabilityFlag(bool is_invulnerable)	{ m_invulnerable = is_invulnerable; }

	// Check or set whether this object has been destroyed
	CMPINLINE bool						IsDestroyed(void) const							{ return m_destroyed; }
	CMPINLINE void						SetDestroyedFlag(bool is_destroyed)				{ m_destroyed = is_destroyed; }

	// Methods to apply damage to the object.  The simple implementation does not account for specific damage 
	// resistances, so all methods effectively reduce object health by the damage amount.  Returns true if
	// the object was destroyed by the damage inflicted
	bool								ApplyDamage(float damage);
	bool								ApplyDamage(Damage damage);
	bool								ApplyDamage(const DamageSet & damage);

	// Destroys the object.  Object is not deallocated, rather the destruction flag is set which indicates the object
	// no longer exists
	void								DestroyObject(void);

protected:

	// Hit points
	Game::HitPoints						m_health;
	Game::HitPoints						m_maxhealth;

	// Invulnerability flag for objects which do not take damage
	bool								m_invulnerable;

	// Flag indicating whether the object has been destroyed
	bool								m_destroyed;
};






#endif