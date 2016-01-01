#include "CompilerSettings.h"
#include "FastMath.h"
#include "SkinnedModel.h"
#include "iSpaceObjectEnvironment.h"
#include "Order.h"
#include "Order_ActorMoveToPosition.h"
#include "Order_ActorMoveToTarget.h"
#include "Order_ActorTravelToPosition.h"
#include "Actor.h"
#include "ActorBase.h"

// Default constructor; for maintaining the constructor hierarchy back to iObject
Actor::Actor(void)
{
	// This default constructor is called from the main constructor.  It serves only to maintain the link to the rest of 
	// the inheritance hierarchy, so that this call will be passed back to EnvObject > ActiveObject > iObject etc.
}

// Constructor
//#ifdef RJ_CPP11_SUPPORT
Actor::Actor(ActorBase *actorbase) : Actor()
//#else
//Actor::Actor(ActorBase *actorbase)			// Perform full construction if C++11 constructor delegation not supported
//#endif
{
	// Store a reference back to the actor base class from which we were created
	m_base = actorbase;

	// Store the string code of the base actor definition and use it to derive a new unique instance code
	SetCode(actorbase->GetCode());

	// Set the object type
	SetObjectType(iObject::ObjectType::ActorObject);

	// This class of object will perform full collision detection by default
	SetCollisionMode(Game::CollisionMode::FullCollision);

	// Copy certain data from the base actor class
	SetSize( actorbase->GetSize() );
	SetMass( actorbase->GetMass() );

	// Initialise key parameters to null
	m_parent = NULL;
	m_name = "";
	m_turnrate = Game::C_ACTOR_DEFAULT_TURN_RATE;
}

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void Actor::InitialiseCopiedObject(Actor *source)
{
	// Pass control to all base classes
	iEnvironmentObject::InitialiseCopiedObject((iEnvironmentObject*)source);
	EntityAI::InitialiseCopiedObject((EntityAI*)source);
}

// Causes the actor to recalculate all its properties & final attribute values.  Called when changes are made to the actor attributes
void Actor::RecalculateAttributes(void)
{
	int attr; float modifier;

	// Set all attributes to their base values by default
	for (int i=0; i<(int)ActorAttr::A_COUNT; i++)
		Attributes[i].Value = Attributes[i].BaseValue;

	// Now apply all direct effects from one attribute to another
	vector<ActorAttributeGeneration::ActorAttributeEffect>::const_iterator it_end = ActorAttributeGeneration::ActorAttributeEffects.end();
	for (vector<ActorAttributeGeneration::ActorAttributeEffect>::const_iterator it = ActorAttributeGeneration::ActorAttributeEffects.begin(); it != it_end; ++it)
	{
		// Determine the position of the current value within the range of values for this attribute
		attr = (int)it->source;
		ActorAttributeGeneration::ActorAttributeGenerationParams arange = ActorAttributeGeneration::ActorAttributeParams[attr];
		if (Attributes[attr].Value > arange.base)		
		{
			// We are greater than the base value, so use [Modifier = EffectAtBase + (((Val-Base)/(Max-Base)) * (EffectAtMax-EffectAtBase))]
			modifier = it->atbase + (((Attributes[attr].Value - arange.base)/(arange.max - arange.base)) * (it->atmax - it->atbase));
		}
		else if (Attributes[attr].Value < arange.base)
		{
			// We are lower than the base value, so use [Modifier = EffectAtBase + ( (1-((Val-Min)/(Base-Min))) * (EffectAtMin-EffectAtBase))]
			modifier = it->atbase + ( (1.0f - ((Attributes[attr].Value - arange.min)/(arange.base-arange.min))) * (it->atmin - it->atbase));
		}
		else modifier = it->atbase;

		// Now apply this modifier to update the current target attribute value
		Attributes[(int)it->target].Value += (Attributes[(int)it->target].BaseValue * modifier);
	}

	// Derived attributes: walk & run speed.  If at default of 1.0 then scale from our current run speed
	if (Attributes[ActorAttr::A_WalkSpeed].Value == 1.0f)	Attributes[ActorAttr::A_WalkSpeed].Value = 
															Attributes[ActorAttr::A_RunSpeed].Value * Game::C_ACTOR_DEFAULT_WALK_MULTIPLIER;
	if (Attributes[ActorAttr::A_StrafeSpeed].Value == 1.0f)	Attributes[ActorAttr::A_StrafeSpeed].Value = 
															Attributes[ActorAttr::A_RunSpeed].Value * Game::C_ACTOR_DEFAULT_STRAFE_MULTIPLIER;
}

