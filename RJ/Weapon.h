#pragma once

#ifndef __WeaponH__
#define __WeaponH__

#include "DX11_Core.h"
#include "Equip.h"
#include "Equipment.h"
#include "CompilerSettings.h"
#include "SpaceTurret.h"

// This class has no special alignment requirements
class Weapon : public Equipment
{
public:
	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class		GetType() const { return Equip::Class::Turret; }

	// Default constructor
	Weapon(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual Weapon*						Clone() const { return new Weapon(*this); }

	// Turret class that this weapon represents
	CMPINLINE std::string				GetTurretCode(void) const					{ return m_turret_code; }
	CMPINLINE void						SetTurretCode(const std::string & code)		{ m_turret_code = code; }

	// Default destructor
	~Weapon(void);

	
private:

	// Class of turret that this weapon represents
	std::string							m_turret_code;

};


#endif