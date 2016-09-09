#pragma once

#ifndef __iEnvironmentObjectH__
#define __iEnvironmentObjectH__

#include "FastMath.h"
#include "iActiveObject.h"
#include "ObjectReference.h"
class iSpaceObjectEnvironment;
class EnvironmentTree;

// Extends the iSpaceObject interface for objects that exist within and relative to some environment
// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iEnvironmentObject : public ALIGN16<iEnvironmentObject>, public iActiveObject
{
public:
	
	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iEnvironmentObject)

	// Default constructor; initialise fields to default values
	iEnvironmentObject(void) :	m_parent(nullptr), m_envposition(NULL_VECTOR), m_envorientation(NULL_VECTOR), m_within_env(false),
								m_element_location(NULL_INTVECTOR3), m_orientchanges(0), m_onground(false), m_env_treenode(NULL)
	{ 
	}

	// Returns the environment that this object is currently located in
	CMPINLINE iSpaceObjectEnvironment *			GetParentEnvironment(void) const			{ return m_parent(); }

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
	void										ChangeEnvironmentOrientation(const FXMVECTOR orient_delta);
	void										SetEnvironmentPositionAndOrientation(const FXMVECTOR pos, const FXMVECTOR orient);

	// Methods to add delta position and orientation to the current relative position
	CMPINLINE void								AddDeltaPosition(const FXMVECTOR dp)	
	{ 
		SetEnvironmentPosition(XMVectorAdd(m_envposition, dp)); 
	}

	// Returns the (precalculated) element that this object is located in.  Large objects may span multiple elements; this is 
	// the element containing the object's centre point
	CMPINLINE INTVECTOR3						GetElementLocation(void) const					{ return m_element_location; }

	// Returns a (precalculated) flag indicating whether this object is within the bounds of its parent environment
	CMPINLINE bool								IsWithinEnvironment(void) const					{ return m_within_env; }

	// Pointer to the environment tree node this object resides in
	CMPINLINE EnvironmentTree *					GetEnvironmentTreeNode(void)					{ return m_env_treenode; }
	CMPINLINE void								SetEnvironmentTreeNode(EnvironmentTree *node)	{ m_env_treenode = node; }

	// Methods to recalculate position & orientation data following a change to the object's environment position & orientation
	void										RecalculateEnvironmentPositionData(void);
	void										RecalculateEnvironmentOrientationData(void);
	void										RecalculateEnvironmentPositionAndOrientationData(void);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate.  Nothing to do at this point.
	void										SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate) { }

	// Performs all physics simulation for this environment object
	void										SimulateObjectPhysics(void);

	// Return a value indicating whether this object is on the 'ground'
	CMPINLINE bool								IsOnGround(void) const					{ return m_onground; }
	CMPINLINE void								SetGroundFlag(bool b)					{ m_onground = b; }

	// Event raised whenever the object has a significant collision with the terrain.  "Significant" denotes impacts greater than 
	// a defined threshold, so excluding e.g. normal floor collisions
	void										CollisionWithTerrain(const GamePhysicsEngine::TerrainImpactData & impact);

	// Shut down the environment object, notifying any parent environment of the change
	void										Shutdown(void);

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void										ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// Parent environment that the object is located in
	ObjectReference<iSpaceObjectEnvironment>	m_parent;

	// Stores the object position/orientation relative to the current environment, as opposed to the absolute pos/orient stored in m_position & m_orientation
	AXMVECTOR									m_envposition;
	AXMVECTOR									m_envorientation;
	
	// Flag indicating whether the object is within its environment's bounds
	bool										m_within_env;

	// Pointer to the environment tree node holding this object
	EnvironmentTree *							m_env_treenode;

	// Keep track of the element(s) this object is located in.  Large objects may span multiple elements; this is 
	// the element containing the object's centre point
	INTVECTOR3									m_element_location;

	// Keep track of the number of orientation changes made before a re-normalisation is required
	int											m_orientchanges;

	// Flag indicating whether the object is on the 'ground'
	bool										m_onground;
};


#endif





