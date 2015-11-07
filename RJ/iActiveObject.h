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

	// Default constructor and destructor
	iActiveObject(void);
	~iActiveObject(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void								InitialiseCopiedObject(iActiveObject *source);

	// Object holding physics information on the object.  16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct ObjectPhysicsState
	{
		// Primary values, i.e. those maintained directly
		XMVECTOR					Acceleration;			// m/s/s			Derived from sum of all engine thrust & mass
		XMVECTOR					WorldMomentum;			// m/s				Directional vector momentum in world space
		XMVECTOR					Heading;				// Vector3			Ship heading, derived from orientation * the basis vector
		XMVECTOR					AngularVelocity;		// rad/sec			Angular velocity of the ship

		// Secondary values, i.e. those derived from the primary values
		XMVECTOR					LocalMomentum;			// m/s				Current directional vector momentum.  Derived from world momentum once per cycle
		XMVECTOR					WorldAcceleration;		// m/s/s			Acceleration in world space.  Derived from local acceleration once per cycle

		// Inertia tensor for performing angular momentum/velocity derivation
		XMMATRIX					InertiaTensor;
		XMMATRIX					InverseInertiaTensor;

		// The distance sq that the object moved during this physics cycle
		float						DeltaMoveDistanceSq;	// m

	} PhysicsState;


	// Access to object mass is controlled
	CMPINLINE float							GetMass(void) const					{ return m_mass; }
	CMPINLINE float							GetInverseMass(void) const			{ return m_invmass; }
	void									SetMass(const float mass);



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
		PhysicsState.WorldMomentum += dm;
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

protected:

	// Mass and precalculated inverse mass of the object
	float									m_mass, m_invmass;				

};


// Apply angular momentum to an object.  Calculates and applies the incremental angular velocity
CMPINLINE void iActiveObject::ApplyAngularMomentum(const FXMVECTOR angular_momentum)
{
	// Transform the desired angular momentum by the inverse inertia tensor to get the change in angular velocity
	XMVECTOR avec = XMVector3TransformCoord(angular_momentum, PhysicsState.InverseInertiaTensor);

	// Apply this change in angular velocity
	PhysicsState.AngularVelocity = XMVectorAdd(PhysicsState.AngularVelocity, avec);
}

// Apply angular velocity to an object.  Not scaled by the object intertia tensor
CMPINLINE void iActiveObject::ApplyAngularVelocity(const FXMVECTOR angular_velocity)
{
	// Simply apply this change in angular velocity directly
	PhysicsState.AngularVelocity = XMVectorAdd(PhysicsState.AngularVelocity, angular_velocity);
}






#endif
