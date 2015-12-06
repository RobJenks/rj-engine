#pragma once

#ifndef __iEnvironmentObjectH__
#define __iEnvironmentObjectH__

#include "FastMath.h"
#include "iActiveObject.h"
class iSpaceObjectEnvironment;

// Extends the iSpaceObject interface for objects that exist within and relative to some environment
// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iEnvironmentObject : public ALIGN16<iEnvironmentObject>, public iActiveObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iEnvironmentObject)

	// Default constructor; initialise fields to default values
	iEnvironmentObject(void) : m_parent(NULL), m_envposition(NULL_VECTOR), m_envorientation(NULL_VECTOR), m_orientchanges(0),
								m_parent_element_min(INTVECTOR3(1, 1, 1)), m_parent_element_max(INTVECTOR3(-1, -1, -1)),
								m_multielement(false), m_onground(false)
	{ 
		// Deliberately initialise element_max to -1s and element_min to +1s so that the first method to 'remove' the object
		// from its current element will skip the loop through existing elements (loop condition will fail on the first check)

	}

	// Returns the environment that this object is currently located in
	CMPINLINE iSpaceObjectEnvironment *			GetParentEnvironment(void) const			{ return m_parent; }

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(iEnvironmentObject *source);

	// Moves the object into a new parent environment, registering/deregistering with objects as necessary
	void										MoveIntoEnvironment(iSpaceObjectEnvironment *env);

	// Methods to get the relative position & orientation of this actor
	CMPINLINE XMVECTOR							GetEnvironmentPosition(void) const			{ return m_envposition; }
	CMPINLINE XMVECTOR							GetEnvironmentOrientation(void) const		{ return m_envorientation; }

	// Methods to set the object position/orientation relative to its parent environment, recalculating absolute data as required
	void										SetEnvironmentPosition(const FXMVECTOR pos);
	void										SetEnvironmentOrientation(const FXMVECTOR orient);
	void										SetEnvironmentPositionAndOrientation(const FXMVECTOR pos, const FXMVECTOR orient);

	// Methods to add delta position and orientation to the current relative position
	CMPINLINE void								AddDeltaPosition(const FXMVECTOR dp)	
	{ 
		SetEnvironmentPosition(XMVectorAdd(m_envposition, dp)); 
	}

	// Methods to recalculate position & orientation data following a change to the object's environment position & orientation
	void										RecalculateEnvironmentPositionData(void);
	void										RecalculateEnvironmentOrientationData(void);
	void										RecalculateEnvironmentPositionAndOrientationData(void);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate.  Nothing to do at this point.
	void										SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) { }

	// Returns a reference to the range of elements that this object currently exists in
	CMPINLINE const INTVECTOR3					GetElementRangeMin(void) const			{ return m_parent_element_min; }
	CMPINLINE const INTVECTOR3					GetElementRangeMax(void) const			{ return m_parent_element_max; }

	// Returns a value indicating whether this object spans multiple elements
	CMPINLINE bool								SpansMultipleElements(void) const		{ return m_multielement; }

	// Performs all physics simulation for this environment object
	void										SimulateObjectPhysics(void);

	// Return a value indicating whether this object is on the 'ground'
	CMPINLINE bool								IsOnGround(void) const					{ return m_onground; }
	CMPINLINE void								SetGroundFlag(bool b)					{ m_onground = b; }

	// Adopts the simulation state of our parent elements.  Takes the "most" simulated state if this differs across the element range
	void										UpdateSimulationStateFromParentElements(void);

	// Event raised whenever the object has a significant collision with the terrain.  "Significant" denotes impacts greater than 
	// a defined threshold, so excluding e.g. normal floor collisions
	void										CollisionWithTerrain(const GamePhysicsEngine::TerrainImpactData & impact);

	// Shut down the environment object, notifying any parent environment of the change
	void										Shutdown(void);

protected:

	// Parent environment that the object is located in, and the element bounds within which it exists
	iSpaceObjectEnvironment *					m_parent;
	INTVECTOR3									m_parent_element_min, m_parent_element_max;
	bool										m_multielement;

	// Stores the object position/orientation relative to the current environment, as opposed to the absolute pos/orient stored in m_position & m_orientation
	AXMVECTOR									m_envposition;
	AXMVECTOR									m_envorientation;
	
	// Keep track of the number of orientation changes made before a re-normalisation is required
	int											m_orientchanges;

	// Flag indicating whether the object is on the 'ground'
	bool										m_onground;
};


#endif





