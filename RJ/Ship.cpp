#include "iContainsHardpoints.h"
#include "Hardpoint.h"
#include "HpEngine.h"
#include "FastMath.h"
#include "Octree.h"
#include "RJDebug.h"
#include "Ray.h"
#include "iSpaceObject.h"
#include "GamePhysicsEngine.h"
#include "SimulationObjectManager.h"
#include "Order.h"
#include "Order_MoveToPosition.h"
#include "Order_MoveToTarget.h"
#include "Order_MoveAwayFromTarget.h"
#include "Order_AttackBasic.h"

#include "Ship.h"

Ship::Ship(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::ShipObject);

	m_shipclass = Ships::Class::Unknown;

	m_standardobject = false;
	m_defaultloadout = "";

	BaseMass = 1.0f;
	VelocityLimit.SetAllValues(1.0f);
	AngularVelocityLimit.SetAllValues(1.0f);
	TurnRate.SetAllValues(0.01f);
	TurnAngle.SetAllValues(0.01f);
	Bank = m_new_bank = NULL_VECTOR;
	BankRate.SetAllValues(0.0f);
	SetBankExtent(NULL_VECTOR3);
	EngineAngularAcceleration.SetAllValues(1000.0f);
	m_isbraking = false;
	BrakeFactor.SetAllValues(1.0f);
	m_flightcomputerinterval = Game::C_DEFAULT_FLIGHT_COMPUTER_EVAL_INTERVAL;
	m_timesincelastflightcomputereval = 0.0f;
	m_timesincelasttargetanalysis = 0U;
	m_shipenginecontrol = true;
	m_targetspeed = m_targetspeedsq = m_targetspeedsqthreshold = 0.0f;
	m_isturning = false;
	m_targetpitch = m_targetyaw = 0.0f;
	m_targetangularvelocity = NULL_VECTOR;
	m_engineangularvelocity = m_engineangularmomentum = NULL_VECTOR;
	m_unadjusted_orient = ID_QUATERNION; 
	m_inv_unadjusted_orient = XMQuaternionInverse(m_unadjusted_orient);
	m_cached_contact_count = m_cached_enemy_contact_count = 0;
	m_avoid_target = NULL;

	// Link the hardpoints collection to this parent object
	m_hardpoints.SetParent<Ship>(this);

	// Notify the turret controller of its parent object
	TurretController.SetParent(this);

	m_thrustchange_flag = true;		// Force an initial refresh
	m_masschange_flag = true;			// Force an initial refresh

	m_turnmodifier_peaceful = Game::C_AI_DEFAULT_TURN_MODIFIER_PEACEFUL;
	m_turnmodifier_combat = Game::C_AI_DEFAULT_TURN_MODIFIER_COMBAT;
	m_turnmodifier = m_turnmodifier_peaceful;

	MinBounds = XMVectorReplicate(-0.5f);
	MaxBounds = XMVectorReplicate(0.5f);
}



// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void Ship::InitialiseCopiedObject(Ship *source)
{
	// Pass control to all base classes
	iSpaceObject::InitialiseCopiedObject((iSpaceObject*)source);


	/* Begin Ship-specific initialisation logic here */

	// Copy the ship hardpoints and link them to this ship
	m_hardpoints.Clone(source->GetHardpoints());
	m_hardpoints.SetParent<Ship>(this);

	// Initialise the turret controller, removing all turrets, and link them to this ship
	TurretController.ForceClearContents();		// To avoid turret parent pointers being reset by a RemoveAll...() call
	TurretController.SetParent(this);

	// Clear any instance-specific fields
	m_cached_contacts.clear();
	m_cached_enemy_contacts.clear();
	m_cached_contact_count = m_cached_enemy_contact_count = 0;

	// Update any other fields that should not be replicated through the standard copy constructor
	m_timesincelastflightcomputereval = 0.0f;
	m_timesincelasttargetanalysis = 0U;
	CancelAllOrders();
	FullStop();
}


// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
void Ship::SimulationStateChanged(iObject::ObjectSimulationState prevstate, iObject::ObjectSimulationState newstate)
{
	// Call the superclass event before proceeding
	iSpaceObject::SimulationStateChanged(prevstate, newstate);

	// If we were not being simulated, and we now are, then we may need to take Ship-specific actions here
	// TODO: this will not always be true in future when we have more granular simulation states 
	if (prevstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// (Take any Ship-specific actions in this method)
	}

	// Conversely, if we are no longer going to be simulated, we can take actions to remove unneeded functionality
	if (newstate == iObject::ObjectSimulationState::NoSimulation)
	{
		// (Take any Ship-specific actions in this method)
	}
}


// Terminates the ship object and deallocates storage.  Also passes control back to the iSpaceObject interface.
void Ship::Shutdown(void)
{
	// Pass control back to the base class
	iSpaceObject::Shutdown();
}


// Sets the target thrust of all engines on the ship
void Ship::SetTargetThrustOfAllEngines(float target)
{
	// Get a reference to the set of engines on this ship
	Hardpoints::HardpointCollection & engines = GetHardpoints().GetHardpointsOfType(Equip::Class::Engine);

	// Set the target thrust of each engine in turn
	Hardpoints::HardpointCollection::iterator it_end = engines.end();
	for (Hardpoints::HardpointCollection::iterator it = engines.begin(); it != it_end; ++it)
		((HpEngine*)(*it))->SetTargetThrust(target);
}

