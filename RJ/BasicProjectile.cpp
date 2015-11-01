#include "FastMath.h"
#include "BasicProjectileDefinition.h"

#include "BasicProjectile.h"

// Constructor to initialise all values
BasicProjectile::BasicProjectile(	const BasicProjectileDefinition *definition, Game::ID_TYPE owner, const D3DXVECTOR3 & position, 
									const D3DXQUATERNION & orientation, unsigned int lifetime)
:
	Definition(definition), 
	Owner(owner),
	Position(position), 
	Orientation(orientation), 

	Expiration(Game::ClockMs + lifetime), 
	Speed(definition->Speed)
{ 
	// Determine the projectile velocity vector by transforming a heading vector by its orientation
	Velocity = D3DXVECTOR3(0.0f, 0.0f, Speed);
	XMVector3Rotate(&Velocity, &Velocity, &Orientation);
}




