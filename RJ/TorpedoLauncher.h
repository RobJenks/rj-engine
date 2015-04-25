#pragma once

#ifndef __TorpedoLauncherH__
#define __TorpedoLauncherH__

#include "DX11_Core.h"
#include "Equip.h"
#include "Equipment.h"
#include "CompilerSettings.h"

class TorpedoLauncher : public Equipment
{
public:
	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::TorpedoLauncher; }

	TorpedoLauncher(void);
	~TorpedoLauncher(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual TorpedoLauncher*			Clone() const { return new TorpedoLauncher(*this); }
};

#endif
