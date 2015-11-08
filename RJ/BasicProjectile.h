#pragma once

#ifndef __BasicProjectileH__
#define __BasicProjectileH__

#include "DX11_Core.h"
class BasicProjectileDefinition;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct BasicProjectile : public ALIGN16<BasicProjectile>
{
public:

	// Fields are exposed publicly
	AXMVECTOR												Position;			// World position
	const BasicProjectileDefinition *						Definition;			// Pointer to projectile definition for this object
	Game::ID_TYPE											Owner;				// ID of the object which owns (i.e. fired) this projectile
	AXMVECTOR												Orientation;		// World orientation
	unsigned int											Expiration;			// Clock ms time at which this projectile will expire
	float													Speed;				// Taken from definition; used to determine the extent for collision detection each frame
	AXMVECTOR												Velocity;			// Position change /sec in world space

	// Default constructor
	BasicProjectile(void) { }

	// Constructor to initialise all values
	BasicProjectile(const BasicProjectileDefinition *definition, Game::ID_TYPE owner, const FXMVECTOR position, 
					const FXMVECTOR orientation, unsigned int lifetime);
};




#endif




