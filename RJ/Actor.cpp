#include "CompilerSettings.h"
#include "FastMath.h"
#include "SkinnedModel.h"
#include "iSpaceObjectEnvironment.h"
#include "Order.h"
#include "Order_ActorMoveToPosition.h"
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
#ifdef RJ_CPP11_SUPPORT
Actor::Actor(ActorBase *actorbase) : Actor()
#else
Actor::Actor(ActorBase *actorbase)			// Perform full construction if C++11 constructor delegation not supported
#endif
{
	// Store a reference back to the actor base class from which we were created
	m_base = actorbase;

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
	XMStoreFloat4x4(&m_xworldmatrix, XMMatrixIdentity());
}

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void Actor::InitialiseCopiedObject(Actor *source)
{
	// Pass control to all base classes
	iEnvironmentObject::InitialiseCopiedObject((iEnvironmentObject*)source);
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
	m_model.World = m_xworldmatrix;
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
	XMMATRIX mrot = XMMatrixRotationQuaternion(XMLoadFloat4(&XMFLOAT4(m_orientation.x, m_orientation.y, m_orientation.z, m_orientation.w)));
	XMMATRIX mtrans = XMMatrixTranslation(	m_position.x, m_position.y, m_position.z );

	// Set the world x-matrix, which will automatically calculate the legacy and inverse required versions
	SetWorldMatrix(&(mscalerottransadj * mrot * mtrans));
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
	if (!m_parent) return;

	// Determine the jump force for this actor.  Force is mass-independent, i.e. post-division through by inverse mass.  
	// TODO: Change this to pull from actor attributes
	float jumpforce = 4.0f;

	// We can only jump if we are currently on the ground
	if (IsOnGround())
	{
		ApplyLocalLinearForceDirect(D3DXVECTOR3(0.0f, jumpforce, 0.0f));
	}	
}

// Set the world matrix with a D3D matrix.  Also calculate the XMATRIX equivalent
void Actor::SetWorldMatrix(D3DXMATRIX *world)
{ 
	// Store the world matrix and calculate its inverse
	m_worldmatrix = (*world);										// Store the world matrix
	D3DXMatrixInverse(&m_inverseworld, NULL, &m_worldmatrix);		// and calculate the inverse world matrix

	// Also convert to the XMMATRIX version for efficient rendering
	m_xworldmatrix = *((XMFLOAT4X4*)&m_worldmatrix);
}