// Sets the target thrust of all engines on the ship to a percentage of their maximum thrust
void Ship::SetTargetThrustPercentageOfAllEngines(float target)
{
	// Get a reference to the set of engines on this ship
	Hardpoints::HardpointCollection & engines = GetHardpoints().GetHardpointsOfType(Equip::Class::Engine);

	// Set the target thrust of each engine in turn
	Hardpoints::HardpointCollection::iterator it_end = engines.end();
	for (Hardpoints::HardpointCollection::iterator it = engines.begin(); it != it_end; ++it)
		((HpEngine*)(*it))->SetTargetThrustPercentage(target);
}

// Increments the target thrust of all engines on the ship
void Ship::IncrementTargetThrustOfAllEngines(void)
{
	// Get a reference to the set of engines on this ship
	Hardpoints::HardpointCollection & engines = GetHardpoints().GetHardpointsOfType(Equip::Class::Engine);

	// Increment the target thrust of each engine in turn
	Hardpoints::HardpointCollection::iterator it_end = engines.end();
	for (Hardpoints::HardpointCollection::iterator it = engines.begin(); it != it_end; ++it)
		((HpEngine*)(*it))->IncrementTargetThrust();
}

// Decrements the target thrust of all engines on the ship
void Ship::DecrementTargetThrustOfAllEngines(void)
{
	// Get a reference to the set of engines on this ship
	Hardpoints::HardpointCollection & engines = GetHardpoints().GetHardpointsOfType(Equip::Class::Engine);

	// Decrement the target thrust of each engine in turn
	Hardpoints::HardpointCollection::iterator it_end = engines.end();
	for (Hardpoints::HardpointCollection::iterator it = engines.begin(); it != it_end; ++it)
		((HpEngine*)(*it))->DecrementTargetThrust();
}

// Attempts to bring the ship to a full stop
void Ship::FullStop(void)
{
	// Set target linear thrust of all engines to zero
	SetTargetSpeed(0.0f);

	// Cease all current maneuvers
	m_targetpitch = m_targetyaw = 0.0f;
	m_targetangularvelocity = NULL_VECTOR;
}

// Implementation of the virtual iContainsHardpoints event method.  Invoked when the hardpoint 
// configuration of the object is changed.  Provides a reference to the hardpoint that was changed, or NULL
// if a more general update based on all hardpoints is required (e.g. after first-time initialisation)
void Ship::HardpointChanged(Hardpoint *hp)
{
	// Get the type of hardpoint being considered, or assign 'unknown' if NULL is provided and we want to make all potential updates
	Equip::Class hptype = (hp ? hp->GetType() : Equip::Class::Unknown);

	/*** HPEngine ***/
	if (hptype == Equip::Class::Engine || hptype == Equip::Class::Unknown)
	{
		// If we just altered an engine hardpoint then force an update of the ship velocity/momentum calculations
		SetThrustVectorChangeFlag();
	}

	// Updating hardpoints may have affected the ship mass
	SetShipMassChangeFlag();
}

// Runs the ship flight computer, evaluating current state and any active orders
void Ship::RunShipFlightComputer(void)
{
	// Reset any ship attributes that should be re-initialised on each ship computer cycle
	//m_targetpitch = m_targetyaw = 0.0f;
	
	// Evaluate any orders in the queue
	ProcessOrderQueue(m_timesincelastflightcomputereval);

	// Identify any nearby collision threats
	IdentifyCollisionThreats();

	// Determine thrust levels for each engine on the ship based on current target parameters
	DetermineEngineThrustLevels();

	// Reset the ship computer execution process ready for another cycle
	m_timesincelastflightcomputereval = 0.0f;
}

// Sets the target ship speed as a percentage of total velocity limit
void Ship::SetTargetSpeedPercentage(float percentage)
{
	SetTargetSpeed(percentage * VelocityLimit.Value);
}

// Sets the target ship speed, used by the ship computer to control engine thrust & braking
void Ship::SetTargetSpeed(float target)
{
	// Store the new target speed
	m_targetspeed = target;

	// Precalculated components relating to target speed for more efficient in-flight calculation by the flight computer
	m_targetspeedsq = (target * target);
	m_targetspeedsqthreshold = (m_targetspeedsq * Game::C_ENGINE_THRUST_DECREASE_THRESHOLD);
}

// Determines the appropriate thrust/brake level for each engine to meet target ship speed
void Ship::DetermineEngineThrustLevels(void)
{
	// Only control thrust levels if the flight computer is enabled
	if (!m_shipenginecontrol) return;

	// Determine the current squared momentum (in local space)
	float msq = XMVectorGetX(XMVector3LengthSq(PhysicsState.LocalMomentum));

	// If we are not moving, and we don't want to be moving, then quit here without performing any further calculations
	if (msq < Game::C_EPSILON && m_targetspeed < Game::C_EPSILON) return;

	// Take action dependending on our current momentum vs the target speed
	else if (msq > m_targetspeedsq)
	{
		// If we are current above the target speed then engage brakes to slow our momentum
		SetTargetThrustOfAllEngines(0.0f);
		ApplyBrakes();
	}
	else if (msq > m_targetspeedsqthreshold)
	{
		// If we are within a small threshold of target speed then start to reduce engine thrust
		SetTargetThrustPercentageOfAllEngines(0.25f + (0.75f * ((msq-m_targetspeedsqthreshold)/(m_targetspeedsq-m_targetspeedsqthreshold))));
		RemoveBrakes();
	}
	else
	{
		// Otherwise, we will set target thrust of all engines to 100%
		SetTargetThrustPercentageOfAllEngines(1.0f);
		RemoveBrakes();
	}
}

