#include "iContainsHardpoints.h"
#include "Hardpoint.h"
#include "HpEngine.h"
#include "FastMath.h"
#include "Octree.h"
#include "iSpaceObject.h"

#include "Ship.h"

Ship::Ship(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::ShipObject);

	this->m_shipclass = Ships::Class::Unknown;

	this->m_standardobject = false;
	this->m_defaultloadout = "";

	this->BaseMass = 1.0f;
	this->VelocityLimit.SetAllValues(1.0f);
	this->AngularVelocityLimit.SetAllValues(1.0f);
	this->TurnRate.SetAllValues(0.01f);
	this->TurnAngle.SetAllValues(0.01f);
	this->Bank = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	this->BankRate.SetAllValues(0.0f);
	this->BankExtent = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	this->EngineAngularAcceleration.SetAllValues(1000.0f);
	this->m_isbraking = false;
	this->BrakeFactor.SetAllValues(1.0f);
	this->m_flightcomputerinterval = Game::C_DEFAULT_FLIGHT_COMPUTER_EVAL_INTERVAL;
	this->m_timesincelastflightcomputereval = 0.0f;
	this->m_shipenginecontrol = true;
	this->m_targetspeed = m_targetspeedsq = m_targetspeedsqthreshold = 0.0f;
	this->m_isturning = false;
	this->m_targetangularvelocity = NULL_VECTOR;
	this->m_engineangularvelocity = m_engineangularmomentum = NULL_VECTOR;

	// Link the hardpoints collection to this parent object
	m_hardpoints.SetParent<Ship>(this);

	this->m_thrustchange_flag = true;		// Force an initial refresh
	this->m_masschange_flag = true;			// Force an initial refresh

	this->m_turnmodifier_peaceful = Game::C_AI_DEFAULT_TURN_MODIFIER_PEACEFUL;
	this->m_turnmodifier_combat = Game::C_AI_DEFAULT_TURN_MODIFIER_COMBAT;
	this->m_turnmodifier = m_turnmodifier_peaceful;

	this->MinBounds = D3DXVECTOR3(-0.5f, -0.5f, -0.5f);
	this->MaxBounds = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	this->m_centretransmatrix = m_centreinvtransmatrix = ID_MATRIX;
	this->OrientationAdjustment = ID_MATRIX;

	// Set the counter of orientation quaterion operations before a normalisation is required
	this->m_orientchanges = 0;
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

	// Update any other fields that should not be replicated through the standard copy constructor
	CancelAllOrders();
	SetTargetThrustOfAllEngines(0.0f);

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
	// Evaluate any orders in the queue
	ProcessOrderQueue(m_timesincelastflightcomputereval);

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
	float msq = (PhysicsState.LocalMomentum.x * PhysicsState.LocalMomentum.x) + 
				(PhysicsState.LocalMomentum.y * PhysicsState.LocalMomentum.y) + 
				(PhysicsState.LocalMomentum.z * PhysicsState.LocalMomentum.z);

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
	
}

