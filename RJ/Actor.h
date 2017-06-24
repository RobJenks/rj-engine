#pragma once

#ifndef __ActorH__
#define __ActorH__

#include "CompilerSettings.h"
#include "iEnvironmentObject.h"
#include "EntityAI.h"
#include "SkinnedModel.h"
#include "ActorAttributes.h"
class iObject;
class ActorBase;
class Order;
class Order_ActorMoveToPosition;
class Order_ActorMoveToTarget;
class Order_ActorTravelToPosition;
class iSpaceObjectEnvironment;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Actor : public ALIGN16<Actor>, public iEnvironmentObject, public EntityAI
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(Actor)

	// Methods to retrieve and set basic properties of the actor
	CMPINLINE std::string 		GetName(void) const					{ return m_name; }
	CMPINLINE void				SetName(const std::string & name)	{ m_name = name; }

	// Retrieve a reference to the ActorBase class for this instance
	CMPINLINE ActorBase *		GetBase(void)			{ return m_base; }

	// Attributes for this actor, including values inherited from the base actor
	ActorInstanceAttributes		Attributes;

	// Returns the turn rate of this actor
	CMPINLINE float				GetTurnRate(void) const	{ return m_turnrate; }

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void						InitialiseCopiedObject(Actor *source);

	// Causes the actor to recalculate all its properties & final attribute values.  Called when changes are made to the actor attributes
	void						RecalculateAttributes(void);

	// Recalculates the actor world matrix following a change to its position or orientation
	void						RecalculateWorldMatrix(void);

	// Method to force an immediate recalculation of player position/orientation, for circumstances where we cannot wait until the
	// end of the frame (e.g. for use in further calculations within the same frame that require the updated data)
	CMPINLINE void				RefreshPositionImmediate(void)
	{
		// Recalculate the world matrix and collision boxes from our new position/orientation data
		RecalculateWorldMatrix();
	}

	// Simulates the actor, for example processes the actor's current order queue.  Called every frame.
	void						SimulateObject(void);

	// Update the actor model for rendering.  NOTE: actor world matrix is calculated here, i.e. only when required for rendeirng
	void						UpdateForRendering(float timefactor);

	// Methods to get and set the model associated with this actor
	CMPINLINE SkinnedModelInstance			GetModel(void)							{ return m_model; }
	CMPINLINE SkinnedModelInstance *		GetModelReference(void)					{ return &m_model; }
	CMPINLINE void							SetModel(SkinnedModelInstance model)	{ m_model = model; }

	// Methods to immediately set the primary animation for this actor instance (without any blending etc)
	void									SetAnimationImmediate(const std::string & anim);
	void									SetAnimationImmediate(const AnimationClip * anim);

	// Returns a bool indicating whether an actor can accept a specified class of order.  Can be overrideen by more specific subclasses if necessary
	bool									CanAcceptOrderType(Order::OrderType type);

	// Method to process the specified order.  Called when processing the full queue.  Return value indicates whether order is now completed & can be removed
	Order::OrderResult						ProcessOrder(Order *order);

	// Order: Moves the actor to a target position in the environment, within a certain tolerance
	Order::OrderResult						MoveToPosition(Order_ActorMoveToPosition & order);

	// Order: Moves the actor to a target object, within a certain tolerance, providing the target is within the same environment
	Order::OrderResult						MoveToTarget(Order_ActorMoveToTarget & order);
	
	// Order: Travels to a destination using the environment nav network.  Spawns multiple child orders to get there.
	Order::OrderResult						TravelToPosition(Order_ActorTravelToPosition & order);

	// Turns the actor towards the specified position.  Y (vertical) coordinate is ignored.
	void									TurnTowardsPosition(const FXMVECTOR position);

	// Move forwards along the current heading, either running or walking
	void									Move(Direction direction, bool run);

	// Turn (about the Y axis) by the specified number of radians
	CMPINLINE void							Turn(float angle);

	// Turn (about the Y axis) by the specified number of radians.  Does not limit by the actor's turn rate - will ensure that
	// we turn by the specified angle
	CMPINLINE void							Turn_NoLimit(float angle);

	// Attempt to perform a jump.  Tests that the actor is on the ground and able to jump
	void									Jump(void);

	// Method called when this object collides with another.  Virtual inheritance from iObject
	void									CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact);

	// Event triggered upon destruction of the entity
	void									DestroyObject(void);

	// Shutdown method to deallocate resources and remove the actor
	void									Shutdown(void);

	// Custom debug string function
	std::string								DebugString(void) const;

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void									ProcessDebugCommand(GameConsoleCommand & command);

	// Constructor & destructor
	Actor(void);
	Actor(ActorBase *actorbase);
	~Actor(void);



protected:

	ActorBase *							m_base;
	SkinnedModelInstance 				m_model;
	
	// Movement and physics parameters for this actor
	float								m_turnrate;						// Radians/sec turn rate

	// Moves the actor to a target position in the same environment, within a certain tolerance sq.  Returns a
	// flag indicating whether we have reached the target (true) or are still in progress (false)
	bool _MoveToPosition(const FXMVECTOR position, float tolerance_sq, bool run);

};

// Turn (about the Y axis) by the specified number of radians
CMPINLINE void Actor::Turn(float angle)
{
	// Make sure the turn angle is within our maximum turn rate
	if (angle < 0.0f)	angle = max(angle, -m_turnrate * Game::TimeFactor);
	else				angle = min(angle, m_turnrate * Game::TimeFactor);
	
	// Generate a delta quaternion for this angle and apply it to our orientation
	ChangeEnvironmentOrientation(XMQuaternionRotationNormal(UP_VECTOR, angle));
}


// Turn (about the Y axis) by the specified number of radians.  Does not limit by the actor's turn rate - will ensure that
// we turn by the specified angle
CMPINLINE void Actor::Turn_NoLimit(float angle)
{
	// Generate a delta quaternion for this angle and apply it to our orientation
	ChangeEnvironmentOrientation(XMQuaternionRotationNormal(UP_VECTOR, angle));
}


#endif