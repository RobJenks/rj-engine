#include "VolLineShader.h"
#include "BasicProjectileDefinition.h"


// Constructor
BasicProjectileDefinition::BasicProjectileDefinition(void)
{
	// Set default values
	SetProjectileSpeed(100.0f);
	SetProjectileBeamLength(5.0f);
	SetProjectileBeamRadius(2.0f);
	SetProjectileColour(ONE_VECTOR);
	SetProjectileLifetime(3000U);
	SetTexture(NULL);
}


// Set the projectile speed
void BasicProjectileDefinition::SetProjectileSpeed(float speed)
{
	// Store the new speed value
	Speed = max(speed, 1.0f);

	// Projectile beam length multiplier is dependent on projectile speed, so recalculate now
	ProjectileBeamLengthMultiplier = (ProjectileBeamLength / Speed);
}

/// Set the projectile beam length that trails the projectile point
void BasicProjectileDefinition::SetProjectileBeamLength(float beam)
{
	// Store the new desired beam length
	ProjectileBeamLength = max(beam, Game::C_EPSILON);

	// Projectile beam length multiplier is dependent on beam length, so recalculate now 
	ProjectileBeamLengthMultiplier = (ProjectileBeamLength / max(Speed, 1.0f));
}

// Sets the volumetric rendering data for this projectile type; will recalculate all derived rendering data
void BasicProjectileDefinition::GenerateProjectileRenderingData(void)
{
	// Retrieve or create a model buffer for rendering of this line data
	Buffer = VolLineShader::LineModel(VolumetricLineData.RenderTexture);
}

