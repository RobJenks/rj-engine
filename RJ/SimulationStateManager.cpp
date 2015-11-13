#include "Utility.h"
#include "SimulationObjectManager.h"
#include "iSpaceObject.h"
#include "iEnvironmentObject.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipTile.h"
#include "ComplexShipElement.h"
#include "SpaceSystem.h"
#include "GameUniverse.h"

#include "SimulationStateManager.h"

/* *** TODO: NEED TO ALSO HANDLE THE EVENT WHERE AN INTERIOR OBJECT ENTERS/LEAVES AN ENVIRONMENT, AND CALL THE RELEVANT METHOD BELOW.  SIMILAR
       TO THE WAY METHODS ARE CALLED WITHIN SPACESYSTEM WHEN SPACE OBJECTS ENTER OR LEAVE.  ADD TO ISPACEOBJECTENVIRONMENT MOST LIKELY *** */

// Default constructor
SimulationStateManager::SimulationStateManager(void)
{
	// Set all fields to defaults
	m_hubsystemcount = 0U;
	m_nexthubsystem = 0U;
}

// Regularly-scheduled method to re-evaluate and update the simulation state of the universe
void SimulationStateManager::Update(void)
{
	// If there are no hub systems then quit immediately
	if (m_hubsystemcount == 0) return;

	// If the index of the next system to be simulated is beyond the end of the collection, wrap it back to zero
	if (m_nexthubsystem >= m_hubsystemcount) m_nexthubsystem = 0U;

	// We know there is at least one hub system in the collection, so this index must be valid.  Evaluate now.
	SpaceSystem *system = m_hubsystems.at(m_nexthubsystem);
	if (system) EvaluateSimulationStateInSystem(system);

	// Increment the next simulation counter
	++m_nexthubsystem;
}


// Method which allows us to specify the next system to be evaluated in scheduled updates.  Must be a 
// hub system.  If we want to re-evaluate a non-hub system, use the EvaluateSystem... method directly
void SimulationStateManager::SetNextHubSystemForEvaluation(SpaceSystem *system)
{
	// Attmept to locate this system in the hub systems collection
	int index = FindInVector<SpaceSystem*>(m_hubsystems, system);
	if (index != -1)
	{
		// Force the next system index to cause this system to be simulated next
		m_nexthubsystem = (unsigned int)index;
	}
}

// Determines the appropriate simulation state for a space object, based upon its proximity to simulation hubs and other criteria
iObject::ObjectSimulationState SimulationStateManager::DetermineSimulationStateForSpaceObject(iSpaceObject * object)
{
	// Parameter check
	if (!object) return iObject::ObjectSimulationState::NoSimulation;

	// If this is a simulation hub then we immediately want to perform full simulation
	if (object->IsSimulationHub()) return iObject::ObjectSimulationState::FullSimulation;

	// If we don't have an environment then we will not be simulated
	if (object->GetSpaceEnvironment() == NULL) return iObject::ObjectSimulationState::NoSimulation;

	// If there are no simulation hubs in this system then we should immediately default to strategic simulation
	if (FindInVector<SpaceSystem*>(m_hubsystems, object->GetSpaceEnvironment()) == -1)
		return iObject::ObjectSimulationState::StrategicSimulation;

	// Otherwise, test our proximity to all registered simulation hubs
	bool insystem = false; XMVECTOR distsq;
	std::vector<iSpaceObject*>::const_iterator it_end = m_space_simhubs.end();
	for (std::vector<iSpaceObject*>::const_iterator it = m_space_simhubs.begin(); it != it_end; ++it)
	{
		// The hub is only relevant to us if it is in the same system.  We know neither is null since 
		// we already tested the object environment above
		if ((*it)->GetSpaceEnvironment() == object->GetSpaceEnvironment())
		{
			// Now test the distance to this hub object; if we are close, we want full simulation
			distsq = XMVector3LengthSq(XMVectorSubtract(object->GetPosition(), (*it)->GetPosition()));
			if (XMVector3LessOrEqual(distsq, Game::C_SPACE_SIMULATION_HUB_RADIUS_SQ_V))
				return iObject::ObjectSimulationState::FullSimulation;
		}
	}

	// Also check if there is a simulation hub in an environment that shares this system, in which 
	// case we may also go with full simulation
	std::vector<iEnvironmentObject*>::const_iterator it2_end = m_env_simhubs.end();
	for (std::vector<iEnvironmentObject*>::const_iterator it2 = m_env_simhubs.begin(); it2 != it2_end; ++it2)
	{
		// Test whether the hub has an environment, and that environment is in the same system as us
		if ((*it2)->GetParentEnvironment() != NULL && (*it2)->GetParentEnvironment()->GetSpaceEnvironment() == object->GetSpaceEnvironment())
		{
			// Test whether the hub's environment is close enough
			distsq = XMVector3LengthSq(XMVectorSubtract(object->GetPosition(), (*it2)->GetPosition()));
			if (XMVector3LessOrEqual(distsq, Game::C_SPACE_SIMULATION_HUB_RADIUS_SQ_V))
				return iObject::ObjectSimulationState::FullSimulation;
		}
	}

	// We are in the same system as a simulation hub, but not close enough to any for full simulation.  Simulate 
	// at tactical level
	return iObject::ObjectSimulationState::TacticalSimulation;
}

