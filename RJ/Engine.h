#pragma once

#ifndef __EngineH__
#define __EngineH__

#include <string>
#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "Equip.h"
#include "Equipment.h"
using namespace std;

class Engine : public Equipment
{
public:
	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Engine; }

	float					BaseMaxThrust;			// The standard max thrust, before any modifiers are applied for e.g. damage
	float					BaseMinThrust;			// The standard min thrust, before any modifiers are applied for e.g. damage

	float					MaxThrust;				// The maximum thrust this engine can attain (m/s)
	float					MinThrust;				// The minimum thrust (incl reverse thrust) this engine can attain (m/s)

	float					Acceleration;			// The acceleration this engine can attain (m/s/s)

	string					EmitterClass;			// String name of the engine thrust emitter associated with this engine

	Engine(void);
	~Engine(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual Engine*				Clone() const { return new Engine(*this); }
};

#endif
