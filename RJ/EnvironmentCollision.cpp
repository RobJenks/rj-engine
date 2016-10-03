#include "EnvironmentCollision.h"

// Initialise static comparator for binary search comparisons
EnvironmentCollision::_EventTimeOrderComparator EnvironmentCollision::EventTimeOrderComparator = EnvironmentCollision::_EventTimeOrderComparator();


// Default constructor
EnvironmentCollision::EnvironmentCollision(void)
	:
	IsActive(false), m_nextevent(0U), Collider(), CollisionStartTime(0.0f), ClosingVelocity(0.0f)
{
}

// Adds an event to the sequence
void EnvironmentCollision::AddEvent(CollisionEventType eventType, float eventTime, float eventDuration, Game::ID_TYPE entity, Game::EntityType entityType)
{
	// Simply push this generic event into the event sequence.  Minimal parameter validation since this is an internal process
	// and responsibility is on the calling methods to validate their own actions
	EventDetails new_event = EventDetails(eventType, eventTime, max(eventDuration, Game::C_EPSILON), entity, entityType);

	// We want to maintain the chonological order of the event sequence when adding this item.  Use lower_bound to 
	// find the first event that takes place AFTER this one
	std::vector<EventDetails>::iterator it = std::lower_bound(Events.begin(), Events.end(), new_event, EnvironmentCollision::EventTimeOrderComparator);
	Events.insert(it, new_event);
}

// Adds an element intersection event
void EnvironmentCollision::AddElementIntersection(int elementID, float intersectTime, float intersectDuration)
{
	// Push into the event sequence
	AddEvent(CollisionEventType::ElementCollision, intersectTime, intersectDuration, elementID, Game::EntityType::E_Element);
}



// Copy constructor
EnvironmentCollision::EnvironmentCollision(const EnvironmentCollision & other)
{
	// Copy all data across directly
	Events = other.Events;
	m_nextevent = other.m_nextevent;
}

// Default destructor
EnvironmentCollision::~EnvironmentCollision(void)
{

}

// Comparator for sorting/searching events in chronological order
bool EnvironmentCollision::_EventTimeOrderComparator::operator() (const EventDetails & lhs, const EventDetails & rhs) const
{
	return (lhs.EventTime <= rhs.EventTime);
}
