#pragma once

#ifndef __ShieldH__
#define __ShieldH__

#include "DX11_Core.h"
#include "Equip.h"
#include "Equipment.h"
#include "CompilerSettings.h"

class Shield : public Equipment
{
public:
	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Shield; }

	Shield(void);
	~Shield(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual Shield*				Clone() const { return new Shield(*this); }
};


#endif