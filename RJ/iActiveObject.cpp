#include "DX11_Core.h"

#include "FastMath.h"
#include "GameVarsExtern.h"

#include "iActiveObject.h"

// Constructor; assigns a unique ID to this object
iActiveObject::iActiveObject(void)
{
	// Set fields to defaults
	
	// Initialise physical state of the object
	SetMass(10.0f);
	PhysicsState.Acceleration = PhysicsState.AngularVelocity = PhysicsState.Heading =
	PhysicsState.LocalMomentum = PhysicsState.WorldAcceleration = PhysicsState.WorldMomentum = NULL_VECTOR;
	PhysicsState.InertiaTensor = PhysicsState.InverseInertiaTensor = ID_MATRIX;
	PhysicsState.DeltaMoveDistanceSq = 0.0f;
}

// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iActiveObject::InitialiseCopiedObject(iActiveObject *source)
{
	// Pass control to all base classes
	iObject::InitialiseCopiedObject(source);
}


// Output debug data on the object.  Acts from this point in the hierarchy downwards
/*std::string iActiveObject::DebugOutput(void) const
{
	std::ostringstream ss;
	DebugOutput(ss);
	return ss.str();
}

// Output debug data on the object.  Internal method that passes a stringbuilder up the hierarchy for more efficient construction
void iActiveObject::DebugOutput(std::ostringstream &ss) const
{
	// Stream direct class data
	ss << "{ ";
	DbgValue(ss, "Mass", m_mass);
	DbgValue(ss, "InvMass", m_mass);
	DbgValue(ss, "Mass", m_mass);



	// Base classes
	iObject::DebugOutput(ss);
	ss << " }";
}*/


// Default destructor
iActiveObject::~iActiveObject(void)
{

}

// Applies an external force to the object.  Force and position are transformed from world space
void iActiveObject::ApplyWorldSpaceForce(const FXMVECTOR worldposition, const FXMVECTOR worldforcevector)
{
	// Determine the additional linear momentum applied to this ship.  Will be scaled down by object mass (a = F/m)
	// Automatically recalculates the equivalent local momentum
	ApplyWorldSpaceLinearForce(worldforcevector);

	// Transform the world space position & force vector into local space, since the angular momentum calculations
	// are performed in local object space
	XMVECTOR localpos, localforce, torque;
	localpos = XMVector3TransformCoord(worldposition, m_inverseworld);
	localforce = XMVector3TransformCoord(worldforcevector, m_inverseorientationmatrix);

	// Determine the additional angular momentum applied to this ship.  The torque applied is equal to the cross product between 
	// the vector from center of mass to the force application point, i.e. "localposition", and the force vector "localforce".
	// Scale the force by this object's inertia tensor to correctly calculate angular momentum change
	localforce = XMVector3TransformCoord(localforce, PhysicsState.InverseInertiaTensor);
	torque = XMVector3Cross(localpos, localforce);
	PhysicsState.AngularVelocity = XMVectorAdd(PhysicsState.AngularVelocity, torque); 
}

// Applies an external force to the object.  Force and position should be specified in local object space
void iActiveObject::ApplyLocalForce(const FXMVECTOR localposition, FXMVECTOR localforcevector)
{
	// Determine the additional linear momentum applied to this ship.  Will be scaled down by object mass (a = F/m)
	// Automatically recalculates the equivalent world momentum
	ApplyLocalLinearForce(localforcevector);

	// Determine the additional angular momentum applied to this ship.  The torque applied is equal to the cross product between 
	// the vector from center of mass to the force application point, i.e. "localposition", and the force vector "localforce".
	// Scale the force by this object's inertia tensor to correctly calculate angular momentum change
	XMVECTOR angular_force, torque;
	angular_force = XMVector3TransformCoord(localforcevector, PhysicsState.InverseInertiaTensor);
	torque = XMVector3Cross(localposition, angular_force);
	PhysicsState.AngularVelocity = XMVectorAdd(PhysicsState.AngularVelocity, torque);
}

// Sets the ship mass, recalculating derived fields as required
void iActiveObject::SetMass(const float mass)
{
	// We restrict mass to positive non-zero values, to ensure inverse mass is also always valid
	m_mass = max(mass, Game::C_EPSILON * 2.0f);
	m_invmass = max(1.0f / m_mass, Game::C_EPSILON * 2.0f);

	// We want to recalculate the object inertia tensor since the object mass has changed
	RecalculateInertiaTensor();
}


// Method to recalculate the object inertia tensor.  Called whenever a contributing factor (size, mass) changes
void iActiveObject::RecalculateInertiaTensor(void)
{
	// We need to use components of the size vector so store a local copy
	XMFLOAT3 sizef;
	XMStoreFloat3(&sizef, m_size);

	// Build an inertia tensor based upon the cuboid approximation (for now).  Dependent on size & mass.
	PhysicsState.InertiaTensor = XMMatrixSet(
		(1.0f / 12.0f) * m_mass * ((sizef.y*sizef.y) + (sizef.z*sizef.z)), 0.0f, 0.0f, 0.0f,
		0.0f, (1.0f / 12.0f) * m_mass * ((sizef.x*sizef.x) + (sizef.z*sizef.z)), 0.0f, 0.0f,
		0.0f, 0.0f, (1.0f / 12.0f) * m_mass * ((sizef.x*sizef.x) + (sizef.y*sizef.y)), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Precalculate the inverse I for runtime efficiency
	PhysicsState.InverseInertiaTensor = XMMatrixInverse(NULL, PhysicsState.InertiaTensor);
}

// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void iActiveObject::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(...)
	
	// Mutator methods
	REGISTER_DEBUG_FN(...)
	

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus != GameConsoleCommand::CommandResult::NotExecuted)		iObject::ProcessDebugCommand(command);

}
