#include "SimulatedEnvironmentCollision.h"


// Generates a string representation of the event
std::string SimulatedEnvironmentCollisionEvent::ToString(void) const
{
	switch (EventType)
	{
		case SimulatedEnvironmentCollisionEventType::ElementDamaged:
			return concat("Element ")(ElementID)(" took ")(Value)(" damage").str();
		case SimulatedEnvironmentCollisionEventType::ElementDestroyed:
			return concat("Element ")(ElementID)(" was destroyed").str().c_str();
		default:
			return "[Unknown event]";
	}
}

// Adds a new event to the collection
void SimulatedEnvironmentCollision::AddEvent(const SimulatedEnvironmentCollisionEvent & new_event)
{
	// Push into the collection
	push_back(new_event);

	// Record the fact that we have a new event at this time 
	LastEventTime = Game::TimeFactor;
}

// Generates a string representation of the entire simulated collision
std::string SimulatedEnvironmentCollision::ToString(void) const
{
	// Use a stringbuilder to concatenate the text for each string together
	std::ostringstream ss;

	// Iterate over each event in turn
	SimulatedEnvironmentCollision::const_iterator it_end = end();
	for (SimulatedEnvironmentCollision::const_iterator it = begin(); it != it_end; ++it)
	{
		ss << (*it).ToString() << "\n";
	}

	// Return the full string representation
	return ss.str();
}