// Set the world matrix with an XMMATRIX.  Also calculate the D3D equivalent
void Actor::SetWorldMatrix(XMMATRIX *world)
{ 
	// Store the X matrix
	XMStoreFloat4x4(&m_xworldmatrix, *world);

	// Also convert to the D3D version for compatibility
	m_worldmatrix = *( ((D3DXMATRIX*)&m_xworldmatrix) );
	D3DXMatrixInverse(&m_inverseworld, NULL, &m_worldmatrix);		// and calculate the inverse world matrix

	// Invalidate the collision OBB based on this change (since we are bypassing the root iObject SetWorldMatrix method)
	CollisionOBB.Invalidate();
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
void Actor::SimulateObject(bool PermitMovement)
{
	// Degree of simulation is based upon the current simulation state
	switch (m_simulationstate)
	{
		/* Full simulation of the actor */
		case iObject::ObjectSimulationState::FullSimulation:

			ProcessOrderQueue(Game::TimeFactor);							// Process any orders in the actor's queue

			SimulateObjectPhysics();										// Simulate all object physics, generally just gravity 
																			// plus any impact force that has been applied.  This must happen LAST.
			break;

		/* Else (e.g. no simulation) */
		default:

			ProcessOrderQueue(Game::TimeFactor);							// Process any orders in the actor's queue
			break;
	}
}

// Method to process the specified order.  Called when processing the full queue.  Returns true if order is now completed & can be removed
Order::OrderResult Actor::ProcessOrder(Order *order)
{
	Order::OrderType type;

	// Determine the type of order being processed
	if (!order) return Order::OrderResult::InvalidOrder;
	type = order->GetType();

	// Take different action depending on the order type
	switch (type)
	{
		// Move to position.  Specifies position and the distance to which we must move within
		case Order::OrderType::ActorMoveToPosition:
			return MoveToPosition(	order->Parameters.Float3_1,		// Target position
									order->Parameters.Float3_2.x,	// Distance to move within
									order->Parameters.Flag_1);		// Indicates whether the actor should run

		// Move to target.  Specifies the target and the distance to which we must move within
		case Order::OrderType::ActorMoveToTarget:
			return MoveToTarget(	(iEnvironmentObject*)order->Parameters.Target_1,		// The target we should move towards
									order->Parameters.Float3_1.x,							// Distance to move within
									order->Parameters.Flag_1);								// Indicates whether the actor should run

		// Travel to position.  Navigates to a destination using the environment nav network
		case Order::OrderType::ActorTravelToPosition:
			return TravelToPosition((Order_ActorTravelToPosition*)order);


			// If this is not an order we can execute, return an invalid order result to have it removed from the queue
		default:
			return Order::OrderResult::InvalidOrder;
	}

}


// Order: Moves the actor to a target position in the same environment, within a certain tolerance
Order::OrderResult Actor::MoveToPosition(D3DXVECTOR3 position, float getwithin, bool run)
{
	// Completion check:  See whether we have reached the target position.  Calculated (squared) distance to target and threshold.
	// Only consider distance in x/z dimensions - y (vertical) dimension is ignored.
	D3DXVECTOR2 diffvec = D3DXVECTOR2((m_envposition.x - position.x), (m_envposition.z - position.z));
	float distsq = ((diffvec.x * diffvec.x) + (diffvec.y * diffvec.y));
	float closedistsq = (getwithin * getwithin);

	// If we are within the close distance then we can stop moving and complete the order
	if (distsq < closedistsq)
	{
		/* Transition to a stopping/idle animation of some kind */
		return Order::OrderResult::ExecutedAndCompleted;
	}	

	// We need to turn towards to the target position
	TurnTowardsPosition(position);

	// Move forwards in this direction
	// TODO: switch to walking when close, if we are currently running?
	Move(Direction::Up, run);

	// We have executed the order this cycle and it is still in progress
	return Order::OrderResult::Executed;
}

// Order: Moves the actor to a target object, within a certain tolerance, providing that target is in the same environment
Order::OrderResult Actor::MoveToTarget(iEnvironmentObject *target, float getwithin, bool run)
{
	// Call the overloaded function, assuming we are in the same environment as the target
	if (target && (target->GetParentEnvironment() == m_parent))
		return MoveToPosition(target->GetEnvironmentPosition(), getwithin, run);
	else
		return Order::OrderResult::InvalidOrder;
}


// Order: Travels to a destination using the environment nav network.  Spawns multiple child orders to get there.
Order::OrderResult Actor::TravelToPosition(Order_ActorTravelToPosition *order)
{
	// Parameter check
	if (!order) return Order::OrderResult::InvalidOrder;

	// Test whether we have traversed all the nodes in the travel path.  If so, return a complete status
	if (order->PathIndex == order->PathLength)
	{
		return Order::OrderResult::ExecutedAndCompleted;
	}

	// Otherwise we want to generate a new direct move order to the next node in the path
	INTVECTOR3 tgt = order->PathNodes[order->PathIndex];
	Order_ActorMoveToPosition *move = new Order_ActorMoveToPosition(	
												D3DXVECTOR3((float)tgt.x, (float)tgt.z, (float)tgt.y),	// Swap y/z since path nodes are in element space 
												order->Parameters.Float3_2.x, order->Parameters.Flag_1 
											);

	// Assign the new order, which will generate a new unique ID for the child order
	this->AssignNewOrder(move);

	// Give the new order a parent pointer to the overall 'travel' order, and a dependency for this overall order
	// on completion of the child (at which point it will generate the next child order)
	move->Parent = order->ID;
	order->Dependency = move->ID;

	// Increment the path index to indicate which node will be next in the path
	++order->PathIndex;

	// Return a success status, to allow this order to continue executing.  Also signal that we generated new 
	// child orders, so that they will also be picked up in the current order evaluation cycle
	return Order::OrderResult::Executed;
}

// Turns the actor towards the specified position.  Y (vertical) coordinate is ignored.
void Actor::TurnTowardsPosition(const D3DXVECTOR3 &position)
{
	float angle = 0.0f;
	const D3DXVECTOR2 BASIS_2D = D3DXVECTOR2(0.0f, 1.0f);

	// Transform the target position vector into the actor orientation space
	D3DXVECTOR3 transformed = D3DXVECTOR3(position.x - m_envposition.x, 0.0f, position.z - m_envposition.z);
	D3DXVec3TransformCoord(&transformed, &transformed, &m_inverseorientationmatrix);

	// Subtract our position vector from this target position to get the delta from the origin, then take the cross product
	// against the basis vector (representing our heading in this relative position space)
	D3DXVECTOR2 tgt = D3DXVECTOR2(transformed.x, transformed.z);
	float crs = CrossProduct2D(BASIS_2D, tgt);

	// The cross product will determine the situation we need to consider
	if (fast_abs(crs) < 0.01f)
	{
		// We are either facing the target or at 180 degrees from it.  Use the dot product to determine which is the case
		float dot = DotProduct2D(BASIS_2D, tgt);
		if (dot > 0.0f)
		{
			// If the dot product is negative then we are at 180 degrees, so turn in either direction
			angle = (-m_turnrate * Game::TimeFactor);
		}
		else
		{
			// Otherwise we are facing roughly towards the target, so calculate the exact angle at this point
			angle = Angle2D(BASIS_2D, tgt);
			if (fast_abs(angle) < Game::C_EPSILON) return;

			if (angle < 0.0f)	angle = max(angle, -m_turnrate * Game::TimeFactor);
			else				angle = min(angle, m_turnrate * Game::TimeFactor);
		}
	}
	else if (crs < 0.0f)	angle = (m_turnrate * Game::TimeFactor);				// If the cross product is negative then the target is to the left
	else					angle = (-m_turnrate * Game::TimeFactor);				// If the cross product is positive then the target is to the right
	
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
	D3DXVECTOR3 delta;

	if (direction == Direction::Up && PhysicsState.LocalMomentum.z < Attributes[ActorAttr::A_RunSpeed].Value)
			delta = D3DXVECTOR3(0.0f, 0.0f, (run ?	(Attributes[ActorAttr::A_RunSpeed].Value * 4.0f * Game::TimeFactor) :
													(Attributes[ActorAttr::A_WalkSpeed].Value * 3.0f * Game::TimeFactor)));	

	else if (direction == Direction::Down && PhysicsState.LocalMomentum.z > -Attributes[ActorAttr::A_RunSpeed].Value)
			delta = D3DXVECTOR3(0.0f, 0.0f, -(Attributes[ActorAttr::A_WalkSpeed].Value * 3.0f * Game::TimeFactor));			

	else if (direction == Direction::Left && PhysicsState.LocalMomentum.x > -Attributes[ActorAttr::A_StrafeSpeed].Value)
			delta = D3DXVECTOR3((run ?	-(Attributes[ActorAttr::A_StrafeSpeed].Value * 4.0f * Game::TimeFactor) :
										-(Attributes[ActorAttr::A_WalkSpeed].Value * 3.0f * Game::TimeFactor)), 0.0f, 0.0f);

	else if (direction == Direction::Right && PhysicsState.LocalMomentum.x < Attributes[ActorAttr::A_StrafeSpeed].Value)
			delta = D3DXVECTOR3((run ?	(Attributes[ActorAttr::A_StrafeSpeed].Value * 4.0f * Game::TimeFactor) :
										(Attributes[ActorAttr::A_WalkSpeed].Value * 3.0f * Game::TimeFactor)), 0.0f, 0.0f);	
	else
		return;
	

	// Transform this vector into the actor orientation space
	//D3DXVec3TransformCoord(&delta, &delta, &m_orientationmatrix);

	// Apply this change in movement to the actor
	//AddDeltaPosition(delta);

	// Apply this delta vector as a direct change in local object momentum
	ApplyLocalLinearForceDirect(delta);
}


// Method called when this object collides with another.  Virtual inheritance from iSpaceObject
void Actor::CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact)
{

}


// Shutdown method to deallocate resources and remove the actor
void Actor::Shutdown(void)
{
	
}

// Destructor
Actor::~Actor(void)
{
}