// Update the actor model for rendering.  NOTE: actor world matrix is calculated HERE, i.e. only when required for rendeirng
void Actor::UpdateForRendering(float timefactor)
{
	// Update the actor world matrix based on current position / orientation / etc.
	RecalculateWorldMatrix();

	// Send the new world matrix to our model instance and calculate the correct bone transforms for its current animation
	XMStoreFloat4x4(&m_model.World, m_worldmatrix);
	m_model.Update(timefactor);
}

// Recalculates the actor world matrix following a change to its position or orientation
void Actor::RecalculateWorldMatrix(void)
{
	// Make sure we have a valid underlying model to retrieve data from
	if (!(m_model.Model)) return;

	// Get each component we will require in order to derive the world matrix
	// WM = Scaling * RotationFix * TranslationFix * ActorRotation * ActorTranslation
	//	  = [ScaleRotationTranslationAdjustment(Precalculated)] * ActorRotation * ActorTranslation
	XMMATRIX mscalerottransadj = XMLoadFloat4x4(m_model.Model->GetScaleRotationTranslationAdjustmentReference());
	XMMATRIX mrot = XMMatrixRotationQuaternion(m_orientation);
	XMMATRIX mtrans = XMMatrixTranslationFromVector(m_position);

	// Set the world matrix (World = ScaleRotTransAdjustment * Rot * Trans).  Also manually invalidate the object
	// collision OBB since we are setting the matrix directly, and may not otherwise trigger the invalidation
	// through normal SetPosition() / SetOrientation() methods
	SetWorldMatrix(XMMatrixMultiply(XMMatrixMultiply(mscalerottransadj, mrot), mtrans));
	CollisionOBB.Invalidate();
}

void Actor::SetAnimationImmediate(const std::string & anim)
{
	// Attempt to retrieve the animation with this string code, and call the overloaded method. Compiler should inline this
	SetAnimationImmediate(m_model.Model->GetAnimation(anim));
}

void Actor::SetAnimationImmediate(const AnimationClip *anim)
{
	// Store the new clip as the current animation.  In future we can handle transitions etc.
	m_model.CurrentAnimation = anim;
}

// Attempt to perform a jump.  Tests that the actor is on the ground and able to jump
void Actor::Jump(void)
{
	// Make sure we have all required data
	if (!m_parent()) return;

	// Determine the jump force for this actor.  Force is mass-independent, i.e. post-division through by inverse mass.  
	// TODO: Change this to pull from actor attributes
	float jumpforce = 4.0f;

	// We can only jump if we are currently on the ground
	if (IsOnGround())
	{
		ApplyLocalLinearForceDirect(XMVectorSetY(NULL_VECTOR, jumpforce));
	}	
}

// Returns a bool indicating whether an actor can accept a specified class of order.  Can be overrideen by more specific subclasses if necessary
bool Actor::CanAcceptOrderType(Order::OrderType type)
{
	// Return true for any order type an actor (regardless of type) can accept
	switch (type)
	{
		// Order types below are all valid options for an actor
		case Order::OrderType::ActorMoveToPosition:
		case Order::OrderType::ActorMoveToTarget:
		case Order::OrderType::ActorTravelToPosition:
			
			return true; 

		// Otherwise we return false
		default:
			return false;
	}
}

// Simulates the actor, for example processes the actor's current order queue.  Called every frame.
void Actor::SimulateObject(void)
{
	// Degree of simulation is based upon the current simulation state
	switch (m_simulationstate)
	{
		/* Full simulation of the actor */
		case iObject::ObjectSimulationState::FullSimulation:

			ProcessOrderQueue(Game::ClockDelta);							// Process any orders in the actor's queue

			SimulateObjectPhysics();										// Simulate all object physics, generally just gravity 
																			// plus any impact force that has been applied.  This must happen LAST.
			break;

		/* Else (e.g. no simulation) */
		default:

			ProcessOrderQueue(Game::ClockDelta);							// Process any orders in the actor's queue
			break;
	}
}

// Method to process the specified order.  Called when processing the full queue.  Returns true if order is now completed & can be removed
Order::OrderResult Actor::ProcessOrder(Order *order)
{
	// Take different action depending on the order type
	if (!order) return Order::OrderResult::InvalidOrder;
	switch (order->GetType())
	{
		// Move to position.  Specifies position and the distance to which we must move within
		case Order::OrderType::ActorMoveToPosition:
			return MoveToPosition((Order_ActorMoveToPosition&)*order);

		// Move to target.  Specifies the target and the distance to which we must move within
		case Order::OrderType::ActorMoveToTarget:
			return MoveToTarget((Order_ActorMoveToTarget&)*order);

		// Travel to position.  Navigates to a destination using the environment nav network
		case Order::OrderType::ActorTravelToPosition:
			return TravelToPosition((Order_ActorTravelToPosition&)*order);


			// If this is not an order we can execute, return an invalid order result to have it removed from the queue
		default:
			return Order::OrderResult::InvalidOrder;
	}

}