// Run all engines to their target thrust level, if applicable and if not already at that level
void Ship::RunAllEnginesToTargetThrust(void)
{
	// We will consider each engine in turn
	Hardpoints::HardpointCollection & engines = GetHardpoints().GetHardpointsOfType(Equip::Class::Engine);

	// Loop through each in turn and run to its target thrust if required
	Hardpoints::HardpointCollection::iterator it_end = engines.end();
	for (Hardpoints::HardpointCollection::iterator it = engines.begin(); it != it_end; ++it)
		((HpEngine*)(*it))->RunToTargetThrust();
	
	// Determine the target angular velocity for our engines based on desired pitch & yaw
	m_targetangularvelocity = XMVectorSet(m_targetpitch * TurnAngle.Value, m_targetyaw * TurnAngle.Value, 0.0f, 0.0f);
}

void Ship::SimulateAllShipEngines(void)
{
	// Optimisation: check whether the thrust generated by any of our engines has changed (F), or whether our ship
	// mass has changed (m), since the last physics calculation of acceleration (a = F/m).  If not, we can retain 
	// the cached acceleration value and use it to save time
	if (ThrustVectorsChanged() || ShipMassChanged())
	{
		// Reset the variables we want to calculate
		this->PhysicsState.Acceleration = NULL_VECTOR3;
		
		// Get a pointer to the collection of engines on this ship
		Hardpoints::HardpointCollection & engines = GetHardpoints().GetHardpointsOfType(Equip::Class::Engine);

		// Loop through each engine in turn and add its contribution to the velocity vector
		Hardpoints::HardpointCollection::iterator it_end = engines.end();
		for (Hardpoints::HardpointCollection::iterator it = engines.begin(); it != it_end; ++it)
		{
			HpEngine* e = (HpEngine*)(*it);

			// Only perform thrust calculations if there is anything mathematically material to calculate
			if (abs(e->GetThrust()) > Game::C_EPSILON)
			{
				// Calculate acceleration (a = F/m) by first just accumulating F values.  Divide through by m afterwards
				PhysicsState.Acceleration = XMVectorAdd(
					PhysicsState.Acceleration,
					XMVectorScale(e->GetThrustVector(), 1000.0f)); // TODO: Constant-adjusted for physics
			}
		}		

		// Divide accumulated velocity vectors (F) through by mass (m) to get acceleration vector (a=F/m)
		PhysicsState.Acceleration = XMVectorScale(PhysicsState.Acceleration, GetInverseMass());

		// We have now recalculated the cached acceleration vector so reset the flags and time interval counter
		ResetThrustVectorChangeFlag();
		ResetShipMassChangeFlag();
	}

	// Now also calculate the angular velocity to be generated by the ship/engines/thrusters to meet the desired heading
	// Based on the following for each component:
	//	if (m_targetangularvelocity.x > PhysicsState.AngularVelocity.x)
	//			m_engineangularvelocity.x = min(TurnRate.Value, m_targetangularvelocity.x - PhysicsState.AngularVelocity.x) * Game::TimeFactor;
	//	else	m_engineangularvelocity.x = max(-TurnRate.Value, m_targetangularvelocity.x - PhysicsState.AngularVelocity.x) * Game::TimeFactor;
	XMVECTOR vel_diff = XMVectorSubtract(m_targetangularvelocity, PhysicsState.AngularVelocity);
	XMVECTOR vel_p = XMVectorMin(m_turnrate_v, vel_diff);
	XMVECTOR vel_n = XMVectorMax(m_turnrate_nv, vel_diff);
	m_engineangularvelocity = 
		XMVectorMultiply(
			XMVectorSelect(vel_p, vel_n,														// Will return 0 (and select from vel_p) if >, or
				XMVectorLessOrEqual(m_targetangularvelocity, PhysicsState.AngularVelocity)),	// will return 0xff (and select from vel_n) if <=
		Game::TimeFactorV);

	// Multiply the target angular momentum by the ship's angular acceleration, derived from its engines
	//m_engineangularvelocity *= (this->EngineAngularAcceleration.Value * 1000.0f);		// TODO: Constant-adjusted for physics

	// Working backwards, derive the angular momentum generated by the engines on the basis that they can generate this
	// degree of angular velocity for the ship.  TODO: perhaps change in future to derive momentum, and from that the velocity,
	// and then contrain to be within the allowable velocity bounds defined by TurnRate
	m_engineangularmomentum = XMVector3TransformCoord(m_engineangularvelocity, PhysicsState.InertiaTensor);
}

