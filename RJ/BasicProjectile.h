#pragma once

#ifndef __BasicProjectileH__
#define __BasicProjectileH__

#include "DX11_Core.h"
class BasicProjectileDefinition;

struct BasicProjectile
{
public:

	// Fields are exposed publicly
	const BasicProjectileDefinition *						Definition;			// Pointer to projectile definition for this object
	Game::ID_TYPE											Owner;				// ID of the object which owns (i.e. fired) this projectile
	D3DXVECTOR3												Position;			// World position
	D3DXQUATERNION											Orientation;		// World orientation
	D3DXVECTOR3												Velocity;			// Position change /sec in world space
	unsigned int											Expiration;			// Clock ms time at which this projectile will expire
	float													Speed;				// Taken from definition; used to determine the extent for collision detection each frame

	// Default constructor
	BasicProjectile(void) { }

	// Constructor to initialise all values
	BasicProjectile(const BasicProjectileDefinition *definition, Game::ID_TYPE owner, const D3DXVECTOR3 & position, 
					const D3DXQUATERNION & orientation, unsigned int lifetime);
};




#endif




