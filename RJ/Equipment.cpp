#include <unordered_map>
#include "Equip.h"
#include "GameDataExtern.h"
#include "GameVarsExtern.h"

#include "Equipment.h"
using namespace std;


/* Constructor */
Equipment::Equipment(void)
{
	// Default values
	this->Code = ""; this->Name = "";

	// By default this item exists
	this->m_destroyed = false;

	// By default the item has no hitpoints/max HP until we set it.  The first time the max HP *is* set
	// (via SetMaxHealth) we can therefore default the current HP to also equal that value
	this->m_maxhealth = 0;
	this->m_health = 0;
}

/* Destructor */
Equipment::~Equipment(void)
{
}

/* Copy constructor */
Equipment::Equipment(const Equipment &E)
{
	this->Code = E.Code;
	this->m_destroyed = E.m_destroyed;
	this->m_health = E.m_health;
	this->m_maxhealth = E.m_maxhealth;
	this->Name = E.Name;
}

void Equipment::SetMaxHealth(Game::HitPoints h)
{
	/* If this is the first time we have set the max health (if it is currently zero) then also 
	 * set the current health to the same value.  i.e. default spawn position is with 100% health. */
	if (this->m_maxhealth == 0.0f) 
	{
		this->m_maxhealth = h;
		this->m_health = h;
	}

	/* If this is not the initial setup, make sure that the current health is adjusted where 
	 * necessary based on the input value h */
	else
	{
		// Set the new max health
		this->m_maxhealth = h;

		// Reduce our current health if it would now be above the new max health
		if (this->m_health > h) this->m_health = h;
	}
}

void Equipment::InitialiseLoadedEquipmentData(void)
{
	Equip::Class type;

	// Do nothing for now
	return;

	// Process each item of equipment in turn
	DataRegister<Equipment>::RegisterType::const_iterator it_end = D::Equipment.Data.end();
	for (DataRegister<Equipment>::RegisterType::const_iterator it = D::Equipment.Data.begin(); it != it_end; ++it) {
		if (it->second) {
			// Take different action depending on the equipment type
			type = it->second->GetType();
			
		}
	}
}