void Ship::SimulateObjectPhysics(void)
{
	// Update momentum vector using ship acceleration, transformed to world space, scaled by the time factor that has passed
	PhysicsState.WorldAcceleration = XMVector3TransformCoord(PhysicsState.Acceleration, m_orientationmatrix);
	PhysicsState.WorldMomentum = XMVectorAdd(PhysicsState.WorldMomentum, XMVectorMultiply(PhysicsState.WorldAcceleration, Game::TimeFactorV));

	// Limit this ship to its maximum tolerable velocity.  Or, if in hardcore mode, allow it but with damage
	if (Game::C_NO_MOMENTUM_LIMIT)
		;								// TODO: Apply damage effects proportionate to how far over max supportable velocity we are
	else
	{
		if (XMVector3AnyTrue(XMVectorGreater(PhysicsState.WorldMomentum, m_vlimit_v)))
			PhysicsState.WorldMomentum = ScaleVector3WithinMagnitudeLimit(PhysicsState.WorldMomentum, VelocityLimit.Value);
	}

	// Apply a drag scalar to the momentum vector to prevent perpetual movement.  Again, weighted by
	// fraction of a second that has passed.  C_MOVEMENT_DRAG_FACTOR is % momentum decrease per second.
	if (!IsZeroVector3(this->PhysicsState.WorldMomentum))
	{
		// this->PhysicsState.WorldMomentum -= (this->PhysicsState.WorldMomentum * Game::C_MOVEMENT_DRAG_FACTOR * Game::TimeFactor);
		PhysicsState.WorldMomentum = XMVectorSubtract(
			PhysicsState.WorldMomentum,
			XMVectorScale(PhysicsState.WorldMomentum, (Game::C_MOVEMENT_DRAG_FACTOR * Game::TimeFactor)));
	}

	// Also apply a drag factor if the ship is currently braking
	if (m_isbraking) 
	{
		XMVECTOR velocityloss = XMVectorMultiply(m_vlimit_v, Game::TimeFactorV);
		XMVECTOR vloss_p = XMVectorSubtract(PhysicsState.WorldMomentum, velocityloss);
		XMVECTOR vloss_n = XMVectorAdd(PhysicsState.WorldMomentum, velocityloss);

		// Based on the following for each component:
		//		float velocityloss = (this->VelocityLimit.Value * Game::TimeFactor);
		//		if (this->PhysicsState.WorldMomentum.x > 0)	this->PhysicsState.WorldMomentum.x = max(0.0f, this->PhysicsState.WorldMomentum.x - velocityloss);
		//		else										this->PhysicsState.WorldMomentum.x = min(0.0f, this->PhysicsState.WorldMomentum.x + velocityloss);
		PhysicsState.WorldMomentum =
			XMVectorSelect(
				XMVectorMax(NULL_VECTOR3, vloss_p),
				XMVectorMin(NULL_VECTOR3, vloss_n),
				XMVectorLessOrEqual(PhysicsState.WorldMomentum, NULL_VECTOR3));
	}

	// Apply the incremental angular velocity generated by ship engines this cycle
	this->ApplyAngularMomentum(m_engineangularmomentum);

	// Limit angular velocity to the ship's max acceptable velocity
	if (XMVector3AnyTrue(XMVectorGreater(PhysicsState.AngularVelocity, m_avlimit_v)))
		PhysicsState.AngularVelocity = ScaleVector3WithinMagnitudeLimit(PhysicsState.AngularVelocity, AngularVelocityLimit.Value);

	// Apply a damping effect on angular velocity
	XMVECTOR angularvelocityloss = XMVectorReplicate(Game::C_ANGULAR_VELOCITY_DAMPING_FACTOR * Game::TimeFactor);
	XMVECTOR aloss_p = XMVectorSubtract(PhysicsState.AngularVelocity, angularvelocityloss);
	XMVECTOR aloss_n = XMVectorAdd(PhysicsState.AngularVelocity, angularvelocityloss);

	// Based on the following for each component:
	//		float angularvelocityloss = (Game::C_ANGULAR_VELOCITY_DAMPING_FACTOR * Game::TimeFactor);
	//		if (PhysicsState.AngularVelocity.x > 0)	PhysicsState.AngularVelocity.x = max(0.0f, PhysicsState.AngularVelocity.x - angularvelocityloss);
	//		else									PhysicsState.AngularVelocity.x = min(0.0f, PhysicsState.AngularVelocity.x + angularvelocityloss);
	PhysicsState.AngularVelocity =
		XMVectorSelect(
			XMVectorMax(NULL_VECTOR3, aloss_p),
			XMVectorMin(NULL_VECTOR3, aloss_n),
			XMVectorLessOrEqual(PhysicsState.AngularVelocity, NULL_VECTOR3));

	// We have made multiple updates to the world momentum, so recalculate the resulting local momentum now
	RecalculateLocalMomentum();	
}

// Simulates the ship movement and recalculates its new position
void Ship::SimulateObject(void)
{
	/* Generalised process:
			1. Determine any changes to ship behaviour, for example adjustments to orders or objectives by the flight computer
			2. Calculate the forces acting on this ship, e.g. the linear momentum generated by the ship engines
			3. Perform a physical simulation of this object, e.g. to calculate new momentum/heading/angular momentum values
			4. Update the ship based on physics, e.g. updating the position based on momentum
	*/

	// Increment the counters that determine when we next perform AI actions
	m_timesincelastflightcomputereval += Game::TimeFactor;
	m_timesincelasttargetanalysis += Game::ClockMs;

	// Test whether enough time has passed for the flight computer to be evaluated again
	if (m_timesincelastflightcomputereval > m_flightcomputerinterval)
	{
		// Analyse all nearby enemy contacts.  This will use a cached collection that does not need to be updated every frame
		AnalyseNearbyContacts();

		// Reset any persistence flags that maintain an action between one execution of the flight computer and the next
		m_isturning = false;

		// Execute the flight computer and reset the counter
		RunShipFlightComputer();
		m_timesincelastflightcomputereval = 0.0f;
	}

	// Handle all ship movement, if permitted by the central simulator (if not, it means we will be moved
	// some other way, e.g. if we are attached to some other object which will calculate our position)
	if (CanSimulateMovement())
	{
		// Performs collision avoidance if we have any nearby collision threat.  This will override any previous 
		// maneuvering orders if required to avoid the collision
		if (IsAvoidingCollision()) PerformCollisionAvoidance();

		// Adjust engine outputs towards target thrust levels
		RunAllEnginesToTargetThrust();

		// Simulate the effect of all ship engines on ship momentum
		SimulateAllShipEngines();

		// Perform a phsyical simulation of the object, based on all the forces currently acting on it
		SimulateObjectPhysics();

		// Recalculate the position of this ship based on its new momentum vector	
		DetermineNewPosition();

		// Derive the new world matrix for this object
		//DeriveNewWorldMatrix();

		// Set the update flag to indicate that this object has now been simulated
		SetPositionUpdated(true);
	}
	
	// Simulate all ship turrets if applicable (TODO: need to pass contacts array)
	if (HasNearbyEnemyContacts() && TurretController.IsActive()) TurretController.Update(m_cached_enemy_contacts);

	// Update position of the ship in the spatial partitioning tree
	if (m_treenode) m_treenode->ItemMoved(this, m_position);
}