// Determines the appropriate simulation state for an interior object, based upon its proximity to simulation hubs and other criteria
iObject::ObjectSimulationState SimulationStateManager::DetermineSimulationStateForInteriorObject(iEnvironmentObject * object)
{
	// Parameter check
	if (!object) return iObject::ObjectSimulationState::NoSimulation;

	// If this is a simulation hub then we immediately want to perform full simulation
	if (object->IsSimulationHub()) return iObject::ObjectSimulationState::FullSimulation;

	// If we don't have an environment then we will not be simulated
	iSpaceObjectEnvironment *environment = object->GetParentEnvironment();
	if (environment == NULL) return iObject::ObjectSimulationState::NoSimulation;

	// If there are no simulation hubs in this system then we should immediately default to strategic simulation
	if (FindInVector<SpaceSystem*>(m_hubsystems, environment->GetSpaceEnvironment()) == -1)
		return iObject::ObjectSimulationState::StrategicSimulation;

	// Otherwise, test our proximity to all registered simulation hubs
	bool proximity = false; XMVECTOR distsq;
	std::vector<iEnvironmentObject*>::const_iterator it_end = m_env_simhubs.end();
	for (std::vector<iEnvironmentObject*>::const_iterator it = m_env_simhubs.begin(); it != it_end; ++it)
	{
		// If we are in the same environment as a simulation hub then we should be fully simulated
		if ((*it)->GetParentEnvironment() == environment)
			return iObject::ObjectSimulationState::FullSimulation;

		// Otherwise, test if our environment is at least close to the hub's environment
		if ((*it)->GetParentEnvironment() != NULL)
		{
			distsq = XMVector3LengthSq(XMVectorSubtract(environment->GetPosition(), (*it)->GetParentEnvironment()->GetPosition()));
			if (XMVector3LessOrEqual(distsq, Game::C_SPACE_SIMULATION_HUB_RADIUS_SQ_V))
				proximity = true;
		}
	}

	// If our environment is close to another containing a hub, we can use tactical simulation for the object
	if (proximity) return iObject::ObjectSimulationState::TacticalSimulation;

	// Last chance: if there are any space-based hubs close to our environment, we should also use tactical simulation
	std::vector<iSpaceObject*>::const_iterator it2_end = m_space_simhubs.end();
	for (std::vector<iSpaceObject*>::const_iterator it2 = m_space_simhubs.begin(); it2 != it2_end; ++it2)
	{
		// Test whether the hub is in the same system as our environment
		if ((*it2)->GetSpaceEnvironment() && (*it2)->GetSpaceEnvironment() == object->GetParentEnvironment()->GetSpaceEnvironment())
		{
			distsq = XMVector3LengthSq(XMVectorSubtract(object->GetParentEnvironment()->GetPosition(), (*it2)->GetPosition()));
			if (XMVector3LessOrEqual(distsq, Game::C_SPACE_SIMULATION_HUB_RADIUS_SQ_V))
				return iObject::ObjectSimulationState::TacticalSimulation;
		}
	}

	// We don't meet any of the criteria, so the interior object will be simulated at strategic level
	return iObject::ObjectSimulationState::StrategicSimulation;
}

