#pragma once

#ifndef __BasicProjectileDefinitionH__
#define __BasicProjectileDefinitionH__

#include "VolumetricLine.h"
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

	// Set the projectile speed
	void									SetProjectileSpeed(float speed);

	/// Set the projectile beam length that trails the projectile point
	void									SetProjectileBeamLength(float beam);

	// Set or retrieve the projectile colour
	CMPINLINE XMVECTOR						GetProjectileColour(void) const					{ return VolumetricLineData.Colour; }
	CMPINLINE void							SetProjectileColour(const FXMVECTOR colour)		{ VolumetricLineData.Colour = colour; }

	// Set or retrieve the projectile beam radius
	CMPINLINE float							GetProjectileBeamRadius(void) const				{ return VolumetricLineData.GetLineRadius(); }
	CMPINLINE void							SetProjectileBeamRadius(float radius)			{ VolumetricLineData.SetLineRadius(radius); }

	// Generates the volumetric rendering data for this projectile type; will recalculate all derived rendering data
	// Must be called after all definition properties are set
	void									GenerateProjectileRenderingData(void);

	// Cached rendering data that can be used for efficient rendering of large projectile sets; generated via "GenerateProjectileRenderingData"
	ModelBuffer *							Buffer;

	// Return or store a reference to the texture for this projectile type
	CMPINLINE Texture *						GetTexture(void) const			{ return VolumetricLineData.RenderTexture; }
	CMPINLINE void							SetTexture(Texture *texture)	{ VolumetricLineData.RenderTexture = texture; }

	// Volumetric line data for this projectile type
	VolumetricLine							VolumetricLineData;			

protected:





};




#endif




