#pragma once

#ifndef __iSpaceObjectH__
#define __iSpaceObjectH__

#include "CompilerSettings.h"
#include "FastMath.h"
#include "iActiveObject.h"
class SpaceSystem;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iSpaceObject : public ALIGN16<iSpaceObject>, public iActiveObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iSpaceObject)

	// Default constructor and destructor
	iSpaceObject(void);
	virtual ~iSpaceObject(void) = 0;

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void									InitialiseCopiedObject(iSpaceObject *source);

	// Removes the object from its current space environment
	void									RemoveFromEnvironment(void);

	// Retrieve a pointer to the space system that this object exists in
	CMPINLINE SpaceSystem *					GetSpaceEnvironment(void) const						{ return m_spaceenvironment; }
	CMPINLINE void							SetSpaceEnvironmentDirect(SpaceSystem *env)			{ m_spaceenvironment = env; }

	// Moves the object into a new space environment.  Declared virtual so that subclasses can implement class-specific logic if required, 
	// before finally calling the base iSpaceObject method
	virtual void							MoveIntoSpaceEnvironment(SpaceSystem *system);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
	// Further derived classes (e.g. ships) can implement this method and then call iSpaceObject::SimulationStateChanged() to maintain the chain
	void									SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate);

	// Shutdown method to remove the space object from simulation
	void									Shutdown(void);

	// Custom debug string function
	std::string								DebugString(void) const;
	
	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void									ProcessDebugCommand(GameConsoleCommand & command);


protected:

	SpaceSystem *							m_spaceenvironment;				// Stores a pointer to the system that this object exists in

};



#endif