// Evaluates the simulation state of all objects within a system.  Also evaluates objects within interior environments in that system
void SimulationStateManager::EvaluateSimulationStateInSystem(SpaceSystem * system)
{
	iSpaceObject *spaceobject;
	iSpaceObjectEnvironment *env;

	// Parameter check
	if (!system) return;

	// Check whether any simulation hubs exist in this system; this will determine the default simulation state
	iObject::ObjectSimulationState defaultstate = (FindInVector<SpaceSystem*>(m_hubsystems, system) == -1 ?
														iObject::ObjectSimulationState::StrategicSimulation :
														iObject::ObjectSimulationState::TacticalSimulation);

	// Default all objects to either strategic or tactical simulation as a default, depending on whether hubs are present
	std::vector<iSpaceObject*>::iterator it, it_end;
	it_end = system->Objects.end();
	for (it = system->Objects.begin(); it != it_end; ++it)
	{
		spaceobject = (*it); if (!spaceobject) continue;
		spaceobject->SetSimulationState(defaultstate);

		// If the object is itself an environment, also update the state of all its occupants to strategic
		if (spaceobject->IsEnvironment())
		{
			SetSimulationStateOfAllEnvironmentObjects(	(iSpaceObjectEnvironment*)spaceobject, 
														iObject::ObjectSimulationState::StrategicSimulation);
		}
	}

	// Advance-declare some variables that will be used multiple times below
	std::vector<iSpaceObject*> spacehubs, objects;

	// Otherwise, first get a reference to all space-based simulation hubs in the system
	GetAllSpaceSimulationHubsInSystem(system, spacehubs);
	bool havespacehubs = !spacehubs.empty();

	// If we have any space hubs, check each one now
	if (havespacehubs)
	{
		// Iterate over each simulation hub in turn
		it_end = spacehubs.end();
		for (it = spacehubs.begin(); it != it_end; ++it)
		{
			// The simulation hub itself should always be fully-simulated; if it is an environment, so should its contents
			(*it)->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
			if ((*it)->IsEnvironment()) SetSimulationStateOfAllEnvironmentObjects(	(iSpaceObjectEnvironment*)(*it), 
																					iObject::ObjectSimulationState::FullSimulation);

			// Get all objects within the simulation hub radius from this object and iterate through them
			Game::ObjectManager.GetAllObjectsWithinDistance((*it), Game::C_SPACE_SIMULATION_HUB_RADIUS, objects,
				SimulationObjectManager::ObjectSearchOptions::NoSearchOptions);
			std::vector<iSpaceObject*>::iterator it2_end = objects.end();
			for (std::vector<iSpaceObject*>::iterator it2 = objects.begin(); it2 != it2_end; ++it2)
			{
				// Make sure the object is valid
				spaceobject = (*it2); if (!spaceobject) continue;

				// We want to fully simulate this object since it is close to a space simulation hub
				spaceobject->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);

				// If this object is itself an environment, all the objects within it will have at least tactical simulation
				// The objects may also later be upgraded to full simulation if there is an interior hub also in their environment
				if (spaceobject->IsEnvironment())
				{
					env = (iSpaceObjectEnvironment*)spaceobject;
					SetSimulationStateOfAllEnvironmentObjects(env, iObject::ObjectSimulationState::TacticalSimulation);
				}
			}
		}
	}

	// Now get a reference to all interior simulation hubs in the system
	std::vector<iEnvironmentObject*> envhubs, envobjects;
	GetAllInteriorSimulationHubsInSystem(system, envhubs);
	bool haveenvhubs = !envhubs.empty();

	// If we have any interior hubs, check each one now
	if (haveenvhubs)
	{
		// Iterate through all interior hubs in turn
		std::vector<iEnvironmentObject*>::iterator it2_end = envhubs.end();
		for (std::vector<iEnvironmentObject*>::iterator it2 = envhubs.begin(); it2 != it2_end; ++it2)
		{
			// The simulation hub itself should always be fully-simulated 
			(*it2)->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);

			// Make sure this interior hub has an environment; if not (and this shouldn't happen) there is nothing more to do
			env = (*it2)->GetParentEnvironment();
			if (!env) continue;

			// All objects sharing the environment with this interior hub, and the environment itself, should also be fully simulated
			env->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
			SetSimulationStateOfAllEnvironmentObjects(env, iObject::ObjectSimulationState::FullSimulation);

			// We also want to update any space objects within range of this hub's environment
			Game::ObjectManager.GetAllObjectsWithinDistance(env, Game::C_SPACE_SIMULATION_HUB_RADIUS, objects,
				SimulationObjectManager::ObjectSearchOptions::NoSearchOptions);
			it_end = objects.end();
			for (it = objects.begin(); it != it_end; ++it)
			{
				// Make sure the object is valid
				spaceobject = (*it); if (!spaceobject) continue;

				// The object itself should be fully simulated since it is near this hub's environment
				spaceobject->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);

				// Test whether the object is itself an environment that contains other objects
				if (spaceobject->IsEnvironment())
				{
					// If so, upgrade all of its occupants to tactical simulation.  ONLY do this if tactical simulation would be an
					// upgrade; we may have already located an interior hub WITHIN the environment, in which case the occupants
					// would (and should) be set to full simulation.
					env = (iSpaceObjectEnvironment*)spaceobject;
					UpgradeSimulationStateOfAllEnvironmentObjects(env, iObject::ObjectSimulationState::TacticalSimulation);
				}
			}
		}
	}

}

