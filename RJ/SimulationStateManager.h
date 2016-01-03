#pragma once

#ifndef __SimulationStateManagerH__
#define __SimulationStateManagerH__

#include <vector>
#include "Utility.h"
#include "ScheduledObject.h"
#include "ObjectReference.h"
#include "iObject.h"
#include "iSpaceObject.h"

// This class has no special alignment requirements
class SimulationStateManager : public ScheduledObject
{
public:
	// Default constructor
	SimulationStateManager(void);

	// Regularly-scheduled method to re-evaluate and update the simulation state of the universe
	void							Update(void);

	// Infrequent update method is not currently used by the simulation manager
	CMPINLINE void					UpdateInfrequent(void) { }

	// Method which allows us to specify the next system to be evaluated in scheduled updates.  Must be a 
	// hub system.  If we want to re-evaluate a non-hub system, use the EvaluateSystem... method directly
	void							SetNextHubSystemForEvaluation(SpaceSystem *system);

	// Method called whenever a space object enters an environment
	void							ObjectEnteringSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment);

	// Method called whenever a space object leaves an environment
	void							ObjectLeavingSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment);

	// Method called whenever an environment object enters an interior location
	void							ObjectEnteringInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment);

	// Method called whenever an environment object leaves an interior location
	void							ObjectLeavingInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment);

	// Method called when a simulation hub enters the environment, or is created.  Affects simulation state of all its surroundings.  
	// Passes control to the relevant method depending on the class of object
	void							SimulationHubEnteringEnvironment(iObject * object);

	// Method called when a space-based simulation hub enters the environment, or is created.  Affects simulation state of all its surroundings.  
	void							SimulationHubEnteringSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment);

	// Method called when an environment object enters an environment, or is created.  Affects simulation state of all its surroundings
	void							SimulationHubEnteringInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment);

	// Method called when a simulation object leaves its current environment, or is removed.  Affects simulation state of all its surroundings.
	// Passes control to the relevant method depending on the class of object
	void							SimulationHubLeavingEnvironment(iObject * object);

	// Method called when a space-based simulation hub enters the environment, or is created.  Affects simulation state of all its surroundings.  
	void							SimulationHubLeavingSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment);

	// Method called when an environment object enters an environment, or is created.  Affects simulation state of all its surroundings
	void							SimulationHubLeavingInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment);

	// Determines the appropriate simulation state for a space object, based upon its proximity to simulation hubs and other criteria
	iObject::ObjectSimulationState	DetermineSimulationStateForSpaceObject(iSpaceObject * object);

	// Determines the appropriate simulation state for an interior object, based upon its proximity to simulation hubs and other criteria
	iObject::ObjectSimulationState	DetermineSimulationStateForInteriorObject(iEnvironmentObject * object);

	// Evaluates the simulation state of all objects within a system.  Also evaluates objects within interior environments in that system
	void							EvaluateSimulationStateInSystem(SpaceSystem * system);

	// Evaluates the simulation state of all objects within an interior environment, in isolation. No objects in the surrounding space
	// environment will be considered when determining simulation state.  
	void							EvaluateSimulationStateInIsolatedInteriorEnvironment(iSpaceObjectEnvironment * environment);

	// Populates the output vector with all space-based simulation hubs in the specified system.  Appends to any existing contents.
	void							GetAllSpaceSimulationHubsInSystem(SpaceSystem *system, std::vector<iSpaceObject*> & outResult);

	// Populates the output vector with all interior simulation hubs in the specified system.  Appends to any existing contents.
	void							GetAllInteriorSimulationHubsInSystem(SpaceSystem *system, std::vector<iEnvironmentObject*> & outResult);

	// Updates the simulation state of every relevant object in the specified environment to a specific level of simulation
	void							SetSimulationStateOfAllEnvironmentObjects(iSpaceObjectEnvironment *environment, iObject::ObjectSimulationState state);

	// Updates the simulation state of every relevant object in the specified environment to a specific level of simulation.  The object 
	// state will only be set if this would be an upgrade (ComparisonResult::GreaterThan) on its current simulation state
	void							UpgradeSimulationStateOfAllEnvironmentObjects(iSpaceObjectEnvironment *environment, iObject::ObjectSimulationState state);

	// Tests whether an environment contains any interior simulation hubs, and sets the environment object flag accordingly
	void							EvaluateEnvironmentAsHubContainer(iSpaceObjectEnvironment * environment);

	// Methods to add or remove items from the primary collections
	void							AddSpaceSimulationHub(iSpaceObject *hub);
	void							RemoveSpaceSimulationHub(iSpaceObject *hub);			
	void							AddInteriorSimulationHub(iEnvironmentObject *hub);
	void							RemoveInteriorSimulationHub(iEnvironmentObject*hub);
	
	// Checks the integrity of the simulation hub collections and makes corrections / removes invalid entries if required
	void							ValidateSimulationHubCollections(void);

	// Default destructor
	~SimulationStateManager(void);

protected:

	// Convenience method to test how many simulation hubs exist in the given system
	int								DetermineSimulationHubCountInSystem(SpaceSystem *system);

	// Methods to add or remove hub systems.  Based on other internal simulation manager methods.  Do not need to be called directly
	void							AddHubSystem(SpaceSystem *system);
	void							RemoveHubSystem(SpaceSystem *system);

	// Methods to remove simulation hubs using a direct iterator reference.  Protected since these should only
	// be called by internal methods that can correctly define the iterator
	void							RemoveSpaceSimulationHub(std::vector<ObjectReference<iSpaceObject>>::iterator hub);
	void							RemoveInteriorSimulationHub(std::vector<ObjectReference<iEnvironmentObject>>::iterator hub);

protected:

	// The state manager maintains a collection of all simulation hubs, for use in determining the simulation state of other objects
	std::vector<ObjectReference<iSpaceObject>>				m_space_simhubs;
	std::vector<ObjectReference<iEnvironmentObject>>		m_env_simhubs;

	// We maintain a list of systems that contain at least one hub object, for efficiency at runtime
	std::vector<SpaceSystem*>								m_hubsystems;
	std::vector<SpaceSystem*>::size_type					m_hubsystemcount;

	// Maintain an index of the hub system to be evaluated in the next scheduled update
	std::vector<SpaceSystem*>::size_type					m_nexthubsystem;
};




#endif