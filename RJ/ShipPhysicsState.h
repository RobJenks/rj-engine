#ifndef __ShipPhysicsStateH__
#define __ShipPhysicsStateH__

#include "DX11_Core.h"


#include "CompilerSettings.h"

class ShipPhysicsState
{
public:
	ShipPhysicsState(void);

	// Primary values, i.e. those maintained directly
	D3DXVECTOR3					Acceleration;			// m/s/s			Derived from sum of all engine thrust & mass
	D3DXVECTOR3					WorldMomentum;			// m/s				Directional vector momentum in world space
	D3DXVECTOR3					Heading;				// Vector3			Ship heading, derived from orientation * the basis vector
	D3DXVECTOR3					AngularVelocity;		// rad/sec			Angular velocity of the ship

	// Secondary values, i.e. those derived from the primary values
	D3DXVECTOR3					LocalMomentum;			// m/s				Current directional vector momentum.  Derived from world momentum once per cycle
	D3DXVECTOR3					WorldAcceleration;		// m/s/s			Acceleration in world space.  Derived from local acceleration once per cycle

	// Access to mass and (derived) inverse mass data is controlled
	CMPINLINE float				GetMass(void) const					{ return m_mass; }
	CMPINLINE float				GetInverseMass(void) const			{ return m_invmass; }
	void						SetMass(const float mass);

	// Inertia tensor for performing angular momentum/velocity derivation
	D3DXMATRIX					InertiaTensor;

		
	

private:

	// Object mass & its inverse is stored and any changes are used to recalcualte derived fields
	float						m_mass, m_invmass;		

	// Method to recalculate the object inertia tensor.  Called whenever a contributing factor changes
	void						RecalculateInertiaTensor(void);

};


#endif