void Ship::SimulateAllShipEngines(void)
{
	// Optimisation: check whether the thrust generated by any of our engines has changed (F), or whether our ship
	// mass has changed (m), since the last physics calculation of acceleration (a = F/m).  If not, we can retain 
	// the cached acceleration value and use it to save time
	if (ThrustVectorsChanged() || ShipMassChanged())
	{
		// Reset the variables we want to calculate
		this->PhysicsState.Acceleration = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		
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
				this->PhysicsState.Acceleration += (*(e->GetThrustVector()) * 1000.0f); // TODO: Constant-adjusted for physics
			}
		}		

		// Divide accumulated velocity vectors (F) through by mass (m) to get acceleration vector (a=F/m)
		this->PhysicsState.Acceleration *= this->GetInverseMass();

		// We have now recalculated the cached acceleration vector so reset the flags and time interval counter
		ResetThrustVectorChangeFlag();
		ResetShipMassChangeFlag();
	}

	// Now also calculate the angular velocity to be generated by the ship/engines/thrusters to meet the desired heading
	if (m_targetangularvelocity.x > PhysicsState.AngularVelocity.x)	
			m_engineangularvelocity.x = min(TurnRate.Value, m_targetangularvelocity.x - PhysicsState.AngularVelocity.x) * Game::TimeFactor;
	else	m_engineangularvelocity.x = max(-TurnRate.Value, m_targetangularvelocity.x - PhysicsState.AngularVelocity.x) * Game::TimeFactor;
	if (m_targetangularvelocity.y > PhysicsState.AngularVelocity.y)	
			m_engineangularvelocity.y = min(TurnRate.Value, m_targetangularvelocity.y - PhysicsState.AngularVelocity.y) * Game::TimeFactor;
	else	m_engineangularvelocity.y = max(-TurnRate.Value, m_targetangularvelocity.y - PhysicsState.AngularVelocity.y) * Game::TimeFactor;
	if (m_targetangularvelocity.z > PhysicsState.AngularVelocity.z)	
			m_engineangularvelocity.z = min(TurnRate.Value, m_targetangularvelocity.z - PhysicsState.AngularVelocity.z) * Game::TimeFactor;
	else	m_engineangularvelocity.z = max(-TurnRate.Value, m_targetangularvelocity.z - PhysicsState.AngularVelocity.z) * Game::TimeFactor;

	// Multiply the target angular momentum by the ship's angular acceleration, derived from its engines
	//m_engineangularvelocity *= (this->EngineAngularAcceleration.Value * 1000.0f);		// TODO: Constant-adjusted for physics

	// Working backwards, derive the angular momentum generated by the engines on the basis that they can generate this
	// degree of angular velocity for the ship.  TODO: perhaps change in future to derive momentum, and from that the velocity,
	// and then contrain to be within the allowable velocity bounds defined by TurnRate
	D3DXVec3TransformCoord(&m_engineangularmomentum, &m_engineangularvelocity, &PhysicsState.InertiaTensor);
}

