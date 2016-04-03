#include "GameVarsExtern.h"
#include "GameSpatialPartitioningTrees.h"
#include "CapitalShipPerimeterBeacon.h"
#include "SpaceSystem.h"
#include "iActiveObject.h"
#include "Player.h"

#include "iSpaceObject.h"


// Constructor; assigns a unique ID to this object
iSpaceObject::iSpaceObject(void)
{
	// Initialise key fields to their default values
	m_spaceenvironment = NULL;
}

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iSpaceObject::InitialiseCopiedObject(iSpaceObject *source)
{
	// Pass control to all base classes
	iActiveObject::InitialiseCopiedObject(source);
}

// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
void iSpaceObject::SimulationStateChanged(iObject::ObjectSimulationState prevstate, iObject::ObjectSimulationState newstate)
{
	// If we were not being simulated, and we now are, then we may need to change how the object is operating
	// TODO: this will not always be true in future when we have more granular simulation states 
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Take action
	}

	// Conversely, if we are no longer going to be simulated, we may be able to stop simulating certain things
	if (newstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// Take action
	}
}

// Moves the space object into a new environment
void iSpaceObject::MoveIntoSpaceEnvironment(SpaceSystem *system, const FXMVECTOR location)
{
	// First, remove the object from its current environment (if applicable)
	if (m_spaceenvironment)
	{
		this->RemoveFromEnvironment();
	}

	// Now add to the new environment, assuming a valid environment has been specified
	if (system)
	{
		system->AddObjectToSystem(this, location);
	}

}

// Removes the object from its current space environment
void iSpaceObject::RemoveFromEnvironment(void)
{
	// If we are located within a system, call the removal method which will also reset all pointers etc. to NULL
	if (m_spaceenvironment)
	{
		m_spaceenvironment->RemoveObjectFromSystem(this);
	}
}

// Shutdown method to remove the space object from simulation
void iSpaceObject::Shutdown(void)
{
	// Remove from any environment and spatial partitioning tree we currently exist in
	RemoveFromEnvironment();

	// Pass control back to the base class
	iObject::Shutdown();
}


// Default destructor
iSpaceObject::~iSpaceObject(void)
{
	
}

