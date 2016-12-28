#include "EnvironmentCollision.h"

// Initialise static comparator for binary search comparisons
EnvironmentCollision::_EventTimeOrderComparator EnvironmentCollision::EventTimeOrderComparator = EnvironmentCollision::_EventTimeOrderComparator();


// Default constructor
EnvironmentCollision::EnvironmentCollision(void)
	:
	ClosingVelocity(0.0f), Collider(), ColliderPreImpactTrajectory(FORWARD_VECTOR), CollisionStartTime(0.0f),
	HasPenetratedOuterHull(false), Intersects(false), m_nextevent(0U), m_state(EnvironmentCollisionState::Inactive)
{
}

// Copy constructor
EnvironmentCollision::EnvironmentCollision(const EnvironmentCollision & other)
	:
	ClosingVelocity(other.ClosingVelocity), Collider(other.Collider), ColliderPreImpactTrajectory(other.ColliderPreImpactTrajectory),
	CollisionStartTime(other.CollisionStartTime), Events(other.Events), HasPenetratedOuterHull(other.HasPenetratedOuterHull),
	Intersects(other.Intersects), IntersectionData(other.IntersectionData), m_nextevent(other.GetNextEvent()),
	m_state(other.GetState())
{
}

// Adds an event to the sequence
void EnvironmentCollision::AddEvent(CollisionEventType eventType, float eventTime, float eventDuration, Game::ID_TYPE entity, Game::EntityType entityType, float param1)
{
	// Simply push this generic event into the event sequence.  Minimal parameter validation since this is an internal process
	// and responsibility is on the calling methods to validate their own actions
	EventDetails new_event = EventDetails(eventType, eventTime, max(eventDuration, Game::C_EPSILON), entity, entityType, param1);

	// We want to maintain the chonological order of the event sequence when adding this item.  Use lower_bound to 
	// find the first event that takes place AFTER this one
	std::vector<EventDetails>::iterator it = std::lower_bound(Events.begin(), Events.end(), new_event, EnvironmentCollision::EventTimeOrderComparator);
	Events.insert(it, new_event);
}

// Adds an element intersection event
void EnvironmentCollision::AddElementIntersection(int elementID, float intersectTime, float intersectDuration, float degree)
{
	// Push into the event sequence
	AddEvent(CollisionEventType::ElementCollision, intersectTime, intersectDuration, elementID, Game::EntityType::E_Element, degree);
}


// Default destructor
EnvironmentCollision::~EnvironmentCollision(void)
{

}

// Make the environment collision data immediately-executable so that it can be simulated immediately 
// in this method, without waiting for the actual intersection times to occur
void EnvironmentCollision::MakeImmediatelyExecutable(void)
{
	// We simply need to wind the collision start time back.  All event times are relative
	// to the collision start time, so all will remain in sequence and eligible to fire immediately
	CollisionStartTime = -100000000.f;
}


// Performs post-processing on the collision object before it is returned for evaluation
void EnvironmentCollision::Finalise(void)
{
	bool outer = false;

	// Post-process the event collection
	std::vector<EnvironmentCollision::EventDetails>::size_type n = Events.size();
	for (std::vector<EnvironmentCollision::EventDetails>::size_type index = 0; index < n; ++index)
	{
		// Add a unique sequence number to each collision event
		Events[index].Index = index;
	}

}


// Comparator for sorting/searching events in chronological order
bool EnvironmentCollision::_EventTimeOrderComparator::operator() (const EventDetails & lhs, const EventDetails & rhs) const
{
	return (lhs.EventTime <= rhs.EventTime);
}


// Static method to return a string description of each possible collision state
std::string EnvironmentCollision::GetStateDescription(EnvironmentCollisionState state)
{
	switch (state)
	{
		case EnvironmentCollisionState::Active:					return "Active collision";
		case EnvironmentCollisionState::Inactive:				return "Inactive";
		case EnvironmentCollisionState::Inactive_Completed:		return "Inactive: Collision completed";
		case EnvironmentCollisionState::Inactive_Deflected:		return "Inactive: Collider was deflected";
		case EnvironmentCollisionState::Inactive_Destroyed:		return "Inactive: Collider was destroyed";
		case EnvironmentCollisionState::Inactive_NoCollision:	return "Inactive: No collision occured";
		case EnvironmentCollisionState::Inactive_Stopped:		return "Inactive: Collider ran out of momentum";
		default:												return "<UNKNOWN STATE>";
	}
};	