// Moves the actor to a target position in the same environment, within a certain tolerance sq.  Returns a
// flag indicating whether we have reached the target (true) or are still in progress (false)
bool Actor::_MoveToPosition(const FXMVECTOR position, float tolerance_sq, bool run)
{
	// Completion check:  See whether we have reached the target position.  Calculated (squared) distance to target and threshold.
	// Only consider distance in x/z dimensions - y (vertical) dimension is ignored.
	// Swizzle both positions to get x/z as the first two parameters, then treat as 2-vectors when subtracting and taking the lengthsq
	float distsq = XMVectorGetX(XMVector2LengthSq(
		XMVectorSubtract(XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Z>(m_envposition),
						 XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Z>(position))));

	// If we are within the close distance then we can stop moving and complete the order
	if (distsq < tolerance_sq)
	{
		/* Transition to a stopping/idle animation of some kind */
		return true;
	}	

	// We need to turn towards to the target position
	TurnTowardsPosition(position);

	// Move forwards in this direction
	// TODO: switch to walking when close, if we are currently running?
	Move(Direction::Up, run);

	// We have executed the order this cycle and it is still in progress
	return false;
}

// Order: Moves the actor to a target position in the same environment, within a certain tolerance
Order::OrderResult Actor::MoveToPosition(Order_ActorMoveToPosition & order)
{
	if (_MoveToPosition(order.Target, order.CloseDistanceSq, order.Run) == true)
		return Order::OrderResult::ExecutedAndCompleted;
	else
		return Order::OrderResult::Executed;
}

// Order: Moves the actor to a target object, within a certain tolerance, providing that target is in the same environment
Order::OrderResult Actor::MoveToTarget(Order_ActorMoveToTarget & order)
{
	// Parameter check; make sure the target exists, and that we are both in the same environment
	if (order.Target() == NULL || order.Target()->GetParentEnvironment() != m_parent()) return Order::OrderResult::InvalidOrder;

	// This is effectively just a move-to-position command with the current position of the target
	if (_MoveToPosition(order.Target()->GetPosition(), order.CloseDistanceSq, order.Run) == true)
		return Order::OrderResult::ExecutedAndCompleted;
	else
		return Order::OrderResult::Executed;
}


// Order: Travels to a destination using the environment nav network.  Spawns multiple child orders to get there.
Order::OrderResult Actor::TravelToPosition(Order_ActorTravelToPosition & order)
{
	// Test whether we have traversed all the nodes in the travel path.  If so, return a complete status
	if (order.PathIndex == order.PathLength)
	{
		return Order::OrderResult::ExecutedAndCompleted;
	}

	// Otherwise we want to generate a new direct move order to the next node in the path
	INTVECTOR3 tgt = order.PathNodes[order.PathIndex];

	// Assign the new order, which will generate a new unique ID for the child order
	Order::ID_TYPE id = AssignNewOrder(new Order_ActorMoveToPosition(
		VectorFromIntVector3SwizzleYZ(tgt),															// Swap y/z since path nodes are in element space 
		((order.PathIndex == (order.PathLength - 1)) ? order.CloseDistance : order.FollowDistance),	// Get within the follow distance, unless this is the last node
		order.Run));

	// Make sure the order was correctly assigned
	Order *move = GetOrder(id);
	if (!move) return Order::OrderResult::InvalidOrder;

	// Give the new order a parent pointer to the overall 'travel' order, and a dependency for this overall order
	// on completion of the child (at which point it will generate the next child order)
	move->Parent = order.ID;
	order.Dependency = move->ID;

	// Increment the path index to indicate which node will be next in the path
	++(order.PathIndex);

	// Return a success status, to allow this order to continue executing.  Also signal that we generated new 
	// child orders, so that they will also be picked up in the current order evaluation cycle
	return Order::OrderResult::Executed;
}

