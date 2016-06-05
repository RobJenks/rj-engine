#pragma once

#ifndef __LightSourceH__
#define __LightSourceH__

#include "ALIGN16.h"
#include "iObject.h"
#include "Light.h"

class LightSource : public ALIGN16<LightSource>, public iObject
{
public:

	// Specify the correct aligned allocators to use for this object type
	USE_ALIGN16_ALLOCATORS(LightSource)

	// Default constructor
	LightSource(void);

	// Return or set the light data for this light source
	CMPINLINE const Light &								GetLight(void) const				{ return m_light; }
	void												SetLight(const Light & data);

	// Importance of the light for use when rendering needs to be prioritised
	CMPINLINE int										GetPriority(void) const				{ return m_priority; }
	CMPINLINE void										SetPriority(int priority)			{ m_priority = priority; }

	// Get or set the range of this light source
	CMPINLINE float										GetRange(void) const				{ return m_light.Data.Range; }
	void												SetRange(float range);

	// Object simulation method.  Light sources do not need to take any action (TODO: dynamic lights, e.g. flickering, could use this)
	CMPINLINE void										SimulateObject(void) { }

	// Light sources do implement a post-simulation update method to reposition their internal light component
	virtual void										PerformPostSimulationUpdate(void);

	// Inherited virtual method.  Destruction method triggered when object HP hits zero
	CMPINLINE void										DestroyObject(void) { }

	// iObject subclasses must implement a method to handle any change in simulation state
	CMPINLINE void										SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) { }

	// Inherited from iObject, called when this object collides with another.  Not relevant since light sources cannot collide with the world
	CMPINLINE void										CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact) { }



protected:

	// Light data
	Light												m_light;

	// Importance of the light for use when rendering needs to be prioritised
	int													m_priority;

};






#endif