// Update the collections of nearby contacts
void Ship::AnalyseNearbyContacts(void)
{
	iSpaceObject *obj;

	// Test whether we are within the interval for re-analysing nearby contacts.  If so, perform a new search
	// for nearby enemy contacts and store them in the cached data collection
	if (m_timesincelasttargetanalysis > Game::C_DEFAULT_SHIP_CONTACT_ANALYSIS_FREQ)
	{
		// Locate all objects in the vicinity of this object, and maintain as the cache of nearby objects
		Game::ObjectManager.GetAllObjectsWithinDistance(this, Game::C_DEFAULT_SHIP_CONTACT_ANALYSIS_RANGE, m_cached_contacts,
			SimulationObjectManager::ObjectSearchOptions::NoSearchOptions);
		m_cached_contact_count = m_cached_contacts.size();
	
		// Parse out any enemy contacts into the cached enemy contact vector.  Also ensure we have no NULLs in 
		// the collection so that we can parse it without NULL checks each frame/ship computer cycle
		// TODO: in future, need something more robust in case e.g. one of the ships is destroyed or leaves
		m_cached_enemy_contacts.clear();
		for (std::vector<iSpaceObject*>::size_type i = 0; i < m_cached_contact_count; ++i)
		{
			// Remove this contact if it is invalid, or if it is us
			obj = m_cached_contacts[i]; 
			if (!obj || obj == this) { RemoveFromVectorAtIndex<iSpaceObject*>(m_cached_contacts, i); --i; --m_cached_contact_count; continue; }

			// Add to the list of enemy contacts if we are hostile towards the object
			if (GetDispositionTowardsObject(obj) == Faction::FactionDisposition::Hostile)
			{
				m_cached_enemy_contacts.push_back(obj);
			}
		}
		m_cached_enemy_contact_count = m_cached_enemy_contacts.size();

		// Reset the counter that indicates when we next search for contacts
		m_timesincelasttargetanalysis = 0U;
	}
}

// Analyses all nearby contacts to identify whether the ship is at risk of collision.  Stores the nearest collision threat 
// as the current avoidance target if applicable
void Ship::IdentifyCollisionThreats(void)
{
	// We will perform collision avoidance on all nearby contacts
	iSpaceObject *obj;
	XMVECTOR objpos, diff, obj_distsq;
	XMVECTOR nearest_collision = LARGE_VECTOR_P;
	bool intersects;

	// We do not have any collision threat by default, unless we find one here
	m_avoid_target = NULL;

	// Determine the collision test vector based on our current momentum
	XMVECTOR avoidvector = DetermineCollisionAvoidanceCheckVector();
	XMVECTOR avoid_rangesq = XMVector3LengthSq(avoidvector);

	// Iterate through the contact collection
	std::vector<iSpaceObject*>::iterator it_end = m_cached_contacts.end();
	for (std::vector<iSpaceObject*>::iterator it = m_cached_contacts.begin(); it != it_end; ++it)
	{
		obj = (*it);
		objpos = obj->GetPosition();
		diff = XMVectorSubtract(objpos, m_position);
		obj_distsq = XMVector3LengthSq(diff);

		// Perform collision avoidance; first, check whether the object is in range
		if (XMVector2Less(obj_distsq, avoid_rangesq))
		{
			// The object is in range, so test whether our momentum vector would intersect it.  We test the
			// vector against (obj.radius + this.radius) so both object sizes are accounted for.  Uses bounding
			// sphere where possible, or OBB for larger objects where the sphere is a poorer approximation
			intersects = (obj->MostAppropriateBoundingVolumeType() == Game::BoundingVolumeType::OrientedBoundingBox ?
				Game::PhysicsEngine.TestVolumetricRayVsOBBIntersection(	Ray(m_position, avoidvector),
																		XMVectorReplicate(m_collisionsphereradius), obj->CollisionOBB.Data()) :
				Game::PhysicsEngine.TestRaySphereIntersection(	m_position, avoidvector, objpos,
																XMVectorReplicate(m_collisionsphereradius + obj->GetCollisionSphereRadius()))
			);

			// Take no further action on this object if there will not be an intersection
			if (!intersects) continue;

			// This is a potential intersection; record it if this would be the nearest one
			if (XMVector2Less(obj_distsq, nearest_collision))
			{
				// The ship computer will attempt to maneuver to avoid collision with the object in "m_avoid_target"
				m_avoid_target = obj;
				nearest_collision = obj_distsq;
			}
		}
	}
}