// Evaluates the simulation state of all objects within an interior environment, in isolation. No objects in the surrounding space
// environment will be considered when determining simulation state.  
void SimulationStateManager::EvaluateSimulationStateInIsolatedInteriorEnvironment(iSpaceObjectEnvironment * environment)
{
	iSpaceObjectEnvironment *hubenv;

	// Parameter check
	if (!environment) return;

	// Check each space hub in turn; if this environment is a space hub then we set it & all its contents to full simulation
	std::vector<iSpaceObject*>::iterator it_end = m_space_simhubs.end();
	for (std::vector<iSpaceObject*>::iterator it = m_space_simhubs.begin(); it != it_end; ++it)
	{
		// We only need to consider this object if it is an environment
		if ((*it)->IsEnvironment())
		{
			hubenv = (iSpaceObjectEnvironment*)(*it);
			if (hubenv == environment)
			{
				// The environment is a space simulation hub; we therefore want to set it & all its contents to full simulation
				environment->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
				SetSimulationStateOfAllEnvironmentObjects(environment, iObject::ObjectSimulationState::FullSimulation);

				// We can return immediately now that everything is at full simulation
				return;
			}
		}
	}

	// Also check all interior simulation hubs to see if any are within the environment
	std::vector<iEnvironmentObject*>::iterator it2_end = m_env_simhubs.end();
	for (std::vector<iEnvironmentObject*>::iterator it2 = m_env_simhubs.begin(); it2 != it2_end; ++it2)
	{
		if ((*it2)->GetParentEnvironment() == environment)
		{
			// An interior simulation hub is within the environment, so again we want to set everything to full simulation
			environment->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
			SetSimulationStateOfAllEnvironmentObjects(environment, iObject::ObjectSimulationState::FullSimulation);

			// We can return immediately now that everything is at full simulation
			return;
		}
	}

	// There are no relevant simulation hubs.  Since we are considering the environment in isolation, we have no way
	// of determining whether tactical or strategic simulation is more appropriate.  Assign tactical simulation
	// in this case for prudence
	environment->SetSimulationState(iObject::ObjectSimulationState::TacticalSimulation);
	SetSimulationStateOfAllEnvironmentObjects(environment, iObject::ObjectSimulationState::TacticalSimulation);
}

