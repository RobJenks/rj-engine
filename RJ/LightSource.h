#pragma once

#ifndef __LightSourceH__
#define __LightSourceH__

#include "ALIGN16.h"
#include "iSpaceObject.h"
#include "Light.h"

class LightSource : public iSpaceObject
{
public:

	// Specify the correct aligned allocators to use for this object type
	USE_ALIGN16_ALLOCATORS(LightSource)

	// Creates a new light source with default properties
	static LightSource *								Create(void);

	// Creates a new light source based on the supplied light object
	static LightSource *								Create(const Light & light);

	// Creates a new light source based on the supplied light data
	static LightSource *								Create(const LightData & data);

	// Return a non-const reference to the underlying light object
	CMPINLINE Light &									LightObject(void)					{ return m_light; }

	// Return the light data for this light source
	CMPINLINE const Light &								GetLight(void) const				{ return m_light; }

	// Importance of the light for use when rendering needs to be prioritised
	CMPINLINE int										GetPriority(void) const				{ return m_priority; }
	CMPINLINE void										SetPriority(int priority)			{ m_priority = priority; }

	// Get or set the range of this light source
	CMPINLINE float										GetRange(void) const				{ return m_light.Data.Range; }
	void												SetRange(float range);

	// Light orientation relative to the light source orientation.A forward vector(0, 0, 1) is rotated
	// by (rel_orient * obj_orient) to give the final light direction
	CMPINLINE XMVECTOR									GetRelativeLightOrientation(void) const					{ return m_relativelightorient; }
	CMPINLINE void										SetRelativeLightOrientation(const FXMVECTOR orient)		{ m_relativelightorient = orient; }

	// Object simulation method.  Light sources do not need to take any action (TODO: dynamic lights, e.g. flickering, could use this)
	CMPINLINE void										SimulateObject(void) { }

	// Light sources do not require a post-simulation update method; simulation is performed in a pre-render step only for visible light sources
	CMPINLINE virtual void								PerformPostSimulationUpdate(void) { }

	// Calculates all derived data for the light required for rendering, for example view-space equivalent data.  We only need to calculate
	// these fields if the light is being rendered this frame
	void												RecalculateRenderingData(void);

	// Inherited virtual method.  Destruction method triggered when object HP hits zero
	CMPINLINE void										DestroyObject(void) { }

	// iObject subclasses must implement a method to handle any change in simulation state
	CMPINLINE void										SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) { }

	// Inherited from iObject, called when this object collides with another.  Not relevant since light sources cannot collide with the world
	CMPINLINE void										CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact) { }

	// Custom debug string function
	std::string											DebugString(void) const;

	// Custom debug string function for light data specifically
	std::string											DebugLightDataString(void) const;

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void												ProcessDebugCommand(GameConsoleCommand & command);



	// Calculate point light volume transform for the given light
	static XMMATRIX										CalculatePointLightTransform(const LightData & light);

	// Calculate spotlight light volume transform for the given light
	static XMMATRIX										CalculateSpotlightTransform(const LightData & light);
	

protected:

	// Constructor; should NOT be called directly since all light sources should be initialised via Create() method
	LightSource(const Light & data);

	// Determine an object code for the light based on its properties
	void												DetermineObjectCode(void);

	// Light data
	Light												m_light;

	// Importance of the light for use when rendering needs to be prioritised
	int													m_priority;

	// Light orientation relative to the light source orientation.  A forward vector (0,0,1) is rotated 
	// by (rel_orient * obj_orient) to give the final light direction
	AXMVECTOR											m_relativelightorient;
};






#endif






