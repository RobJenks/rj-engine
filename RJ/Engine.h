#pragma once

#ifndef __EngineH__
#define __EngineH__

#include <string>
#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "ModifiedValue.h"
#include "Modifier.h"
#include "Equip.h"
#include "Equipment.h"

// This class has no special alignment requirements
class Engine : public Equipment
{
public:

	// Returns the type of this equipment subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Engine; }

	// Default constructor
	Engine(void);


	CMPINLINE float			GetBaseMinThrust(void) const { return m_min_thrust.BaseValue; }
	CMPINLINE float			GetBaseMaxThrust(void) const { return m_max_thrust.BaseValue; }
	
	CMPINLINE void			SetBaseMinThrust(float min_thrust) { m_min_thrust.SetBaseValue(min_thrust); }
	CMPINLINE void			SetBaseMaxThrust(float max_thrust) { m_max_thrust.SetBaseValue(max_thrust); }
	
	CMPINLINE float			GetMinThrust(void) const { return m_min_thrust.Value; }
	CMPINLINE float			GetMaxThrust(void) const { return m_max_thrust.Value; }

	CMPINLINE float			GetBaseAcceleration(void) const { return m_acceleration.BaseValue; }
	CMPINLINE float			GetAcceleration(void) const { return m_acceleration.Value; }
	CMPINLINE void			SetBaseAcceleration(float acceleration) { return m_acceleration.SetBaseValue(acceleration); }

	// Set modifiers via movable rvalue reference (preferable for efficiency)
	CMPINLINE void			SetMinThrustModifiers(std::vector<Modifier<float>> && modifiers) { m_min_thrust.SetModifiers(std::move(modifiers)); }
	CMPINLINE void			SetMaxThrustModifiers(std::vector<Modifier<float>> && modifiers) { m_max_thrust.SetModifiers(std::move(modifiers)); }
	CMPINLINE void			SetAccelerationModifiers(std::vector<Modifier<float>> && modifiers) { m_acceleration.SetModifiers(std::move(modifiers)); }

	// Set modifiers via const lvalue reference
	CMPINLINE void			SetMinThrustModifiers(const std::vector<Modifier<float>> & modifiers) { m_min_thrust.SetModifiers(modifiers); }
	CMPINLINE void			SetMaxThrustModifiers(const std::vector<Modifier<float>> & modifiers) { m_max_thrust.SetModifiers(modifiers); }
	CMPINLINE void			SetAccelerationModifiers(const std::vector<Modifier<float>> & modifiers) { m_acceleration.SetModifiers(modifiers); }


	// String name of the engine thrust emitter associated with this engine
	std::string				EmitterClass;			

	// Default destructor
	~Engine(void);

	// Virtual override method to clone the subclass when called through the base class
	virtual Engine*				Clone() const { return new Engine(*this); }

private :

	ModifiedValue<float>		m_min_thrust;		// Minimum possible thrust level (includes -ve for reversing)
	ModifiedValue<float>		m_max_thrust;		// Maximum possible thrust level
	
	ModifiedValue<float>		m_acceleration;		// The acceleration this engine can attain (m/s/s)


};

#endif
