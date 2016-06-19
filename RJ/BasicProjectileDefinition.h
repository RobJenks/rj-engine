#pragma once

#ifndef __BasicProjectileDefinitionH__
#define __BasicProjectileDefinitionH__

#include "ErrorCodes.h"
#include "VolumetricLine.h"
#include "Damage.h"
class Texture;
class ModelBuffer;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class BasicProjectileDefinition : public ALIGN16<BasicProjectileDefinition>
{
public:

	// Constructor
	BasicProjectileDefinition(void);

	// Key projectile parameters
	float									Speed;								// Projectile speed /sec
	float									ProjectileBeamLength;				// Extent of the projectile beam that trails the projectile point
	float									ProjectileBeamLengthMultiplier;		// The scaling factor to be applied to projectile velocity to generate the beam
	unsigned int							Lifetime;							// Projectile lifetime (ms) before it expires and is removed

	// Get or set the unique string code for this projectile definition
	CMPINLINE std::string					GetCode(void) const								{ return m_code; }
	CMPINLINE void							SetCode(const std::string & code)				{ m_code = code; }

	// Get or set the unique string code for this projectile definition
	CMPINLINE std::string					GetName(void) const								{ return m_name; }
	CMPINLINE void							SetName(const std::string & code)				{ m_name = code; }

	// Set the projectile speed
	void									SetProjectileSpeed(float speed);

	/// Set the projectile beam length that trails the projectile point
	void									SetProjectileBeamLength(float beam);

	// Set or retrieve the projectile colour
	CMPINLINE XMFLOAT4						GetProjectileColour(void) const					{ return VolumetricLineData.Colour; }
	CMPINLINE void							SetProjectileColour(const XMFLOAT4 & colour)	{ VolumetricLineData.Colour = colour; }

	// Set or retrieve the projectile beam radius
	CMPINLINE float							GetProjectileBeamRadius(void) const				{ return VolumetricLineData.GetLineRadius(); }
	CMPINLINE void							SetProjectileBeamRadius(float radius)			{ VolumetricLineData.SetLineRadius(radius); }

	// Add a new form of damage applied by this projectile
	CMPINLINE void							AddDamageType(const Damage & damage)			{ m_damage.push_back(damage); }

	// Remove all types of damage currently performed by this projectile type
	CMPINLINE void							RemoveAllDamageTypes(void)						{ m_damage.clear(); }

	// Returns a reference to the damage imparted by this projectile type
	CMPINLINE const DamageSet &				GetProjectileDamage(void) const					{ return m_damage; }

	// Generates the volumetric rendering data for this projectile type; will recalculate all derived rendering data
	// Must be called after all definition properties are set
	void									GenerateProjectileRenderingData(void);

	// Cached rendering data that can be used for efficient rendering of large projectile sets; generated via "GenerateProjectileRenderingData"
	ModelBuffer *							Buffer;

	// Return or store a reference to the texture for this projectile type
	CMPINLINE Texture *						GetTexture(void) const			{ return VolumetricLineData.RenderTexture; }
	Result									SetTexture(Texture *texture);
	Result									SetTexture(const std::string & filename);

	// Return or store the default projectile lifetime 
	CMPINLINE unsigned int					GetProjectileLifetime(void) const				{ return Lifetime; }
	CMPINLINE void							SetProjectileLifetime(unsigned int lifetime)	{ Lifetime = lifetime; }

	// Volumetric line data for this projectile type
	VolumetricLine							VolumetricLineData;		

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Destructor
	~BasicProjectileDefinition(void);

protected:

	std::string								m_code;
	std::string								m_name;

	DamageSet								m_damage;

};




#endif




