#include "Texture.h"
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

// Set the texture for this projectile type from an external texture resource
Result BasicProjectileDefinition::SetTexture(const std::string & filename)
{
	// Parameter check
	if (filename == NullString) return ErrorCodes::CouldNotInitialiseBasicProjectileTexture;

	// Attempt to initialise a new texture object from the specified file
	Texture *tex = new Texture();
	Result result = tex->Initialise(filename);

	// We won't store the resulting texture if an error occured during initialisation
	if (result != ErrorCodes::NoError)
	{
		SafeDelete(tex);
		return ErrorCodes::CouldNotInitialiseBasicProjectileTexture;
	}
	
	// Pass control to the overloaded method and return the result of storing this texture
	return SetTexture(tex);
}

Result BasicProjectileDefinition::SetTexture(Texture *texture)
{
	// If a texture already exists then deallocate it first
	if (VolumetricLineData.RenderTexture)
	{
		SafeDelete(VolumetricLineData.RenderTexture);
	}

	// Simply store the texture reference and return success
	VolumetricLineData.RenderTexture = texture; 
	return ErrorCodes::NoError;
}