void Ship::SimulateObjectPhysics(void)
{
	// Update momentum vector using ship acceleration, transformed to world space, scaled by the time factor that has passed
	D3DXVec3TransformCoord(&PhysicsState.WorldAcceleration, &PhysicsState.Acceleration, &m_orientationmatrix);
	this->PhysicsState.WorldMomentum += (this->PhysicsState.WorldAcceleration * Game::TimeFactor);

	// Limit this ship to its maximum tolerable velocity.  Or, if in hardcore mode, allow it but with damage
	if (Game::C_NO_MOMENTUM_LIMIT)
		;								// TODO: Apply damage effects proportionate to how far over max supportable velocity we are
	else
		ScaleVectorWithinMagnitudeLimit(PhysicsState.WorldMomentum, VelocityLimit.Value);

	// Apply a drag scalar to the momentum vector to prevent perpetual movement.  Again, weighted by
	// fraction of a second that has passed.  C_MOVEMENT_DRAG_FACTOR is % momentum decrease per second.
	if (!IsZeroVector(this->PhysicsState.WorldMomentum))
		this->PhysicsState.WorldMomentum -= (this->PhysicsState.WorldMomentum * Game::C_MOVEMENT_DRAG_FACTOR * Game::TimeFactor);

	// Also apply a drag factor if the ship is currently braking
	if (m_isbraking) {
		float velocityloss = (this->VelocityLimit.Value * Game::TimeFactor);

		if (this->PhysicsState.WorldMomentum.x > 0)	this->PhysicsState.WorldMomentum.x = max(0.0f, this->PhysicsState.WorldMomentum.x - velocityloss);
		else										this->PhysicsState.WorldMomentum.x = min(0.0f, this->PhysicsState.WorldMomentum.x + velocityloss);

		if (this->PhysicsState.WorldMomentum.y > 0)	this->PhysicsState.WorldMomentum.y = max(0.0f, this->PhysicsState.WorldMomentum.y - velocityloss);
		else										this->PhysicsState.WorldMomentum.y = min(0.0f, this->PhysicsState.WorldMomentum.y + velocityloss);

		if (this->PhysicsState.WorldMomentum.z > 0)	this->PhysicsState.WorldMomentum.z = max(0.0f, this->PhysicsState.WorldMomentum.z - velocityloss);
		else										this->PhysicsState.WorldMomentum.z = min(0.0f, this->PhysicsState.WorldMomentum.z + velocityloss);
	}

	// Apply the incremental angular velocity generated by ship engines this cycle
	this->ApplyAngularMomentum(m_engineangularmomentum);

	// Limit angular velocity to the ship's max acceptable velocity
	ScaleVectorWithinMagnitudeLimit(PhysicsState.AngularVelocity, AngularVelocityLimit.Value);

	// Apply a damping effect on angular velocity
	float angularvelocityloss = (Game::C_ANGULAR_VELOCITY_DAMPING_FACTOR * Game::TimeFactor);

	if (PhysicsState.AngularVelocity.x > 0)	PhysicsState.AngularVelocity.x = max(0.0f, PhysicsState.AngularVelocity.x - angularvelocityloss);
	else									PhysicsState.AngularVelocity.x = min(0.0f, PhysicsState.AngularVelocity.x + angularvelocityloss);

	if (PhysicsState.AngularVelocity.y > 0)	PhysicsState.AngularVelocity.y = max(0.0f, PhysicsState.AngularVelocity.y - angularvelocityloss);
	else									PhysicsState.AngularVelocity.y = min(0.0f, PhysicsState.AngularVelocity.y + angularvelocityloss);

	if (PhysicsState.AngularVelocity.z > 0)	PhysicsState.AngularVelocity.z = max(0.0f, PhysicsState.AngularVelocity.z - angularvelocityloss);
	else									PhysicsState.AngularVelocity.z = min(0.0f, PhysicsState.AngularVelocity.z + angularvelocityloss);

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

	// Test whether enough time has passed for the flight computer to be evaluated again
	m_timesincelastflightcomputereval += Game::TimeFactor;
	if (m_timesincelastflightcomputereval > m_flightcomputerinterval)
	{
		// Reset any persistence flags that maintain an action between one execution of the flight computer and the next
		m_isturning = false;

		// Execute the flight computer
		RunShipFlightComputer();
	}

	// Handle all ship movement, if permitted by the central simulator (if not, it means we will be moved
	// some other way, e.g. we attached to some other object which will calculate our position)
	if (CanSimulateMovement())
	{
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

	// Update position of the ship in the spatial partitioning tree
	if (m_treenode) m_treenode->ItemMoved(this, m_position);
}

// Perform the post-simulation update.  Pure virtual inherited from iObject base class
void Ship::PerformPostSimulationUpdate(void)
{
	// Derive a new world matrix based on our updated position/orientation
	// TOOD: Need to test for SpatialDataChanged() if the "IsPostSimulationUpdateRequired()" method starts considering
	// other fields besides the spatial data flag
	DeriveNewWorldMatrix();
}

void Ship::ChangeOrientation(const D3DXQUATERNION &rot)
{
	// Multiply orientation D3DXQUATERNIONs to generate the new D3DXQUATERNION
	D3DXQUATERNION orient = (rot * m_orientation);

	// Check whether we have passed the threshold number of quaternion operations that would necessitate a re-normalisation
	if (++m_orientchanges > QUATERNION_NORMALISATION_THRESHOLD)
	{
		// Normalise each of the key quaternions being maintained by this ship
		D3DXQuaternionNormalize(&orient, &orient);
		m_orientchanges = 0;
	}

	// Store the new orientation and recalculate derived data
	SetOrientation(orient);
}

void Ship::ChangeOrientationAndRecalculate(const D3DXQUATERNION &rot)
{
	// Multiply orientation D3DXQUATERNIONs 
	D3DXQUATERNION orient = (rot * m_orientation);
	
	// Immediately renormalise, resetting the normalisation counter
	D3DXQuaternionNormalize(&orient, &orient);
	m_orientchanges = 0;
	
	// Store the new orientation and recalculate derived data
	SetOrientation(orient);
}

void Ship::AddDeltaOrientation(const D3DXQUATERNION &dq)
{
	// Add the incremental quaternion
	D3DXQUATERNION orient = (m_orientation + dq);

	// Check whether we have passed the threshold number of quaternion operations that would necessitate a re-normalisation
	if (++m_orientchanges > QUATERNION_NORMALISATION_THRESHOLD)
	{
		// Normalise each of the key quaternions being maintained by this ship
		D3DXQuaternionNormalize(&orient, &orient);
		m_orientchanges = 0;
	}

	// Store the new orientation and recalculate derived data
	SetOrientation(orient);
}

void Ship::AddDeltaOrientationAndRecalculate(const D3DXQUATERNION &dq)
{
	// Add the incremental quaternion
	D3DXQUATERNION orient = (m_orientation + dq);

	// Immediately renormalise, resetting the normalisation counter
	D3DXQuaternionNormalize(&orient, &orient);
	m_orientchanges = 0;

	// Store the new orientation and recalculate derived data
	SetOrientation(orient);
}

void Ship::DetermineNewPosition(void)
{
	// If we have angular velocity then apply it to the ship orientation now
	if (!IsZeroVector(this->PhysicsState.AngularVelocity))
	{
		this->AddDeltaOrientation(0.5f * D3DXQUATERNION(this->PhysicsState.AngularVelocity.x, this->PhysicsState.AngularVelocity.y, 
									this->PhysicsState.AngularVelocity.z, 0.0f) * this->m_orientation * Game::TimeFactor);
	}

	// Derive a new heading for the ship based on its orientation quaternion
	D3DXVec3TransformCoord(&(PhysicsState.Heading), &BASIS_VECTOR, &m_orientationmatrix);

	// For now, simply apply the world momentum vector to our current position to get a new position
	// Weight momentum by the time factor to get a consistent momentum change per second
	if (!IsZeroVector(this->PhysicsState.WorldMomentum))
	{
		// Update ship location by applying a time-weighted fraction of this world momentum to the current location
		// Also store the distance that was moved this cycle, for use by the physics/collision engine later
		D3DXVECTOR3 delta = PhysicsState.WorldMomentum * Game::TimeFactor;
		this->AddDeltaPosition(delta);
		PhysicsState.DeltaMoveDistanceSq = (delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
	}

}

// Turns the ship by a specified percentage of its total turn capability in each axis, banking if required
void Ship::TurnShip(float yaw_pc, float pitch_pc, bool bank)
{
	// Determine the target angular velocity for the ship engines to work towards
	this->m_targetangularvelocity = D3DXVECTOR3(pitch_pc * TurnAngle.Value, yaw_pc * TurnAngle.Value, 0.0f);

	// Calculate an additional banking factor if required
	if (bank)
	{
		// We only need to change banking values if the game is running.  This is a bit of a hack, since everything else in the game naturally
		// accomodates pausing since it incorporates the Game::TimeFactor or Game::ClockMs.  However this banking logic (specifically the 
		// derivation of be_* and abe_*) is not weighted by time and therefore needs to be limited by a direct check of the pause flag
		if (!Game::Paused)
		{
			// Apply a banking factor to the turn as well, depending on the extent to which we are turning
			float bf_yz = this->BankRate.Value * Game::TimeFactor * yaw_pc;			// Bank factor for yaw/roll (about y/z)
			float bf_x = this->BankRate.Value * Game::TimeFactor * pitch_pc;		// Bank factor for pitch (about x)

			// Determine bank limits based on the current degree of turning in each axis
			float be_x = pitch_pc * this->BankExtent.x; float abe_x = fabs(be_x);
			float be_y = yaw_pc   * this->BankExtent.y; float abe_y = fabs(be_y);
			float be_z = yaw_pc   * this->BankExtent.z; float abe_z = fabs(be_z);

			// Add the bank factor and clamp to lie within the allowable range for this degree of turn
			this->Bank.x = Clamp(this->Bank.x + (bf_x  *  this->BankExtent.x), -abe_x, abe_x);
			this->Bank.y = Clamp(this->Bank.y + (bf_yz *  this->BankExtent.y), -abe_y, abe_y);
			this->Bank.z = Clamp(this->Bank.z + (bf_yz * -this->BankExtent.z), -abe_z, abe_z);

			// Now generate the orientation adjustment matrix from these banking values
			D3DXMatrixRotationYawPitchRoll(&this->OrientationAdjustment,
											this->Bank.y,
											this->Bank.x,
											this->Bank.z);
		}
	}
	else
	{
		// If no banking is required we can simply use the identity matrix rather than an adjustment
		this->OrientationAdjustment = ID_MATRIX;
	}

	// Signal that the ship is currently turning (until next execution of the flight computer)
	m_isturning = true;

	// Apply the delta "rot" D3DXQUATERNION to the ship world orientation quaternion.  This will automatically recalculate
	// the model orientation matrix based on orientation & any adjustments
	// this->ChangeOrientation(m_targetturn);
}

// Determines the yaw and pitch required to turn the ship to face a point in space.  Both values are [0.0-1.0] turn percentages
void Ship::DetermineYawAndPitchToTarget(D3DXVECTOR3 target, float *pOutYaw, float *pOutPitch)
{
	// Determine the difference vector to this target, transform into local coordinate space (where our heading is the basis
	// vector [0, 0, 1], for mathematical simplicity) and normalise the difference vector
	D3DXVECTOR3 tgt = (target - m_position);
	D3DXVec3TransformCoord(&tgt, &tgt, &m_inverseorientationmatrix);
	D3DXVec3Normalize(&tgt, &tgt);		// TODO: can optimise this method?

	// Calculate the cross and dot products for ship yaw
	/* 
		heading = BASIS_VECTOR;		
		tgt = (s2->Location - ss->Location);	// (tgt is then transformed by the inverse ship orientation matrix)

		Optimisation: we know heading = the basis vector, so can substitute components for 0,0,1 and simplify accordingly  
		ycross = (heading.z*tgt.x) - (heading.x*tgt.z);		= (1*tgt.x) - (0*tgt.z)		= tgt.x
		ydot = (heading.z*tgt.z) + (heading.x*tgt.x);		= (1*tgt.z) + (0*tgt.x)		= tgt.z
		pcross = (heading.y*tgt.z) - (heading.z*tgt.y);		= (0*tgt.z) - (1*tgt.y)		= -tgt.y 
		pdot = (heading.x*tgt.x) + (heading.z*tgt.z);		= (0*tgt.x) + (1*tgt.z)		= tgt.z

		We therefore don't need to even maintain heading as a variable.  We can also just use tgt components in place of cross/dot
	*/

	// Determine yaw value depending on the current angle to target
	if (fast_abs(tgt.x) > 0.01f)	(*pOutYaw) = tgt.x;		// Plot a yaw component proportionate to the angle the ship needs to cover
	else {
		if	(tgt.z < 0.0f)			(*pOutYaw) = -1.0f;		// We are over 180deg from the target, so perform a full turn
		else						(*pOutYaw) = 0.0f;		// We are on the correct heading so maintain yaw
	}

	// Now determine pitch value, also based on the current angle to target
	if (fast_abs(tgt.y) > 0.01f)	(*pOutPitch) = -tgt.y;	// Plot a pitch component proportionate to the angle the ship needs to cover
	else {
		if	(tgt.z < 0.0f)			(*pOutPitch) = -1.0f;	// We are over 180deg from the target, so perform a full turn
		else						(*pOutPitch) = 0.0f;	// We are on the correct heading so maintain pitch
	}
}

// Turns the ship to a specified target object, banking if required
void Ship::TurnToTarget(iObject *target, bool bank)
{
	// Parameter check
	float yaw, pitch;
	if (!target) return;

	// Determine the yaw and pitch required to align ourselves with this target
	DetermineYawAndPitchToTarget(target, &yaw, &pitch);

	// Multiply the turn percentages by the turn modifier for our current state (peaceful, in combat, etc)
	yaw *= m_turnmodifier; pitch *= m_turnmodifier;

	// Initiate a turn in this direction
	TurnShip(yaw, pitch, true);
}

// Turns the ship to a specified target position, banking if required
void Ship::TurnToTarget(D3DXVECTOR3 target, bool bank)
{
	float yaw, pitch;

	// Determine the yaw and pitch required to align ourselves with this target position
	DetermineYawAndPitchToTarget(target, &yaw, &pitch);

	// Multiply the turn percentages by the turn modifier for our current state (peaceful, in combat, etc)
	yaw *= m_turnmodifier; pitch *= m_turnmodifier;

	// Initiate a turn in this direction
	TurnShip(yaw, pitch, true);
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
			
			return true; 

		// Otherwise we return false
		default:
			return false;
	}
}

// Method to process the specified order.  Called when processing the full queue.  Return value indicates whether order is now completed & can be removed
Order::OrderResult Ship::ProcessOrder(Order *order)
{
	Order::OrderType type;

	// Determine the type of order being processed
	if (!order) return Order::OrderResult::InvalidOrder;
	type = order->GetType();

	// Take different action depending on the order type
	switch (type)
	{
		// Move to position.  Specifies position and the distance to which we must move within
		case Order::OrderType::MoveToPosition:
			return MoveToPosition(	order->Parameters.Float3_1,		// Target position
									order->Parameters.Float3_2.x);	// Distance to move within

		// Move to target.  Specifies the target and the distance to which we must move within
		case Order::OrderType::MoveToTarget:
			return MoveToTarget(	order->Parameters.Target_1,		// The target we should move towards
									order->Parameters.Float3_1.x);	// Distance to move within



			// If this is not an order we can execute, return an invalid order result to have it removed from the queue
		default:
			return Order::OrderResult::InvalidOrder;
	}
}

// Order: Moves the ship to a target position, within a certain tolerance
Order::OrderResult Ship::MoveToPosition(D3DXVECTOR3 position, float closedistance)
{
	// Completion check:  See whether we have reached the target position.  Calculated (squared) distance to target and threshold
	D3DXVECTOR3 diffvec = D3DXVECTOR3((m_position.x - position.x), (m_position.y - position.y), (m_position.z - position.z));
	float distsq = ((diffvec.x * diffvec.x) + (diffvec.y * diffvec.y) + (diffvec.z * diffvec.z));
	float closedistsq = (closedistance * closedistance);

	// If we are within the close distance then set target speed to zero and mark the order as completed
	if (distsq < closedistsq)
	{
		this->SetTargetSpeed(0.0f);
		return Order::OrderResult::ExecutedAndCompleted;
	}	

	// Otherwise we need to coninue on a course to the target.  Turn the ship towards the target position
	this->TurnToTarget(position, true);

	// For now, accelerate to max thrust.  TODO: later, potentially apply a gradual decrease in thrust as ship approaches the target
	this->SetTargetSpeedPercentage(1.0f);

	// Return a value indicating that we have executed the order
	return Order::OrderResult::Executed;
}

// Order: Moves the ship to a target object, within a certain tolerance
Order::OrderResult Ship::MoveToTarget(iSpaceObject *target, float closedistance)
{
	// Call the overloaded function; this is essentially a move-to-position order for the target's current position
	if (target)
		return MoveToPosition(target->GetPosition(), closedistance);
	else
		return Order::OrderResult::InvalidOrder;
}


// Default destructor
Ship::~Ship(void)
{

}