// Populates the output vector with all space-based simulation hubs in the specified system.  Appends to any existing contents.
void SimulationStateManager::GetAllSpaceSimulationHubsInSystem(SpaceSystem *system, std::vector<iSpaceObject*> & outResult)
{
	// Parameter check
	if (!system) return;

	// Check each registered space simulation hub in turn
	std::vector<iSpaceObject*>::iterator it_end = m_space_simhubs.end();
	for (std::vector<iSpaceObject*>::iterator it = m_space_simhubs.begin(); it != it_end; ++it)
	{
		// If the hub is in this system then add it to the output vector
		if ((*it)->GetSpaceEnvironment() == system) outResult.push_back((*it));
	}	
}

// Populates the output vector with all interior simulation hubs in the specified system.  Appends to any existing contents.
void SimulationStateManager::GetAllInteriorSimulationHubsInSystem(SpaceSystem *system, std::vector<iEnvironmentObject*> & outResult)
{
	// Parameter check
	if (!system) return;

	// Check each registered interior simulation hub in turn
	std::vector<iEnvironmentObject*>::iterator it_end = m_env_simhubs.end();
	for (std::vector<iEnvironmentObject*>::iterator it = m_env_simhubs.begin(); it != it_end; ++it)
	{
		// If the hub is in this system then add it to the output vector
		if ((*it)->GetParentEnvironment() && (*it)->GetParentEnvironment()->GetSpaceEnvironment() == system)
			outResult.push_back((*it));
	}
}

// Method called whenever a space object enters an environment
void SimulationStateManager::ObjectEnteringSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment)
{
	// Parameter check; don't need to check 'environment' here since it is not used, only passed downstream (where it is checked)
	if (!object) return;

	// Take different action depending on whether the ship is a simulation hub
	if (object->IsSimulationHub())
	{
		// This is a simulation hub, so we want to update the surrounding environment to be fully simulated.  This object's
		// simulation state will be unaffected since we always fully simulate a simulation hub object
		SimulationHubEnteringSpaceEnvironment(object, environment);
	}
	else
	{
		// This is a normal object.  Determine its simulation state based upon any local simulation hubs
		object->SetSimulationState(DetermineSimulationStateForSpaceObject(object));
	}
}

// Method called whenever a space object leaves an environment
void SimulationStateManager::ObjectLeavingSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Take different action depending on whether the ship is a simulation hub
	if (object->IsSimulationHub())
	{
		// This is a simulation hub.  We want to reasses the surrounding environment in case it no longer needs 
		// to be fully simulated.  The hub's simulation state will be unaffected since we always fully simulate a simulation hub object
		SimulationHubLeavingSpaceEnvironment(object, environment);
	}
	else
	{
		// This is a normal object.  Disable its simulation while it is no longer in an environment
		object->SetSimulationState(iObject::ObjectSimulationState::NoSimulation);
	}
}

// Method called whenever an environment object enters an interior location
void SimulationStateManager::ObjectEnteringInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Take different action depending on whether the environment object is a simulation hub
	if (object->IsSimulationHub())
	{
		// This is a simulation hub, so we want to update the surrounding environment to be fully simulated.  This object's
		// simulation state will be unaffected since we always fully simulate a simulation hub object
		SimulationHubEnteringInteriorEnvironment(object, environment);
	}
	else
	{
		// This is a normal object.  Determine its simulation state based upon any local simulation hubs
		object->SetSimulationState(DetermineSimulationStateForInteriorObject(object));
	}
}

