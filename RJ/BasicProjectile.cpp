#include "FastMath.h"
#include "BasicProjectileDefinition.h"

#include "BasicProjectile.h"

// Constructor to initialise all values
BasicProjectile::BasicProjectile(	const BasicProjectileDefinition *definition, Game::ID_TYPE owner, const FXMVECTOR position, 
									const FXMVECTOR orientation, unsigned int lifetime)
:
	Definition(definition), 
	Owner(owner),
	Position(position), 
	Orientation(orientation), 

	Expiration(Game::ClockMs + lifetime), 
	Speed(definition->Speed)
{ 
	// Determine the projectile velocity vector by transforming a (speed-scaled) heading vector by its orientation
	Velocity = XMVector3Rotate(XMVectorSetZ(NULL_VECTOR, Speed), Orientation);
}




