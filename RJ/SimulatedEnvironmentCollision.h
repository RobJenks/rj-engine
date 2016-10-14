#pragma once

#ifndef __SimulatedEnvironmentCollisionH__
#define __SimulatedEnvironmentCollisionH__

#include <string>
#include <vector>
#include "CompilerSettings.h"
#include "GameVarsExtern.h"


// Enumeration of possible (simulated) environment collision event types
enum SimulatedEnvironmentCollisionEventType { UnknownSimulatedEventType = 0, ElementDamaged, ElementDestroyed };

// Class holding data on a particular simulated event
struct SimulatedEnvironmentCollisionEvent
{
public:

	SimulatedEnvironmentCollisionEventType		EventType;
	int											ElementID;
	float										Value;

	// Default constructor
	SimulatedEnvironmentCollisionEvent(void) : EventType(SimulatedEnvironmentCollisionEventType::UnknownSimulatedEventType), ElementID(0), Value(0.0f) { }

	// Constructor providing all parameters
	SimulatedEnvironmentCollisionEvent(SimulatedEnvironmentCollisionEventType event_type, int element_id, float value)
		: EventType(event_type), ElementID(element_id), Value(value) { }

	// Copy constructor
	SimulatedEnvironmentCollisionEvent(const SimulatedEnvironmentCollisionEvent & other)
		: EventType(other.EventType), ElementID(other.ElementID), Value(other.Value) { }

	// Assignment operator
	SimulatedEnvironmentCollisionEvent & operator=(const SimulatedEnvironmentCollisionEvent & other)
	{
		EventType = other.EventType; ElementID = other.ElementID; Value = other.Value; return *this;
	}

	// Generates a string representation of the event
	std::string									ToString(void) const;
};

// Class representing an entire simulated environment collision
class SimulatedEnvironmentCollision : public std::vector<SimulatedEnvironmentCollisionEvent>
{
public:

	// Maintain a reference to the ID of the environment involved in this collision
	Game::ID_TYPE								EnvironmentID;

	// Clock time at which the last collision event occured
	float										LastEventTime;

	// Default constructor
	SimulatedEnvironmentCollision(void) : std::vector<SimulatedEnvironmentCollisionEvent>()
	{
		EnvironmentID = 0U;
		LastEventTime = 0.0f;
	}

	// Copy constructor
	SimulatedEnvironmentCollision(const SimulatedEnvironmentCollision & other);

	// Assignment operator
	SimulatedEnvironmentCollision & operator=(const SimulatedEnvironmentCollision & other);

	// Adds a new event to the collection
	void										AddEvent(const SimulatedEnvironmentCollisionEvent & new_event);

	// Resets the simulated collision data
	void										Reset(void)
	{
		EnvironmentID = 0U;
		LastEventTime = 0.0f;
		clear();
	}

	// Generates a string representation of the entire simulated collision
	std::string									ToString(void) const;
};



#endif