void Ship::PerformCollisionAvoidance(void)
{
	// Execute target avoidance if appropriate; this will override existing turn commands where required to avoid a collision (since 
	// this call to TurnShip() is being made last
	XMFLOAT2 pitch_yaw;
	DetermineCollisionAvoidanceResponse(pitch_yaw);
	TurnShipIfRequired(pitch_yaw.y, pitch_yaw.x, true);
}

// Determines the maneuver required to avoid the current avoidance target.  Does not perform a null test on the 
// avoidance target for efficiency; this is a protected method that can assume the avoidance target is non-null and valid
void Ship::DetermineCollisionAvoidanceResponse(XMFLOAT2 & outPitchYaw)
{
	static const AXMVECTOR parallel_adj = XMVectorSetX(NULL_VECTOR, 0.1f);

	// Useful info: http://mathinsight.org/dot_product
	// Get the vector from our current position to the avoidance target
	XMVECTOR tgt = XMVectorSubtract(m_avoid_target->GetPosition(), m_position);

	// Normalise our world momentum vector to save additional calculations later
	XMVECTOR wm_n = XMVector3NormalizeEst(PhysicsState.WorldMomentum);

	// Take the projection of this target vector (a) onto our world momentum vector (b) , via
	// projection = ((a . b) / |b|).  Since we have normalised b, |b| == 1 and we can just use 
	// projection = (a . b).  This gives a point perpendicular to the momentum vector 
	// that is in line with the avoidance target
	XMVECTOR proj = XMVector3Dot(tgt, wm_n);

	// If the dot product projection is negative, the angle between WM and TGT vectors is obtuse.  This means the 
	// target is behind us and we can safely ignore it
	if (XMVector2Less(proj, NULL_VECTOR)) { outPitchYaw = NULL_FLOAT2; return; }

	// If the cross product of WM and TGT is ~zero, the two vectors are ~parallel.  We need to add a small offset
	// to the target vector in this case.  We will use a minor offset along the ship local right basis vector
	if (IsZeroVector3(XMVector3Cross(wm_n, tgt)))
		tgt = XMVectorAdd(tgt, XMVector3TransformCoord(parallel_adj, m_orientationmatrix));

	// The point on the momentum vector is now (normalise(wm) * proj), however we have already
	// normalised so can just take (wm_n * proj)
	proj = XMVectorMultiply(wm_n, proj);

	// Our collision response vector is therefore ((norm(proj - tgt) * distance), where we will use 
	// ((radius0 + radius1) * safety_multiplier) as the distance
	XMVECTOR response = XMVectorScale(XMVector3NormalizeEst(XMVectorSubtract(proj, tgt)),
		((m_collisionsphereradius + m_avoid_target->GetCollisionSphereRadius()) * Game::C_COLLISION_AVOIDANCE_RESPONSE_SAFETY_MULTIPLIER));

	// Finally, determine the pitch and yaw required to turn the ship towards this target 
	DetermineYawAndPitchToTarget(*this, XMVectorAdd(m_avoid_target->GetPosition(), response), outPitchYaw);
}

void Ship::DetermineNewPosition(void)
{
	// If we have angular velocity then apply it to the ship orientation now
	if (!IsZeroVector3(this->PhysicsState.AngularVelocity))
	{
		// We can use a shorter implementation if no banking is involved
		if (!IsZeroVector3(Bank) || !IsZeroVector3(m_new_bank))
		{
			// Calculate a new orientation including banking adjustments
			XMVECTOR invbank = XMQuaternionInverse(XMQuaternionRotationRollPitchYawFromVector(Bank));
			XMVECTOR angvel = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(PhysicsState.AngularVelocity, Game::TimeFactor));
			XMVECTOR newbank = XMQuaternionRotationRollPitchYawFromVector(m_new_bank);

			m_unadjusted_orient = XMQuaternionMultiply(XMQuaternionMultiply(invbank, angvel), m_orientation);
			m_inv_unadjusted_orient = XMQuaternionInverse(m_unadjusted_orient);

			SetOrientation(XMQuaternionMultiply(newbank, m_unadjusted_orient));

			// Update the current bank value to the new banking value
			Bank = m_new_bank;
		}
		else
		{
			// Simply use an unadjusted orientation
			XMVECTOR angvel = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(PhysicsState.AngularVelocity, Game::TimeFactor));
			m_unadjusted_orient = XMQuaternionMultiply(angvel, m_orientation);
			m_inv_unadjusted_orient = XMQuaternionInverse(m_unadjusted_orient);
			SetOrientation(m_unadjusted_orient);

			// No need to update the new/old bank values since both are zero
		}
	}

	// Derive a new heading for the ship based on its orientation quaternion
	PhysicsState.Heading = XMVector3Rotate(BASIS_VECTOR, m_orientation);

	// For now, simply apply the world momentum vector to our current position to get a new position
	// Weight momentum by the time factor to get a consistent momentum change per second
	if (!IsZeroVector3(this->PhysicsState.WorldMomentum))
	{
		// Update ship location by applying a time-weighted fraction of this world momentum to the current location
		// Also store the distance that was moved this cycle, for use by the physics/collision engine later
		XMVECTOR delta = XMVectorMultiply(PhysicsState.WorldMomentum, Game::TimeFactorV);
		this->AddDeltaPosition(delta);
		PhysicsState.DeltaMoveDistanceSq = XMVectorGetX(XMVector3LengthSq(delta));
	}

}

