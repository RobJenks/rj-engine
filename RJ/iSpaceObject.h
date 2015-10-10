#pragma once

#ifndef __iSpaceObjectH__
#define __iSpaceObjectH__

#include "CompilerSettings.h"
#include "FastMath.h"
#include "iActiveObject.h"
#include "Octree.h"
class SpaceSystem;

class iSpaceObject : public iActiveObject
{
public:

	// Default constructor and destructor
	iSpaceObject(void);
	~iSpaceObject(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void									InitialiseCopiedObject(iSpaceObject *source);

	// Removes the object from its current space environment
	void									RemoveFromEnvironment(void);

	// Retrieve and set the spatial partitioning tree node this object belongs to
	CMPINLINE Octree<iSpaceObject*> *		GetSpatialTreeNode(void) const						{ return m_treenode; }
	CMPINLINE void							SetSpatialTreeNode(Octree<iSpaceObject*> * node)	{ m_treenode = node; }

	// Retrieve a pointer to the space system that this object exists in
	CMPINLINE SpaceSystem *					GetSpaceEnvironment(void) const						{ return m_spaceenvironment; }
	CMPINLINE void							SetSpaceEnvironmentDirect(SpaceSystem *env)			{ m_spaceenvironment = env; }

	// Moves the object into a new space environment.  Declared virtual so that subclasses can implement class-specific logic if required, 
	// before finally calling the base iSpaceObject method
	virtual void							MoveIntoSpaceEnvironment(SpaceSystem *system, const D3DXVECTOR3 & location);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
	// Further derived classes (e.g. ships) can implement this method and then call iSpaceObject::SimulationStateChanged() to maintain the chain
	void									SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate);

	// Shutdown method to remove the space object from simulation
	void									Shutdown(void);

protected:


protected:

	Octree<iSpaceObject*> *				m_treenode;						// Stores a pointer to the spatial partitioning node we belong to

	SpaceSystem *						m_spaceenvironment;				// Stores a pointer to the system that this object exists in

};



#endif
