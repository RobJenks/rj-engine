#include "FastMath.h"
#include "GameVarsExtern.h"
#include "BasicProjectileDefinition.h"
#include "CoreEngine.h"
#include "LightingManagerObject.h"

#include "BasicProjectile.h"

// Constructor to initialise all values
BasicProjectile::BasicProjectile(const BasicProjectileDefinition * definition, Game::ID_TYPE owner, const FXMVECTOR position,
	const FXMVECTOR orientation)
	:
	Definition(definition),
	Owner(owner),
	Position(position),
	Orientation(orientation),

	LaunchTime(Game::ClockMs),
	Expiration(Game::ClockMs + definition->Lifetime), 
	Speed(definition->Speed), 
	ProjectileBeamLengthMultiplier(definition->ProjectileBeamLengthMultiplier)
{ 
	// Determine the projectile velocity vector by transforming a (speed-scaled) heading vector by its orientation
	Velocity = XMVector3Rotate(XMVectorSetZ(NULL_VECTOR, Speed), Orientation);
}

// Constructor to initialise all values, including an initial velocity imparted by a moving parent object
BasicProjectile::BasicProjectile(const BasicProjectileDefinition * definition, Game::ID_TYPE owner, const FXMVECTOR position,
	const FXMVECTOR orientation, const FXMVECTOR base_world_velocity)
	:
	Definition(definition),
	Owner(owner),
	Position(position),
	Orientation(orientation),

	LaunchTime(Game::ClockMs),
	Expiration(Game::ClockMs + definition->Lifetime)
{
	
	// We also need to account for the initial (parent) velocity when defining our overall velocity vector
	Velocity = XMVectorAdd(base_world_velocity, XMVector3Rotate(XMVectorSetZ(NULL_VECTOR, Definition->Speed), Orientation));

	// The projectile is now not travelling at its intended speed, so determine the actual projectile speed
	Speed = XMVectorGetX(XMVector3LengthEst(Velocity));
	ProjectileBeamLengthMultiplier = (definition->ProjectileBeamLength / max(Speed, 1.0f));
}

// Generate a render instance for this projectile, using data from this instance and the projectile definition
RM_Instance BasicProjectile::GenerateRenderInstance(void)
{
	// Volumetric line endpoints are embedded within the first two rows of the instance matrix 
	// P1 will be scaled so that the beam grows from zero to its full length during the 
	// first 'Speed' seconds after launch
	float beam_mult = (float)(Game::ClockMs - LaunchTime) * 0.001f;

	// Get relevant data on the volumetric line
	XMFLOAT4 start, end;
	XMStoreFloat4(&start, Position);
	XMStoreFloat4(&end, XMVectorSubtract(Position, XMVectorScale(Velocity,
						min(beam_mult, ProjectileBeamLengthMultiplier))));
	const XMFLOAT4 &col = Definition->VolumetricLineData.Colour;

 	// Store all data within the instance matrix; start pos in row 1, end pos in row 2, line colour and alpha in row 3, row4 = unused
	// Additional parameters (e.g. beam radius) are stored within the instance parameter vector
	return RM_Instance(XMFLOAT4X4(start.x, start.y, start.z, 1.0f, end.x, end.y, end.z, 1.0f, col.x, col.y, col.z, col.w, 0.0f, 0.0f, 0.0f, 0.0f),
		RM_Instance::SORT_KEY_RENDER_LAST, Definition->VolumetricLineData.Params);
}




