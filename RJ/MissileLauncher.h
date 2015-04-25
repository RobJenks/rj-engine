#pragma once

#ifndef __MissileLauncherH__
#define __MissileLauncherH__

#include "DX11_Core.h"
#include "Equip.h"
#include "Equipment.h"
#include "CompilerSettings.h"

class MissileLauncher : public Equipment
{
public:
	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::MissileLauncher; }

	MissileLauncher(void);
	~MissileLauncher(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual MissileLauncher*				Clone() const { return new MissileLauncher(*this); }
};


#endif