#pragma once

#ifndef __iActiveObjectH__
#define __iActiveObjectH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "iObject.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class iActiveObject : public ALIGN16<iActiveObject>, public iObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(iActiveObject)

	// Default constructor and destructor
	iActiveObject(void);
	virtual ~iActiveObject(void) = 0;

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void								InitialiseCopiedObject(iActiveObject *source);

	// Object holding physics information on the object.  16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct ObjectPhysicsState
	{
		// Primary values, i.e. those maintained directly
		AXMVECTOR					Acceleration;			// m/s/s			Derived from sum of all engine thrust & mass
		AXMVECTOR					WorldMomentum;			// m/s				Directional vector momentum in world space
		AXMVECTOR					Heading;				// Vector3			Ship heading, derived from orientation * the basis vector
		AXMVECTOR					AngularVelocity;		// rad/sec			Angular velocity of the ship

		// Secondary values, i.e. those derived from the primary values
		AXMVECTOR					LocalMomentum;			// m/s				Current directional vector momentum.  Derived from world momentum once per cycle
		AXMVECTOR					WorldAcceleration;		// m/s/s			Acceleration in world space.  Derived from local acceleration once per cycle

		// Inertia tensor for performing angular momentum/velocity derivation
		AXMMATRIX					InertiaTensor;
		AXMMATRIX					InverseInertiaTensor;

		// The distance sq that the object moved during this physics cycle
		float						DeltaMoveDistanceSq;	// m

		// Default constructor
		ObjectPhysicsState(void);

		// Copy constructor
		ObjectPhysicsState(const ObjectPhysicsState & other);

		// Assignment operator
		ObjectPhysicsState& operator=(const ObjectPhysicsState&);
		
		// Detructor
		CMPINLINE ~ObjectPhysicsState(void) { }

	} PhysicsState;


	// Access to object mass is controlled
	CMPINLINE float							GetMass(void) const					{ return m_mass; }
	CMPINLINE float							GetInverseMass(void) const			{ return m_invmass; }
	void									SetMass(const float mass);

	// Returns the impact resistance of this object, i.e. the remaining force it can withstand from physical 
	// impacts, with an impact point at the specified element
	float									GetImpactResistance(void) const;


	// Virtual method, called when this object collides with another
	virtual void							CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact);// = 0;

	// Returns a flag indicating whether this object is a 'fast mover' that should be simulated via continuous (CCD) rather
	// than discrete collision detection.  Definition of a 'fast mover' is an object that will move more than a defined
	// percentage of its minimum extent in a single frame
	CMPINLINE bool							IsFastMover(void) const { return (PhysicsState.DeltaMoveDistanceSq > m_fastmoverthresholdsq); }

	// Returns a flag indicating whether this object is static, i.e. has no linear or angular momentum
	CMPINLINE bool							IsStatic(void) const { return (IsZeroVector3(PhysicsState.WorldMomentum) && IsZeroVector3(PhysicsState.AngularVelocity)); }

	// Methods to apply external forces to the object
	void									ApplyLocalForce(const FXMVECTOR localposition, FXMVECTOR localforcevector);
	void									ApplyWorldSpaceForce(const FXMVECTOR worldposition, const FXMVECTOR worldforcevector);

	// Apply angular momentum to an object.  Calculates and applies the incremental angular velocity
	CMPINLINE void							ApplyAngularMomentum(const FXMVECTOR angular_momentum);

	// Apply angular velocity to an object.  Not scaled by the object intertia tensor
	CMPINLINE void							ApplyAngularVelocity(const FXMVECTOR angular_velocity);

	// Set the local momentum of the object, recalculating the object world momentum accordingly
	CMPINLINE void							SetLocalMomentum(const FXMVECTOR m)
	{
		// We are directly setting the local momentum, so do not need to transform into world space first
		PhysicsState.LocalMomentum = m;
		RecalculateWorldMomentum();
	}

	// Add local momentum to the object, recalculating the object world momentum accordingly
	CMPINLINE void							AddLocalMomentum(const FXMVECTOR dm)
	{
		// We first want to transform the local momentum delta into world space, and apply the increment in world space, 
		// to preserve the existing world momentum of this object
		XMVECTOR world_dm = XMVector3TransformCoord(dm, m_orientationmatrix);

		// Add the world momentum, which will automatically recalculate the new overall local momentum
		AddWorldMomentum(world_dm);
	}

	// Set the world momentum of the object, recalculating the object local momentum accordingly
	CMPINLINE void							SetWorldMomentum(const FXMVECTOR m)
	{
		PhysicsState.WorldMomentum = m;
		RecalculateLocalMomentum();
	}

	// Add world momentum to the object, recalculating the object local momentum accordingly
	CMPINLINE void							AddWorldMomentum(const FXMVECTOR dm)
	{
		PhysicsState.WorldMomentum = XMVectorSetW(XMVectorAdd(PhysicsState.WorldMomentum, dm), 0.0f);
		RecalculateLocalMomentum();
	}

	// Recalculates the object local momentum based upon its current world momentum
	CMPINLINE void							RecalculateLocalMomentum(void)
	{
		PhysicsState.LocalMomentum = XMVector3TransformCoord(PhysicsState.WorldMomentum, m_inverseorientationmatrix);
	}

	// Recalculates the object world momentum based upon its current local momentum.  This is rarely a good idea since it will not
	// preserve the existing world momentum.
	CMPINLINE void							RecalculateWorldMomentum(void)
	{
		PhysicsState.WorldMomentum = XMVector3TransformCoord(PhysicsState.LocalMomentum, m_orientationmatrix);
	}

	// Method to recalculate the object inertia tensor.  Could be called whenever a contributing factor (mass/size) changes
	void									RecalculateInertiaTensor(void);

	// Applies a world-space linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	CMPINLINE void							ApplyWorldSpaceLinearForce(const FXMVECTOR worldforcevector)
	{
		// Apply the change in linear world momentum, accounting for mass (a = F/m).  Recalculates local momentum in the process
		AddWorldMomentum(XMVectorScale(worldforcevector, m_invmass));
	}

	// Apply a local linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	CMPINLINE void							ApplyLocalLinearForce(const FXMVECTOR localforcevector)
	{
		// Apply the change in linear local momentum, accounting for mass (a = F/m).  Recalculates world momentum in the process
		AddLocalMomentum(XMVectorScale(localforcevector, m_invmass));
	}

	// Applies a world-space linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	// Force is applied directly, i.e. it is not divided through by mass (this is assumed to have already happened)
	CMPINLINE void							ApplyWorldSpaceLinearForceDirect(const FXMVECTOR worldforcevector)
	{
		// Apply the change in linear world momentum.  Ignore object mass; is already accounted for
		// in F.  Recalculates local momentum in the process
		AddWorldMomentum(worldforcevector);
	}

	// Apply a local linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	// Force is applied directly, i.e. it is not divided through by mass (this is assumed to have already happened)
	CMPINLINE void							ApplyLocalLinearForceDirect(const FXMVECTOR localforcevector)
	{
		// Apply the change in linear local momentum.  Ignore object mass; is already accounted for
		// in F.  Recalculates world momentum in the process
		AddLocalMomentum(localforcevector);
	}

	// Accessor methods for key data in the PhysicsState structure
	CMPINLINE XMVECTOR						GetAcceleration(void) const					{ return PhysicsState.Acceleration; }
	CMPINLINE XMVECTOR						GetWorldMomentum(void) const				{ return PhysicsState.WorldMomentum; }
	CMPINLINE XMVECTOR						GetHeading(void) const						{ return PhysicsState.Heading; }
	CMPINLINE XMVECTOR						GetAngularVelocity(void) const				{ return PhysicsState.AngularVelocity; }
	CMPINLINE XMVECTOR						GetLocalMomentum(void) const				{ return PhysicsState.LocalMomentum; }
	CMPINLINE XMVECTOR						GetWorldAcceleration(void) const			{ return PhysicsState.WorldAcceleration; }
	CMPINLINE XMMATRIX						GetInertiaTensor(void) const				{ return PhysicsState.InertiaTensor; }
	CMPINLINE XMMATRIX						GetInverseInertiaTensor(void) const			{ return PhysicsState.InverseInertiaTensor; }
	CMPINLINE float							GetDeltaMoveDistSq(void) const				{ return PhysicsState.DeltaMoveDistanceSq; }


	// Shut down the object, unregister it and deallocate all resources
	void									Shutdown(void);


	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void									ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// Mass and precalculated inverse mass of the object
	float									m_mass, m_invmass;				

	// Output debug data on the object.  Internal method that passes a stringbuilder up the hierarchy for more efficient construction
	//void									DebugOutput(std::ostringstream &ss) const;

};


// Apply angular momentum to an object.  Calculates and applies the incremental angular velocity
CMPINLINE void iActiveObject::ApplyAngularMomentum(const FXMVECTOR angular_momentum)
{
	// Transform the desired angular momentum by the inverse inertia tensor to get the change in angular velocity
	XMVECTOR avec = XMVector3TransformCoord(angular_momentum, PhysicsState.InverseInertiaTensor);

	// Apply this change in angular velocity
	PhysicsState.AngularVelocity = XMVectorSetW(XMVectorAdd(PhysicsState.AngularVelocity, avec), 0.0f);
}

// Apply angular velocity to an object.  Not scaled by the object intertia tensor
CMPINLINE void iActiveObject::ApplyAngularVelocity(const FXMVECTOR angular_velocity)
{
	// Simply apply this change in angular velocity directly
	PhysicsState.AngularVelocity = XMVectorSetW(XMVectorAdd(PhysicsState.AngularVelocity, angular_velocity), 0.0f);
}






#endif
