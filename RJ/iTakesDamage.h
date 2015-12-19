#pragma once

#ifndef __iTakesDamageH__
#define __iTakesDamageH__

#include "Damage.h"

class iTakesDamage
{

public:

	

	// Recalculate overall damage resistance values to each each type of damage
	void					RecalculateDamageResistance(void);

	// Retrieve overall damage resistance 


protected:

	// Entity may have resistances to one or more types of damage
	DamageResistanceSet		m_damageresistance;

};


#endif