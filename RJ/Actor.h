#pragma once

#ifndef __ActorH__
#define __ActorH__

#include "CompilerSettings.h"
#include "iEnvironmentObject.h"
#include "iConsumesOrders.h"
#include "SkinnedModel.h"
#include "ActorAttributes.h"
#include "Order.h"
class iObject;
class ActorBase;
class Order_ActorTravelToPosition;
class iSpaceObjectEnvironment;


class Actor : public iEnvironmentObject, public iConsumesOrders
{
public:

	// Methods to retrieve and set basic properties of the actor
	CMPINLINE std::string 		GetName(void) const					{ return m_name; }
	CMPINLINE void				SetName(const std::string & name)	{ m_name = name; }

	// Retrieve a reference to the ActorBase class for this instance
	CMPINLINE ActorBase *		GetBase(void)			{ return m_base; }

	// Attributes for this actor, including values inherited from the base actor
	ActorInstanceAttributes		Attributes;

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(Actor *source);

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
	void						SimulateObject(bool PermitMovement);

	// Perform the post-simulation update.  Pure virtual inherited from iObject base class
	CMPINLINE void				PerformPostSimulationUpdate(void) { }

	// Update the actor model for rendering.  NOTE: actor world matrix is calculated here, i.e. only when required for rendeirng
	void						UpdateForRendering(float timefactor);

	// Methods for retrieving & setting the object world matrix
	CMPINLINE XMFLOAT4X4 *		GetXWorldMatrix(void) { return &m_xworldmatrix; }
	void						SetWorldMatrix(D3DXMATRIX *world);
	void						SetWorldMatrix(XMMATRIX *world);
	
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
	Order::OrderResult						MoveToPosition(D3DXVECTOR3 position, float getwithin, bool run);

	// Order: Moves the actor to a target object, within a certain tolerance, providing the target is within the same environment
	Order::OrderResult						MoveToTarget(iEnvironmentObject *target, float getwithin, bool run);
	
	// Order: Travels to a destination using the environment nav network.  Spawns multiple child orders to get there.
	Order::OrderResult						TravelToPosition(Order_ActorTravelToPosition *order);

	// Turns the actor towards the specified position.  Y (vertical) coordinate is ignored.
	void									TurnTowardsPosition(const D3DXVECTOR3 &position);

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
	void									CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact);
	
	// Shutdown method to deallocate resources and remove the actor
	void									Shutdown(void);

	// Constructor & destructor
	Actor(void);
	Actor(ActorBase *actorbase);
	~Actor(void);



private:

	ActorBase *							m_base;
	SkinnedModelInstance 				m_model;
	std::string							m_name;

	// Movement and physics parameters for this actor
	float								m_turnrate;						// Radians/sec turn rate

	// X-world matrix for this actor
	XMFLOAT4X4							m_xworldmatrix;
};


// Turn (about the Y axis) by the specified number of radians
CMPINLINE void Actor::Turn(float angle)
{
	// Make sure the turn angle is within our maximum turn rate
	if (angle < 0.0f)	angle = max(angle, -m_turnrate * Game::TimeFactor);
	else				angle = min(angle, m_turnrate * Game::TimeFactor);

	// Generate a delta quaternion for this angle and apply it to our orientation
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &UP_VECTOR, angle);
	AddDeltaOrientation(q);
}


// Turn (about the Y axis) by the specified number of radians.  Does not limit by the actor's turn rate - will ensure that
// we turn by the specified angle
CMPINLINE void Actor::Turn_NoLimit(float angle)
{
	// Generate a delta quaternion for this angle and apply it to our orientation
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &UP_VECTOR, angle);
	AddDeltaOrientation(q);
}


#endif