#include "FastMath.h"
#include "BasicProjectileDefinition.h"

#include "BasicProjectile.h"

// Constructor to initialise all values
BasicProjectile::BasicProjectile(const BasicProjectileDefinition * definition, Game::ID_TYPE owner, const FXMVECTOR position,
	const FXMVECTOR orientation, unsigned int lifetime)
	:
	Definition(definition),
	Owner(owner),
	Position(position),
	Orientation(orientation),

	LaunchTime(Game::ClockMs),
	Expiration(Game::ClockMs + lifetime), 
	Speed(definition->Speed), 
	ProjectileBeamLengthMultiplier(definition->ProjectileBeamLengthMultiplier)
{ 
	// Determine the projectile velocity vector by transforming a (speed-scaled) heading vector by its orientation
	Velocity = XMVector3Rotate(XMVectorSetZ(NULL_VECTOR, Speed), Orientation);
}

// Constructor to initialise all values, including an initial velocity imparted by a moving parent object
BasicProjectile::BasicProjectile(const BasicProjectileDefinition * definition, Game::ID_TYPE owner, const FXMVECTOR position,
	const FXMVECTOR orientation, unsigned int lifetime, const FXMVECTOR base_world_velocity)
	:
	Definition(definition),
	Owner(owner),
	Position(position),
	Orientation(orientation),

	LaunchTime(Game::ClockMs),
	Expiration(Game::ClockMs + lifetime)
{
	
	// We also need to account for the initial (parent) velocity when defining our overall velocity vector
	Velocity = XMVectorAdd(base_world_velocity, XMVector3Rotate(XMVectorSetZ(NULL_VECTOR, Definition->Speed), Orientation));

	// The projectile is now not travelling at its intended speed, so determine the actual projectile speed
	Speed = XMVectorGetX(XMVector3LengthEst(Velocity));
	ProjectileBeamLengthMultiplier = (definition->ProjectileBeamLength / max(Speed, 1.0f));
}

// Generate a render instance for this projectile, using data from this instance and the projectile definition
void BasicProjectile::GenerateRenderInstance(RM_Instance & outInstance)
{
	// Volumetric line endpoints are embedded within the first two rows of the instance matrix 
	float beam_mult = (float)(Game::ClockMs - LaunchTime) * 0.001f;
	outInstance.World.r[0] = Position;
	outInstance.World.r[1] = XMVectorSubtract(Position, XMVectorScale(Velocity,
		min(beam_mult * Speed, 1.0f)));// *Definition->ProjectileBeamLength));

	// Line colour and alpha are stored within the third matrix row
	outInstance.World.r[2] = Definition->VolumetricLineData.Colour;

	// Additional parameters (e.g. beam radius) are stored within the instance parameter vector
	outInstance.Params = Definition->VolumetricLineData.Params;
}




