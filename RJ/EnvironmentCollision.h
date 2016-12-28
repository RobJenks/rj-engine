#pragma once

#ifndef __EnvironmentCollisionH__
#define __EnvironmentCollisionH__

#include "GameVarsExtern.h"
#include "ObjectReference.h"
#include "ElementIntersection.h"


class EnvironmentCollision
{
public:

	// Enumeration of possible event types during a collision
	enum CollisionEventType
	{
		Unknown = 0,
		ElementCollision
	};

	// Enumeration of possible states that this collision can be in
	enum EnvironmentCollisionState { 
		Active = 0,						// Collision is currently in progress
		Inactive,						// Collision is not active, reason unknown
		Inactive_NoCollision,			// Collision never took place (no intersection)
		Inactive_Destroyed,				// Collider was destroyed
		Inactive_Deflected,				// Collider was deflected and did not penetrate the environment
		Inactive_Stopped,				// Collider ran out of momentum
		Inactive_Completed };			// Collision event completed normally

	// Enumeration of possible effects of an element collision
	enum ElementCollisionResult { NoImpact = 0, ElementDamaged, ElementDestroyed };

	// Structure holding an event that occurs during the collision
	struct EventDetails
	{
		// Index of this event in the sequence
		std::vector<EventDetails>::size_type		Index;

		// The entity that is the subject of this event
		Game::ID_TYPE								EntityID;
		Game::EntityType							EntityType;

		// The event that occured
		CollisionEventType							EventType;

		// The clock time (secs) at which this event occurs, relative to the overall start time (= 0.0)
		float										EventTime;

		// Duration of the event (secs)
		float										EventDuration;

		// Parameters for the event
		float										Param1;					// E.g. the degree of intersection with an element

		// Default constructor
		CMPINLINE EventDetails(void) : EntityID(0), EntityType(Game::EntityType::E_Unknown), EventType(CollisionEventType::Unknown),
									   EventTime(0.0f), EventDuration(0.0f), Param1(0.0f) { }

		// Constructor with all core parameters
		CMPINLINE EventDetails(CollisionEventType eventType, float eventTime, float eventDuration, Game::ID_TYPE entity, Game::EntityType entityType, float param1)
			: EventType(eventType), EventTime(eventTime), EventDuration(eventDuration), EntityID(entity), EntityType(entityType), Param1(param1) { }

		// Copy constructor
		CMPINLINE EventDetails(const EventDetails & other) : EntityID(other.EntityID), EntityType(other.EntityType), EventType(other.EventType),
							 								 EventTime(other.EventTime), EventDuration(other.EventDuration), Param1(other.Param1) { }
	};

	// Vector of events, sorted in time order
	std::vector<EventDetails>						Events;

	// Returns the number of events in this collision
	CMPINLINE std::vector<EventDetails>::size_type	GetEventCount(void) const						{ return Events.size(); }

	// Index of the next event to be processed in the sequence.  Can be used to execute the events
	// more efficiently since we only need to test events [next n), and can exit at the first future event
	CMPINLINE std::vector<EventDetails>::size_type	GetNextEvent(void) const						{ return m_nextevent; };

	// Marks the current event as completed, and increments the counter to the next one.  If this brings us to
	// the end of the event sequence, the object will be inactivated
	CMPINLINE void									CurrentEventCompleted(void)
	{
		// Move the event counter, inactivating the object if we are done
		if (++m_nextevent >= GetEventCount()) EndCollision(EnvironmentCollisionState::Inactive_Completed);
	}

	// Performs post-processing on the collision object before it is returned for evaluation
	void											Finalise(void);

	// Reference to the object that is colliding with this environment
	ObjectReference<iActiveObject>					Collider;

	// The time (clock time, secs) at which the collision event began
	float											CollisionStartTime;

	// Closing velocity between the object and environment.  Will be adjusted downwards as each collision effect is applied
	// This represents the incoming velocity of the object, relative to a static environment
	float											ClosingVelocity;

	// Flag indicating whether an intersection actually takes place
	bool											Intersects;

	// Return the current state of this collision
	CMPINLINE EnvironmentCollisionState				GetState(void) const							{ return m_state; }

	// Flag indicating whether this represents a valid collision that is currently is progress
	CMPINLINE bool									IsActive(void) const							{ return (m_state == EnvironmentCollision::EnvironmentCollisionState::Active); }

	// Activate the collision event
	CMPINLINE void									Activate(void)									{ m_state = EnvironmentCollisionState::Active; }

	// Deactivate the collision event
	CMPINLINE void									EndCollision(void)								{ m_state = EnvironmentCollisionState::Inactive; }
	CMPINLINE void									EndCollision(EnvironmentCollisionState reason)	{ m_state = reason; }

	// Collection of intersection data used to generate these collision events, for reference during collision processing
	ElementIntersectionData							IntersectionData;

	// Flag indicating whether this collision has penetrated the outer hull of the environment.  Updated as the 
	// collision is processed
	bool											HasPenetratedOuterHull;

	// Store the pre-impact trajectory of the projectile.  If this becomes a penetrating collision we will then
	// revert the projectile to travel along its pre-impact trajectory (rather than the newly-calculated deflection
	// trajectory calculated by the physics engine)
	AXMVECTOR										ColliderPreImpactTrajectory;

	// Adds an event to the sequence
	void											AddEvent(CollisionEventType eventType, float eventTime, float eventDuration, Game::ID_TYPE entity, Game::EntityType entityType, float param1);

	// Adds an element intersection event
	void											AddElementIntersection(int elementID, float intersectTime, float intersectDuration, float degree);

	// Make the environment collision data immediately-executable so that it can be simulated immediately 
	// in this method, without waiting for the actual intersection times to occur
	void											MakeImmediatelyExecutable(void);

	// Default constructor
	EnvironmentCollision(void);

	// Copy constructor
	EnvironmentCollision(const EnvironmentCollision & other);

	// Default destructor
	~EnvironmentCollision(void);

	// Functor for sorting/searching events in chronological order
	static struct _EventTimeOrderComparator
	{
		bool operator() (const EventDetails & lhs, const EventDetails & rhs) const;
	};
	static _EventTimeOrderComparator EventTimeOrderComparator;

	// Static method to return a string description of each possible collision state
	static std::string								GetStateDescription(EnvironmentCollisionState state);

protected:

	// Index of the next event to be executed
	std::vector<EventDetails>::size_type							m_nextevent;

	// Current state of the collision event
	EnvironmentCollision::EnvironmentCollisionState					m_state;

};


#endif