// Turns the ship by a specified percentage of its total turn capability in each axis, banking if required
void Ship::TurnShip(float yaw_pc, float pitch_pc, bool bank)
{
	// Store the desired pitch and yaw.  This will overwrite any previous target values
	m_targetyaw = yaw_pc;
	m_targetpitch = pitch_pc;

	// We will now calculate an additional banking factor if required.  If not, we normalise the banking back to zero
	if (!bank) 
	{ 
		// If we are already at zero bank, and we don't want to bank, there is nothing more to do and we can quit here
		if (IsZeroVector3(Bank))
		{
			m_new_bank = NULL_VECTOR;
			m_isturning = true;
			return;
		}

		// We don't want to bank, but are at non-zero bank, so revert bank to zero over time
		pitch_pc = yaw_pc = 0.0f; 
	}
	
	// Target bank is our current pitch/yaw values multiplied by the maximum possible bank
	XMVECTOR yawpitch = XMVectorSet(pitch_pc, yaw_pc, -yaw_pc, 0.0f);
	XMVECTOR target_bank_amt = XMVectorMultiply(yawpitch, BankExtent);

	// Target bank increment is the difference between this and our current bank state
	XMVECTOR target_bank_inc = XMVectorSubtract(target_bank_amt, Bank);

	// We can only bank by a certain amount per cycle, determined by our bank rate and the frame time
	XMVECTOR max_bank = XMVectorMultiply(XMVectorReplicate(BankRate.Value), Game::TimeFactorV);
	XMVECTOR bank_inc = XMVectorClamp(target_bank_inc, XMVectorNegate(max_bank), max_bank);

	// The new bank value will be the current value plus this increment, constrained within the valid bank extents
	m_new_bank = XMVectorClamp(XMVectorAdd(Bank, bank_inc), XMVectorNegate(BankExtent), BankExtent);

	// Signal that the ship is currently turning (until next execution of the flight computer)
	m_isturning = true;
}

// Turns the ship to a specified target object, banking if required
void Ship::TurnToTarget(iObject *target, bool bank)
{
	// Parameter check
	XMFLOAT2 pitch_yaw;
	if (!target) return;

	// Determine the yaw and pitch required to align ourselves with this target
	DetermineYawAndPitchToTarget((*this), target->GetPosition(), pitch_yaw);

	// Multiply the turn percentages by the turn modifier for our current state (peaceful, in combat, etc)
	pitch_yaw.x *= m_turnmodifier; pitch_yaw.y *= m_turnmodifier;

	// Initiate a turn in this direction
	TurnShip(pitch_yaw.y, pitch_yaw.x, true);
}

// Turns the ship to a specified target position, banking if required
void Ship::TurnToTarget(FXMVECTOR target, bool bank)
{
	XMFLOAT2 pitch_yaw;

	// Determine the yaw and pitch required to align ourselves with this target position
	DetermineYawAndPitchToTarget((*this), target, pitch_yaw);

	// Multiply the turn percentages by the turn modifier for our current state (peaceful, in combat, etc)
	pitch_yaw.x *= m_turnmodifier; pitch_yaw.y *= m_turnmodifier;

	// Initiate a turn in this direction
	TurnShip(pitch_yaw.y, pitch_yaw.x, true);
}

// Validates and sets the ship bank extents (in radians)
void Ship::SetBankExtent(const FXMVECTOR extent_radians)
{
	// Ensure the extents are positive and non-zero, to allow unchecked division by the bank extents
	// (e.g. Bank/BankExtents for % banking) without division by zero errors
	BankExtent = XMVectorSetW(XMVectorMax(extent_radians, Game::C_EPSILON_V), 0.0f);
}

// Returns a bool indicating whether a ship can accept a specified class of order.  Overridden with additional orders by simple/complex subclasses
bool Ship::CanAcceptOrderType(Order::OrderType type)
{
	// Return true for any order type a ship (regardless of type) can accept
	switch (type)
	{
		// Order types below are all valid options for a ship
		case Order::OrderType::MoveToPosition:
		case Order::OrderType::MoveToTarget:
		case Order::OrderType::MoveAwayFromTarget:
		case Order::OrderType::AttackBasic:
			
			return true; 

		// Otherwise we return false
		default:
			return false;
	}
}

// Method to process the specified order.  Called when processing the full queue.  Return value indicates whether order is now completed & can be removed
Order::OrderResult Ship::ProcessOrder(Order *order)
{
	// Determine the type of order being processed
	if (!order) return Order::OrderResult::InvalidOrder;
	Order::OrderType type = order->GetType();

	// Take different action depending on the order type
	switch (type)
	{
		// Move to position.  Specifies position and the distance to which we must move within
		case Order::OrderType::MoveToPosition:
			return MoveToPosition((Order_MoveToPosition&)*order);

		// Move to target.  Specifies the target and the distance to which we must move within
		case Order::OrderType::MoveToTarget:
			return MoveToTarget((Order_MoveToTarget&)*order);

		// Move away from the specified target
		case Order::OrderType::MoveAwayFromTarget:
			return MoveAwayFromTarget((Order_MoveAwayFromTarget&)*order);

		// Perform a basic attack on the target.  Will close on the target while firing, then
		// peel off and circle for another run
		case Order::OrderType::AttackBasic:
			return AttackBasic((Order_AttackBasic&)*order);


		// If this is not an order we can execute, return an invalid order result to have it removed from the queue
		default:
			return Order::OrderResult::InvalidOrder;
	}
}