// Method called whenever an environment object leaves an interior location
void SimulationStateManager::ObjectLeavingInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Take different action depending on whether the environment object is a simulation hub
	if (object->IsSimulationHub())
	{
		// This is a simulation hub.  We want to reasses the surrounding environment in case it no longer needs 
		// to be fully simulated.  The hub's simulation state will be unaffected since we always fully simulate a simulation hub object
		SimulationHubLeavingInteriorEnvironment(object, environment);
	}
	else
	{
		// This is a normal object.  Disable its simulation while it is no longer in an environment
		object->SetSimulationState(iObject::ObjectSimulationState::NoSimulation);
	}
}

// Method called when a simulation hub enters the environment, or is created.  Affects simulation state of all its surroundings.  
// Passes control to the relevant method depending on the class of object
void SimulationStateManager::SimulationHubEnteringEnvironment(iObject * object)
{
	// Parameter check
	if (!object) return;

	// Take different action depending on the class of object
	if (object->GetObjectClass() == iObject::ObjectClass::SpaceObjectClass)
	{
		iSpaceObject *obj = (iSpaceObject*)object;
		SimulationHubEnteringSpaceEnvironment(obj, obj->GetSpaceEnvironment());
	}
	else if (object->GetObjectClass() == iObject::ObjectClass::EnvironmentObjectClass)
	{
		iEnvironmentObject *obj = (iEnvironmentObject*)object;

		// We need to make sure that the new environment is not null (which could happen in some scenarios)
		if (obj->GetParentEnvironment())
		{
			SimulationHubEnteringInteriorEnvironment(obj, obj->GetParentEnvironment());
		}
	}
}

// Method called when a space-based simulation hub enters the environment, or is created.  Affects simulation state of all its surroundings.  
void SimulationStateManager::SimulationHubEnteringSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Add to the list of space simulation hubs
	if (FindInVector<iSpaceObject*>(m_space_simhubs, object) == -1)
		AddSpaceSimulationHub(object);

	// Add this environment to the hub system list if it isn't already listed
	AddHubSystem(environment);

	// Now perform a full re-evaluation of the simulation state within this system
	EvaluateSimulationStateInSystem(environment);
}

// Method called when an environment object enters an environment, or is created.  Affects simulation state of all its surroundings
void SimulationStateManager::SimulationHubEnteringInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Add to the list of interior simulation hubs
	if (FindInVector<iEnvironmentObject*>(m_env_simhubs, object) == -1)
		AddInteriorSimulationHub(object);

	// Notify the environment that it definitely contains at least one interior simulation hub
	environment->NotifyIsContainerOfSimulationHubs(true);

	// Check whether the environment exists within a system (should always be the case)
	if (environment->GetSpaceEnvironment())
	{
		// Add the system that this environment exists in to the hub system list if it isn't already listed
		AddHubSystem(environment->GetSpaceEnvironment());

		// Now perform a full re-evaluation of the simulation state within this system
		EvaluateSimulationStateInSystem(environment->GetSpaceEnvironment());
	}
	else
	{
		// If the environment is not in a system (very unlikely) then we should at least perform a full 
		// re-evaluation of the environment itself
		EvaluateSimulationStateInIsolatedInteriorEnvironment(environment);
	}
}

// Method called when a simulation object leaves its current environment, or is removed.  Affects simulation state of all its surroundings.
// Passes control to the relevant method depending on the class of object
void SimulationStateManager::SimulationHubLeavingEnvironment(iObject * object)
{
	// Parameter check
	if (!object) return;

	// Take different action depending on the class of object
	if (object->GetObjectClass() == iObject::ObjectClass::SpaceObjectClass)
	{
		iSpaceObject *obj = (iSpaceObject*)object;
		SimulationHubLeavingSpaceEnvironment(obj, obj->GetSpaceEnvironment());
	}
	else if (object->GetObjectClass() == iObject::ObjectClass::EnvironmentObjectClass)
	{
		iEnvironmentObject *obj = (iEnvironmentObject*)object;

		// We need to make sure that the new environment is not null (which could happen in some scenarios)
		if (obj->GetParentEnvironment())
		{
			SimulationHubLeavingInteriorEnvironment(obj, obj->GetParentEnvironment());
		}
	}
}

