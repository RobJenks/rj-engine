#include "SpaceTurret.h"
#include "Weapon.h"

// Default constructor
Weapon::Weapon(void) 
	: 
	Equipment(), m_turret_code(NullString)
{	
}

// Default destructor
Weapon::~Weapon(void)
{
}
