#include "TextureDX11.h"
#include "MaterialDX11.h"
#include "VolLineShader.h"
#include "BasicProjectileDefinition.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RenderDeviceDX11.h"


// Constructor
BasicProjectileDefinition::BasicProjectileDefinition(void)
{
	// Set default values
	SetProjectileSpeed(100.0f);
	SetProjectileBeamLength(5.0f);
	SetProjectileBeamRadius(2.0f);
	SetProjectileColour(ONE_FLOAT4);
	SetProjectileLifetime(3000U);
	SetMaterial(NULL);
	SetLaunchAudio(AudioParameters::Null);
}


// Set the projectile speed
void BasicProjectileDefinition::SetProjectileSpeed(float speed)
{
	// Store the new speed value
	Speed = max(speed, 1.0f);

	// Projectile beam length multiplier is dependent on projectile speed, so recalculate now
	ProjectileBeamLengthMultiplier = (ProjectileBeamLength / Speed);
}

// Set the projectile beam length that trails the projectile point
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
	Buffer = VolLineShader::LineModel(VolumetricLineData.RenderMaterial);
}

// Set the material for this projectile type 
Result BasicProjectileDefinition::SetMaterial(const std::string & name)
{
	// Parameter check
	if (name.empty()) return ErrorCodes::CouldNotInitialiseBasicProjectileTexture;

	// Attempt to initialise a new texture object from the specified file
	MaterialDX11 *material = Game::Engine->GetRenderDevice()->Assets.GetMaterial(name);
	
	// We won't store the resulting texture if an error occured during initialisation
	if (material == NULL)
	{
		Game::Log << LOG_WARN << "Attempted to assign invalid material \"" << name << "\" to basic projectile definition \"" << m_code << "\"\n";
	}
	
	// Pass control to the overloaded method and return the result of storing this texture
	return SetMaterial(material);
}

const TextureDX11 * BasicProjectileDefinition::GetTexture(void) const 
{
	return (VolumetricLineData.RenderMaterial ? VolumetricLineData.RenderMaterial->GetTexture(Material::TextureType::Diffuse) : NULL);
}

Result BasicProjectileDefinition::SetMaterial(MaterialDX11 *material)
{
	if (material != NULL)
	{
		VolumetricLineData.RenderMaterial = material;
	}

	return ErrorCodes::NoError;
}


// Destructor
BasicProjectileDefinition::~BasicProjectileDefinition(void)
{
}