// Moves the ship to a target position, within a certain tolerance.  Returns a flag indicating whether we have reached the target
bool Ship::_MoveToPosition(const FXMVECTOR position, float tolerance_sq)
{
	// Completion check:  See whether we have reached the target position.  Calculated (squared) distance to target
	float distsq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(m_position, position)));

	// If we are within the close distance then set target speed to zero and mark the order as completed
	if (distsq < tolerance_sq)
	{
		FullStop();
		return true;
	}

	// Otherwise we need to continue on a course to the target.  Turn the ship towards the target position
	this->TurnToTarget(position, true);

	// For now, accelerate to max thrust.  TODO: later, potentially apply a gradual decrease in thrust as ship approaches the target
	this->SetTargetSpeedPercentage(1.0f);

	// We have not yet reached the target
	return false;
}

// Moves the ship to a target position.  There is no completion check for this order; used when we have just determined
// a target position that is far away, and we do not need to check when we get there (since e.g. the order will be checking)
void Ship::_MoveToPosition_NoCompletionCheck(const FXMVECTOR position)
{
	// Set a course to the target.  Turn the ship towards the target position
	this->TurnToTarget(position, true);

	// For now, accelerate to max thrust.  TODO: later, potentially apply a gradual decrease in thrust as ship approaches the target
	this->SetTargetSpeedPercentage(1.0f);
}


// Order: Moves the ship to a target position, within a certain tolerance
Order::OrderResult Ship::MoveToPosition(Order_MoveToPosition & order)
{
	if (_MoveToPosition(order.Target, order.CloseDistanceSq) == true)
		return Order::OrderResult::ExecutedAndCompleted;
	else
		return Order::OrderResult::Executed;
}

// Order: Moves the ship to a target object, within a certain tolerance
Order::OrderResult Ship::MoveToTarget(Order_MoveToTarget & order)
{
	// Parameter check
	if (!order.Target) return Order::OrderResult::InvalidOrder;

	// Call the primary movement function; this is essentially a move-to-position order for the target's current position
	if (_MoveToPosition(order.Target->GetPosition(), order.CloseDistanceSq) == true)
		return Order::OrderResult::ExecutedAndCompleted;
	else
		return Order::OrderResult::Executed;
}

// Order: Moves the ship a specified distance away from some target
Order::OrderResult Ship::MoveAwayFromTarget(Order_MoveAwayFromTarget & order)
{
	// Parameter check
	if (!order.Target) return Order::OrderResult::InvalidOrder;

	// Test whether we have retreated far enough from the target ship
	XMVECTOR tgt_to_ship = XMVectorSubtract(m_position, order.Target->GetPosition());
	XMVECTOR distsq = XMVector3LengthSq(tgt_to_ship);
	if (XMVector2Greater(distsq, order.RetreatDistanceSqV))
	{
		FullStop();
		return Order::OrderResult::ExecutedAndCompleted;
	}

	// Our retreat vector will be a linear interpolation between (A) the vector away from the target, through us, and (B) the 
	// current world momentum of our ship.  The momentum weighting defines the contribution of each.  ((1-Weight)*A + Weight*B)
	XMVECTOR vec = XMVectorLerp(XMVector3NormalizeEst(tgt_to_ship), XMVector3NormalizeEst(PhysicsState.WorldMomentum), order.MomentumWeighting);

	// Retreat along this vector; we set a distance well outside our retreat distance, and do not need to check for completion
	// since the completion event will be when we trigger the distance check above
	_MoveToPosition_NoCompletionCheck(XMVectorScale(vec, order._VectorTravelTarget));
	return Order::OrderResult::Executed;
}

// Order: Perform a basic attack on the target.  Will close on the target while firing, then
// peel off and circle for another run
Order::OrderResult Ship::AttackBasic(Order_AttackBasic & order)
{
	// Parameter check
	if (!order.Target) return Order::OrderResult::InvalidOrder;

	// TODO: If the target is destroyed, or no longer exists, return executed & completed

	// We need a new sub-order; if we are outside the retreat range, close on the target
	XMVECTOR distsq = XMVector3LengthSq(XMVectorSubtract(order.Target->GetPosition(), m_position));
	if (XMVector2Greater(distsq, order.RetreatDistSqV))
	{
		// We want to close on the target; give an order to move into the object within the desired close distance
		Order_MoveToTarget *move = new Order_MoveToTarget(order.Target, order.CloseDist);
		AssignNewOrder(move);

		// Set this sub-order as a dependency and assign it; control will return to this order when "move" completes
		move->Parent = order.ID; 
		order.Dependency = move->ID;
		return Order::OrderResult::Executed;
	}
	else
	{
		// We want to put some distance between ourself and the target
		Order_MoveAwayFromTarget *move = new Order_MoveAwayFromTarget(order.Target, order.RetreatDist, 0.5f);
		AssignNewOrder(move);

		// Set this sub-order as a dependency and assign it; control will return to this order when "move" completes
		move->Parent = order.ID;
		order.Dependency = move->ID;
		return Order::OrderResult::Executed;
	}

}

// Event triggered upon destruction of the object
void Ship::DestroyObject(void)
{
	OutputDebugString("Destruction of Ship\n");
}


// Default destructor
Ship::~Ship(void)
{

}




