#ifndef __MaterialDataHLSLH__
#define __MaterialDataHLSLH__

#include "CppHLSLLocalisation.hlsl.h"


// Standard material definition
struct MaterialData
{
	float4					GlobalAmbient;						// Global ambient contribution for the scene.  TODO: could become a global in future
	//------------------------------------------ ( 16 bytes )
	float4					AmbientColor;						// Simulates effect of global lighting contribution, e.g. illumination from the sun
	//------------------------------------------ ( 16 bytes )
	float4					EmissiveColor;						// Light emitted by the object itself
	//------------------------------------------ ( 16 bytes )
	float4					DiffuseColor;						// The inherent object colour that is reflected evenly in all directions
	//------------------------------------------ ( 16 bytes )
	float4					SpecularColor;						// Colour reflected by shiny object back along the viewing direction
	//------------------------------------------ ( 16 bytes )
	float4					Reflectance;						// The amount of reflected environment colour that should be blended with diffuse.  For environment mapping; not currently used
	//------------------------------------------ ( 16 bytes )
	float					Opacity;							// Overall object opacity.  Any object with opacity < 1.0 is considered transparent
	float					SpecularPower;						// Normalised specular power term; determines degree of shininess for these objects.  Range [0.0 1.0]
	float					SpecularScale;						// Value used to scale all specular power values, since power <= 1.0 does not make sense
	float					IndexOfRefraction;					// Degree of refraction for light passing through this object.  For transparent objects only.  For env mapping; not currently used
	//------------------------------------------ ( 16 bytes )
	bool					HasAmbientTexture;					// Indicates whether this material will bind the specified texture resource to the shader
	bool					HasEmissiveTexture;					// Indicates whether this material will bind the specified texture resource to the shader
	bool					HasDiffuseTexture;					// Indicates whether this material will bind the specified texture resource to the shader
	bool					HasSpecularTexture;					// Indicates whether this material will bind the specified texture resource to the shader
	//------------------------------------------ ( 16 bytes )
	bool					HasSpecularPowerTexture;			// Indicates whether this material will bind the specified texture resource to the shader
	bool					HasNormalTexture;					// Indicates whether this material will bind the specified texture resource to the shader
	bool					HasBumpTexture;						// Indicates whether this material will bind the specified texture resource to the shader
	bool					HasOpacityTexture;					// Indicates whether this material will bind the specified texture resource to the shader
	//------------------------------------------ ( 16 bytes )
	float					BumpIntensity;						// Scale factor to apply to all height values from the material bump map
	float					AlphaThreshold;						// Opacity threshold below which pixels will be discarded without shading
	float2					_padding;							// To maintain structure alignment & packing
	//------------------------------------------ ( 16 bytes )
	//------------------------------------------ ( 16 * 10 = 160 bytes)


	// Default constructor for use within CPP classes
#ifdef __cplusplus
	inline MaterialData(void)
		: GlobalAmbient(0.1f, 0.1f, 0.15f, 1.0f)
		, AmbientColor(0.0f, 0.0f, 0.0f, 1.0f)
		, EmissiveColor(0.0f, 0.0f, 0.0f, 1.0f)
		, DiffuseColor(1.0f, 1.0f, 1.0f, 1.0f)
		, SpecularColor(0.0f, 0.0f, 0.0f, 1.0f)
		, Reflectance(0.0f, 0.0f, 0.0f, 0)
		, Opacity(1.0f)
		, SpecularPower(-1.0f)
		, SpecularScale(128.0f)
		, IndexOfRefraction(-1.0f)
		, HasAmbientTexture(false)
		, HasEmissiveTexture(false)
		, HasDiffuseTexture(false)
		, HasSpecularTexture(false)
		, HasSpecularPowerTexture(false)
		, HasNormalTexture(false)
		, HasBumpTexture(false)
		, HasOpacityTexture(false)
		, BumpIntensity(5.0f)
		, AlphaThreshold(0.1f)
	{ }
#endif
};


#endif 