#pragma once

#ifndef __EquipmentH__
#define __EquipmentH__

#include "DX11_Core.h"
#include "GameVarsExtern.h"
#include "Equip.h"
#include "CompilerSettings.h"


// This class has no special alignment requirements
class Equipment
{
private:
	Game::HitPoints				m_maxhealth;				// The maximum (& default starting) hitpoints for this equipment
	Game::HitPoints				m_health;					// The current hitpoint remaining for this item of equipment
	bool						m_destroyed;				// Determines whether the item has been 'destroyed' (may not be permanent)

public:
	// Returns the type of this equipment.  Virtual derived method so that subclasses can return their type
	// via overriding function in the Equipment vftable
	virtual CMPINLINE Equip::Class	GetType() const = 0;

	string					Name;					// The string name of this equipment
	string					Code;					// The internal string code for this equipment
	
	Game::HitPoints			GetMaxHealth(void);				// Get max HP level
	void					SetMaxHealth(Game::HitPoints h);		// Set max HP

	Game::HitPoints			GetHealth(void);				// Get current HP level
	void					SetHealth(Game::HitPoints h);			// Set current HP

	Equipment(void);
	~Equipment(void);
	Equipment(const Equipment &E);

	virtual Equipment * Clone() const = 0;

	static void InitialiseLoadedEquipmentData(void);
};

CMPINLINE Game::HitPoints Equipment::GetMaxHealth(void) { return this->m_maxhealth; }
CMPINLINE Game::HitPoints Equipment::GetHealth(void) { return this->m_health; }


#endif