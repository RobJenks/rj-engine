#pragma once

#ifndef __iContainsTurrets__
#define __iContainsTurrets__

#include "TurretController.h"

class iContainsTurrets
{
public:

	// This object contains a turret controller to manage simulation and AI of all its turrets
	TurretController					TurretController;

	// Default constructor
	iContainsTurrets(void);

	// Default destructor
	~iContainsTurrets(void);

};



#endif


