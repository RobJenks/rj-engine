#pragma once

#ifndef __HpEngineH__
#define __HpEngineH__

#include "DX11_Core.h"
#include "Ship.h"
#include "ShipPhysicsState.h"
#include "CompilerSettings.h"
#include "Hardpoint.h"

class Engine;
class SpaceEmitter;
class CoreEngine;
class Hardpoint;

class HpEngine : public Hardpoint
{
public:
	// Returns the type of this hardpoint subclass
	virtual CMPINLINE Equip::Class	GetType() const { return Equip::Class::Engine; }

	void							RecalculateHardpointData(void);		// Recalculates hardpoint stats based on mounted equipment

	void							SetThrust(float);					// Sets the engine thrust level
	CMPINLINE float					GetThrust(void);					// Gets the current thrust level

	CMPINLINE D3DXVECTOR3*			GetThrustVector(void);				// Gets the thrust vector for this engine
	
	CMPINLINE float					GetTargetThrust(void) { return TargetThrust; }
	void							SetTargetThrust(float f_thrust);
	void							SetTargetThrustPercentage(float percentage);
	void							IncrementTargetThrust(void);				// Increments target thrust level
	void							DecrementTargetThrust(void);				// Decrements target thrust level
	
	void							RunToTargetThrust(void);				// Adjusts thrust towards the target level, based on engine parameters
	void							NotifyParentOfThrustChange();			// Notify the parent ship that we are changing the engine thrust somehow

	CMPINLINE Engine*				GetEngine(void);					// Returns a pointer to the currently-mounted engine (if there is one)
	void							MountEngine(Engine *engine);		// Mounts a new engine on this hardpoint

	void							InitialiseEngineThrustEmitter(void);	// Creates a new engine thrust emitter based on class name in engine object
	void							UpdateEngineThrustEmitter(void);					// Updates the engine thrust emitter based on current engine state
	void							DestroyEngineThrustEmitter(void);					// Disposes of the engine thrust emitter
	CMPINLINE SpaceEmitter*			GetEngineThrustEmitter(void) { return m_emitter; }	// Returns a reference to the thrust emitter

	// Virtual override method to clone the subclass when called through the base class
	virtual HpEngine*				Clone() const
	{
		HpEngine *hp = new HpEngine(*this);			// Clone the hardpoint and all fields
		hp->MountEquipment(NULL);					// Remove any reference to mounted equipment; we only clone the HP, not the equipment on it
		return hp;
	}

	// Virtual override method to delete the subclass when called through the base class
	virtual void					Delete() const { delete this; }

	// Virtual override method to mount equipment on this hardpoint, if called from the base class
	virtual void					MountEquipment(Equipment *e);

	HpEngine(void);
	~HpEngine(void);
	HpEngine(const HpEngine &H);
	HpEngine& operator=(const HpEngine&H);

//private:
	D3DXVECTOR3			BaseThrustVector;		// Directional thrust vector relative to ship body
	D3DXVECTOR3			CachedThrustVector;		// A cached thrust vector to avoid recalculation

	float				Thrust;					// Current thrust produced by this engine
	float				TargetThrust;			// The thrust level we are aiming to reach

	SpaceEmitter		*m_emitter;				// The particle emitter for rendering this engine's thrust

	// Properties relating to the engine thrust emitter, since we will be adjusting the emitter itself based on engine state
	float				m_emit_freqlow, m_emit_freqhigh;			// Emission frequency high/low, at 100% thrust
	float				m_emit_freqlowrange, m_emit_freqhighrange;	// Emission frequency maximum, at lowest non-zero thrust
	D3DXVECTOR3			m_emit_vellow, m_emit_velhigh;				// Velocity high/low, at 100% thrust

};


CMPINLINE float HpEngine::GetThrust() { return Thrust; }
CMPINLINE D3DXVECTOR3 *HpEngine::GetThrustVector() { return &CachedThrustVector; }

CMPINLINE Engine *HpEngine::GetEngine() { return (Engine*)m_equipment; }


#endif