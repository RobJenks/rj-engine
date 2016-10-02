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

	// Structure holding an event that occurs during the collision
	struct EventDetails
	{
		// The entity that is the subject of this event
		Game::ID_TYPE								EntityID;
		Game::EntityType							EntityType;

		// The event that occured
		CollisionEventType							EventType;

		// The clock time (secs) at which this event occurs, relative to the overall start time (= 0.0)
		float										EventTime;

		// Duration of the event (secs)
		float										EventDuration;

		// Default constructor
		EventDetails(void) :	EntityID(0), EntityType(Game::EntityType::E_Unknown), EventType(CollisionEventType::Unknown), 
								EventTime(0.0f), EventDuration(0.0f) { }

		// Constructor with all core parameters
		EventDetails(CollisionEventType eventType, float eventTime, float eventDuration, Game::ID_TYPE entity, Game::EntityType entityType)
			: EventType(eventType), EventTime(eventTime), EventDuration(eventDuration), EntityID(entity), EntityType(entityType) { }

		// Copy constructor
		EventDetails(const EventDetails & other) :	EntityID(other.EntityID), EntityType(other.EntityType), EventType(other.EventType), 
													EventTime(other.EventTime), EventDuration(other.EventDuration) { }
	};

	// Vector of events, sorted in time order
	std::vector<EventDetails>						Events;

	// Index of the last event that was processed in the sequence.  Can be used to execute the events
	// more efficiently since we only need to test events [last n), and can exit at the first future event
	// Set as an int rather than std::vector<..>::size_type since the latter is unsigned, and need to initialise as -1
	int												LastEventExecuted;

	// Reference to the object that is colliding with this environment
	ObjectReference<iObject*>						Collider;

	// The time (clock time, secs) at which the collision event began
	float											CollisionStartTime;

	// Flag indicating whether this represents a valid collision that is currently is progress
	bool											IsActive;

	// Collection of intersection data used to generate these collision events, for reference during collision processing
	ElementIntersectionData							IntersectionData;



	// Adds an event to the sequence
	void											AddEvent(CollisionEventType eventType, float eventTime, float eventDuration, Game::ID_TYPE entity, Game::EntityType entityType);

	// Adds an element intersection event
	void											AddElementIntersection(int elementID, float intersectTime, float intersectDuration);


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


protected:

};


#endif