// Turns the actor towards the specified position.  Y (vertical) coordinate is ignored.
void Actor::TurnTowardsPosition(const FXMVECTOR position)
{
	float angle = 0.0f;
	
	// Zero out the Y component of each vector (TODO: for now), get the difference vector and then transform it into the actor orientation space
	XMVECTOR transformed = XMVector3TransformCoord(
								XMVectorSubtract(XMVectorSetY(position, 0.0f), XMVectorSetY(m_envposition, 0.0f)),
								m_inverseorientationmatrix);

	// Take the 2D cross product between this transformed target vector (in local space) and the basis vector (i.e. our local forward direction)
	XMVECTOR tgt = XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Z>(transformed);
	XMVECTOR crs = XMVector2Cross(BASIS_VECTOR2, tgt);

	// The cross product will determine the situation we need to consider
	static const XMVECTOR cthreshold = XMVectorReplicate(0.01f);
	if (XMVector2Less(XMVectorAbs(crs), cthreshold))
	{
		// We are either facing the target or at 180 degrees from it.  Use the dot product to determine which is the case
		XMVECTOR dot = XMVector2Dot(BASIS_VECTOR2, tgt);
		if (XMVector2Greater(dot, NULL_VECTOR))
		{
			// If the dot product is negative then we are at 180 degrees, so turn in either direction
			angle = (-m_turnrate * Game::TimeFactor);
		}
		else
		{ 
			// Otherwise we are facing roughly towards the target, so calculate the exact angle at this point
			// If we are near-enough facing the target then simply return here
			XMVECTOR avec = XMVector2AngleBetweenVectors(BASIS_VECTOR2, tgt);
			if (XMVector2Less(XMVectorAbs(avec), Game::C_EPSILON_V)) return;

			// We need to make an adjustment; determine the exact required angle
			angle = XMVectorGetX(avec);
			if (angle < 0.0f)	angle = max(angle, -m_turnrate * Game::TimeFactor);
			else				angle = min(angle, m_turnrate * Game::TimeFactor);
		}
	}
	else if (XMVector2Less(crs, NULL_VECTOR))	angle = (m_turnrate * Game::TimeFactor);	// If the cross product is negative then the target is to the left
	else										angle = (-m_turnrate * Game::TimeFactor);	// If the cross product is positive then the target is to the right
	
	// If the desired turn angle is lower than epsilon then quit here to avoid calculating miniscule turns
	if (fast_abs(angle) < Game::C_EPSILON) return;

	// Turn the actor by this angle
	Turn_NoLimit(angle);
}

// Move forwards along the current heading, either running or walking
void Actor::Move(Direction direction, bool run)
{
	// We only have control of our movement if we are on the ground
	if (!IsOnGround()) return;

	// Create a vector for this movement, dependent on travel direction
	XMVECTOR delta;

	float lmZ = XMVectorGetZ(PhysicsState.LocalMomentum);
	if (direction == Direction::Up && lmZ < Attributes[ActorAttr::A_RunSpeed].Value)										// Forward movement
		delta = XMVectorSetZ(NULL_VECTOR, 
			(run ?	(Attributes[ActorAttr::A_RunSpeed].Value * 4.0f * Game::TimeFactor) :
					(Attributes[ActorAttr::A_WalkSpeed].Value * 3.0f * Game::TimeFactor)));

	else if (direction == Direction::Down && lmZ > -Attributes[ActorAttr::A_RunSpeed].Value)								// Backward movement
		delta = XMVectorSetZ(NULL_VECTOR, Attributes[ActorAttr::A_WalkSpeed].Value * -3.0f * Game::TimeFactor);

	else
	{
		float lmX = XMVectorGetX(PhysicsState.LocalMomentum);
		if (direction == Direction::Left && lmX > -Attributes[ActorAttr::A_StrafeSpeed].Value)								// Left strafing movement
			delta = XMVectorSetX(NULL_VECTOR, 
					(run ?	(Attributes[ActorAttr::A_StrafeSpeed].Value * -4.0f * Game::TimeFactor) :
							(Attributes[ActorAttr::A_WalkSpeed].Value * -3.0f * Game::TimeFactor)));

		else if (direction == Direction::Right && lmX < Attributes[ActorAttr::A_StrafeSpeed].Value)							// Right strafing movement
			delta = XMVectorSetX(NULL_VECTOR, 
					(run ?	(Attributes[ActorAttr::A_StrafeSpeed].Value * 4.0f * Game::TimeFactor) :
							(Attributes[ActorAttr::A_WalkSpeed].Value * 3.0f * Game::TimeFactor)));
		else																												// Unknown movement type
			return;
	}

	// Apply this delta vector as a direct change in local object momentum
	ApplyLocalLinearForceDirect(delta);
}


// Method called when this object collides with another.  Virtual inheritance from iSpaceObject
void Actor::CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact)
{

}

// Event triggered upon destruction of the entity
void Actor::DestroyObject(void)
{
	OutputDebugString("Destruction of Actor\n");
}

// Shutdown method to deallocate resources and remove the actor
void Actor::Shutdown(void)
{
	// Pass control back to the base class
	iEnvironmentObject::Shutdown();
}

// Destructor
Actor::~Actor(void)
{
}
