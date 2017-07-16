#pragma once

#ifndef __WeaponH__
#define __WeaponH__

#include "DX11_Core.h"
#include "Equip.h"
#include "Equipment.h"
#include "CompilerSettings.h"


// This class has no special alignment requirements
class Weapon : public Equipment
{
public:
	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Turret; }

	Weapon(void);
	~Weapon(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual Weapon*				Clone() const { return new Weapon(*this); }
};


#endif