// Method called when a space-based simulation hub enters the environment, or is created.  Affects simulation state of all its surroundings.  
void SimulationStateManager::SimulationHubLeavingSpaceEnvironment(iSpaceObject * object, SpaceSystem * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Remove from the collection of space simulation hubs
	int i = 0; while (FindInVector<iSpaceObject*>(m_space_simhubs, object) != -1 && (++i < 10))
		RemoveSpaceSimulationHub(object);

	// Check how many simulation hubs exist in this system.  If this was the only one, remove the system from the hub systems list
	if (DetermineSimulationHubCountInSystem(environment) == 0)
		RemoveHubSystem(environment);

	// Now perform a full re-evaluation of the simulation state within this system
	EvaluateSimulationStateInSystem(environment);
}

// Method called when an environment object enters an environment, or is created.  Affects simulation state of all its surroundings
void SimulationStateManager::SimulationHubLeavingInteriorEnvironment(iEnvironmentObject * object, iSpaceObjectEnvironment * environment)
{
	// Parameter check
	if (!object || !environment) return;

	// Remove from the collection of interior simulation hubs
	int i = 0; while (FindInVector<iEnvironmentObject*>(m_env_simhubs, object) != -1 && (++i < 10))
		RemoveInteriorSimulationHub(object);

	// Check whether the removal of this hub means the environment no longer contains any simulation hubs
	EvaluateEnvironmentAsHubContainer(environment);

	// Check whether the environment is located within a system (very likely)
	if (environment->GetSpaceEnvironment())
	{
		// Check how many simulation hubs exist in this system.  If this was the only one, remove the system from the hub systems list
		if (DetermineSimulationHubCountInSystem(environment->GetSpaceEnvironment()) == 0)
			RemoveHubSystem(environment->GetSpaceEnvironment());

		// Now perform a full re-evaluation of the simulation state within this system
		EvaluateSimulationStateInSystem(environment->GetSpaceEnvironment());
	}
	else
	{
		// If the environment is not in a system (very unlikely) then we should at least perform a full 
		// re-evaluation of the environment itself
		EvaluateSimulationStateInIsolatedInteriorEnvironment(environment);
	}
}

// Convenience method to test how many simulation hubs exist in the given system.  System may be NULL, in which 
// case the method will return the number of hubs without a system (i.e. whose system is also NULL)
int SimulationStateManager::DetermineSimulationHubCountInSystem(SpaceSystem *system)
{
	int count = 0;

	// Test the system that each space simulation hub exists in
	std::vector<iSpaceObject*>::const_iterator it_end = m_space_simhubs.end();
	for (std::vector<iSpaceObject*>::const_iterator it = m_space_simhubs.begin(); it != it_end; ++it)
	{
		if ((*it)->GetSpaceEnvironment() == system) 
			++count;
	}

	// Also test whether any interior simulation hubs are ultimately within this system
	std::vector<iEnvironmentObject*>::const_iterator it2_end = m_env_simhubs.end();
	for (std::vector<iEnvironmentObject*>::const_iterator it2 = m_env_simhubs.begin(); it2 != it2_end; ++it2)
	{
		if ((*it2)->GetParentEnvironment() && (*it2)->GetParentEnvironment()->GetSpaceEnvironment() == system)
			++count;
	}

	// Return the total count
	return count;
}

