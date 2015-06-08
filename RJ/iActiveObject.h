#pragma once

#ifndef __iActiveObjectH__
#define __iActiveObjectH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "iObject.h"

class iActiveObject : public iObject
{
public:

	// Default constructor and destructor
	iActiveObject(void);
	~iActiveObject(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void								InitialiseCopiedObject(iActiveObject *source);

	// Object holding physics information on the object
	struct ObjectPhysicsState
	{
		// Primary values, i.e. those maintained directly
		D3DXVECTOR3					Acceleration;			// m/s/s			Derived from sum of all engine thrust & mass
		D3DXVECTOR3					WorldMomentum;			// m/s				Directional vector momentum in world space
		D3DXVECTOR3					Heading;				// Vector3			Ship heading, derived from orientation * the basis vector
		D3DXVECTOR3					AngularVelocity;		// rad/sec			Angular velocity of the ship

		// Secondary values, i.e. those derived from the primary values
		D3DXVECTOR3					LocalMomentum;			// m/s				Current directional vector momentum.  Derived from world momentum once per cycle
		D3DXVECTOR3					WorldAcceleration;		// m/s/s			Acceleration in world space.  Derived from local acceleration once per cycle

		// Inertia tensor for performing angular momentum/velocity derivation
		D3DXMATRIX					InertiaTensor;
		D3DXMATRIX					InverseInertiaTensor;

		// The distance sq that the object moved during this physics cycle
		float						DeltaMoveDistanceSq;	// m

	} PhysicsState;


	// Access to object mass is controlled
	CMPINLINE float							GetMass(void) const					{ return m_mass; }
	CMPINLINE float							GetInverseMass(void) const			{ return m_invmass; }
	void									SetMass(const float mass);

	// Methods to retrieve the (automatically-maintained) inverse orientation matrix
	CMPINLINE D3DXMATRIX *					GetInverseOrientationMatrix(void)	{ return &m_inverseorientationmatrix; }

	// Returns a flag indicating whether this object is a 'fast mover' that should be simulated via continuous (CCD) rather
	// than discrete collision detection.  Definition of a 'fast mover' is an object that will move more than a defined
	// percentage of its minimum extent in a single frame
	CMPINLINE bool							IsFastMover(void) const { return (PhysicsState.DeltaMoveDistanceSq > m_fastmoverthresholdsq); }

	// Returns a flag indicating whether this object is static, i.e. has no linear or angular momentum
	CMPINLINE bool							IsStatic(void) const { return (IsZeroVector(PhysicsState.WorldMomentum) && IsZeroVector(PhysicsState.AngularVelocity)); }

	// Methods to apply external forces to the object
	void									ApplyLocalForce(const D3DXVECTOR3 & localposition, D3DXVECTOR3 localforcevector);
	void									ApplyWorldSpaceForce(const D3DXVECTOR3 & worldposition, const D3DXVECTOR3 & worldforcevector);

	// Apply angular momentum to an object.  Calculates and applies the incremental angular velocity
	CMPINLINE void							ApplyAngularMomentum(const D3DXVECTOR3 & angular_momentum);

	// Apply angular velocity to an object.  Not scaled by the object intertia tensor
	CMPINLINE void							ApplyAngularVelocity(const D3DXVECTOR3 & angular_velocity);

	// Set the local momentum of the object, recalculating the object world momentum accordingly
	CMPINLINE void							SetLocalMomentum(const D3DXVECTOR3 & m)
	{
		// We are directly setting the local momentum, so do not need to transform into world space first
		PhysicsState.LocalMomentum = m;
		RecalculateWorldMomentum();
	}

	// Add local momentum to the object, recalculating the object world momentum accordingly
	CMPINLINE void							AddLocalMomentum(const D3DXVECTOR3 & dm)
	{
		// We first want to transform the local momentum delta into world space, and apply the increment in world space, 
		// to preserve the existing world momentum of this object
		D3DXVECTOR3 world_dm;
		D3DXVec3TransformCoord(&world_dm, &dm, &m_orientationmatrix);

		// Add the world momentum, which will automatically recalculate the new overall local momentum
		AddWorldMomentum(world_dm);
	}

	// Set the world momentum of the object, recalculating the object local momentum accordingly
	CMPINLINE void							SetWorldMomentum(const D3DXVECTOR3 & m)
	{
		PhysicsState.WorldMomentum = m;
		RecalculateLocalMomentum();
	}

	// Add world momentum to the object, recalculating the object local momentum accordingly
	CMPINLINE void							AddWorldMomentum(const D3DXVECTOR3 & dm)
	{
		PhysicsState.WorldMomentum += dm;
		RecalculateLocalMomentum();
	}

	// Recalculates the object local momentum based upon its current world momentum
	CMPINLINE void							RecalculateLocalMomentum(void)
	{
		D3DXVec3TransformCoord(&PhysicsState.LocalMomentum, &PhysicsState.WorldMomentum, &m_inverseorientationmatrix);
	}

	// Recalculates the object world momentum based upon its current local momentum.  This is rarely a good idea since it will not
	// preserve the existing world momentum.
	CMPINLINE void							RecalculateWorldMomentum(void)
	{
		D3DXVec3TransformCoord(&PhysicsState.WorldMomentum, &PhysicsState.LocalMomentum, &m_orientationmatrix);
	}

	// Method to recalculate the object inertia tensor.  Could be called whenever a contributing factor (mass/size) changes
	void									RecalculateInertiaTensor(void);

	// Applies a world-space linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	CMPINLINE void							ApplyWorldSpaceLinearForce(const D3DXVECTOR3 & worldforcevector)
	{
		// Apply the change in linear world momentum, accounting for mass (a = F/m).  Recalculates local momentum in the process
		AddWorldMomentum(worldforcevector * m_invmass);
	}

	// Apply a local linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	CMPINLINE void							ApplyLocalLinearForce(const D3DXVECTOR3 & localforcevector)
	{
		// Apply the change in linear local momentum, accounting for mass (a = F/m).  Recalculates world momentum in the process
		AddLocalMomentum(localforcevector * m_invmass);
	}

	// Applies a world-space linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	// Force is applied directly, i.e. it is not divided through by mass (this is assumed to have already happened)
	CMPINLINE void							ApplyWorldSpaceLinearForceDirect(const D3DXVECTOR3 & worldforcevector)
	{
		// Apply the change in linear world momentum.  Ignore object mass; is already accounted for
		// in F.  Recalculates local momentum in the process
		AddWorldMomentum(worldforcevector);
	}

	// Apply a local linear force to the object (no angular component is applied; the force applies equally on every point of the object)
	// Force is applied directly, i.e. it is not divided through by mass (this is assumed to have already happened)
	CMPINLINE void							ApplyLocalLinearForceDirect(const D3DXVECTOR3 & localforcevector)
	{
		// Apply the change in linear local momentum.  Ignore object mass; is already accounted for
		// in F.  Recalculates world momentum in the process
		AddLocalMomentum(localforcevector);
	}

protected:

	// Mass and precalculated inverse mass of the object
	float									m_mass, m_invmass;				

	// Inverse orientation matrix will be maintained at the iActiveObject level, since it is required for a number 
	// of physics operations.  Not maintained at the base iObject level since not required for the majority of static objects.
	// This represents the orientation of a space object in world space, or the LOCAL orientation of an environment object relative
	// to its environment
	D3DXMATRIX								m_inverseorientationmatrix;		// Inverse matrix, precalculated for efficiency
};


// Apply angular momentum to an object.  Calculates and applies the incremental angular velocity
CMPINLINE void iActiveObject::ApplyAngularMomentum(const D3DXVECTOR3 & angular_momentum)
{
	// Transform the desired angular momentum by the inverse inertia tensor to get the change in angular velocity
	D3DXVECTOR3 avec;
	D3DXVec3TransformCoord(&avec, &angular_momentum, &PhysicsState.InverseInertiaTensor);

	// Apply this change in angular velocity
	PhysicsState.AngularVelocity += avec;
}

// Apply angular velocity to an object.  Not scaled by the object intertia tensor
CMPINLINE void iActiveObject::ApplyAngularVelocity(const D3DXVECTOR3 & angular_velocity)
{
	// Simply apply this change in angular velocity directly
	PhysicsState.AngularVelocity += angular_velocity;
}






#endif