// Updates the simulation state of every relevant object in the specified environment to a specific level of simulation
void SimulationStateManager::SetSimulationStateOfAllEnvironmentObjects(iSpaceObjectEnvironment *environment, iObject::ObjectSimulationState state)
{
	// Parameter check
	if (!environment) return;

	// Iterate through all elements in the environment and set their simulation state.  This will also automatically
	// update the simulation state of any linked elements
	INTVECTOR3 elsize = environment->GetElementSize();
	for (int x = 0; x < elsize.x; ++x)
		for (int y = 0; y < elsize.y; ++y)
			for (int z = 0; z < elsize.z; ++z)
				environment->GetElementDirect(x, y, z)->SetSimulationState(state);

	// Iterate through all objects in the environment and change their state.  We have to do this separately
	// since they will only inherit state from their parent element upon moving to a different element.  We
	// therefore force it here to make sure the state is updated immediately
	std::vector<iEnvironmentObject*>::iterator it2_end = environment->Objects.end();
	for (std::vector<iEnvironmentObject*>::iterator it2 = environment->Objects.begin(); it2 != it2_end; ++it2)
	{
		// Set the simulation state of this object
		(*it2)->SetSimulationState(state);
	}
}

// Updates the simulation state of every relevant object in the specified environment to a specific level of simulation.  The object 
// state will only be set if this would be an upgrade (ComparisonResult::GreaterThan) on its current simulation state
void SimulationStateManager::UpgradeSimulationStateOfAllEnvironmentObjects(iSpaceObjectEnvironment *environment, iObject::ObjectSimulationState state)
{
	// Parameter check
	if (!environment) return;

	// Iterate through all objects in the environment
	std::vector<iEnvironmentObject*>::iterator it_end = environment->Objects.end();
	for (std::vector<iEnvironmentObject*>::iterator it = environment->Objects.begin(); it != it_end; ++it)
	{
		// Make sure this would actually be an upgrade to the object's current state
		if (iObject::CompareSimulationStates(state, (*it)->SimulationState()) == ComparisonResult::GreaterThan)
		{
			// Set the simulation state of this object
			(*it)->SetSimulationState(state);
		}
	}
}

// Tests whether an environment contains any interior simulation hubs, and sets the environment object flag accordingly
void SimulationStateManager::EvaluateEnvironmentAsHubContainer(iSpaceObjectEnvironment * environment)
{
	// Parameter check
	if (!environment) return;

	// Iterate through each interior simulation hub in turm
	std::vector<iEnvironmentObject*>::iterator it_end = m_env_simhubs.end();
	for (std::vector<iEnvironmentObject*>::iterator it = m_env_simhubs.begin(); it != it_end; ++it)
	{
		// If this hub is located within the environment then set the flag and quit immediately
		if ((*it) && (*it)->GetParentEnvironment() == environment)
		{
			environment->NotifyIsContainerOfSimulationHubs(true);
			return;
		}
	}

	// None of the interior simulation hubs are within this environment, so set the flag to false
	environment->NotifyIsContainerOfSimulationHubs(false);
}

// Add a hub system, which will be evaluated on a regular basis to determine correct simulation state
void SimulationStateManager::AddHubSystem(SpaceSystem *system)
{
	// Parameter check
	if (!system) return;

	// We don't want to add anything if the system is already in the collection
	if (FindInVector<SpaceSystem*>(m_hubsystems, system) != -1) return;

	// Add to the hub systems collection
	m_hubsystems.push_back(system);

	// Store the new size of the hub systems vector
	m_hubsystemcount = m_hubsystems.size();
}

// Removes a hub system, which will release it from the regular simulation state updates
void SimulationStateManager::RemoveHubSystem(SpaceSystem *system)
{
	// Parameter check
	if (!system) return;

	// Attempt to remove the system, if it exists
	RemoveFromVector<SpaceSystem*>(m_hubsystems, system);

	// Store the new size of the hub systems vector
	m_hubsystemcount = m_hubsystems.size();
}

// Default destructor
SimulationStateManager::~SimulationStateManager(void)
{

}

/* 
	Logic for determining simulation state of a space object:

		If no hubs in system > Strategic
		If space object near space hub > Full
		If space object near interior hub environment > Full
		Else if still in the same system > Tactical

	Logic for determining simulation state of an interior object:

		If no hubs in system > Strategic
		Same environment as interior hub > Full
		Environment near interior hub environment > Tactical
		Environment near space hub > Tactical
		Else if still in same system > Strategic



*/
