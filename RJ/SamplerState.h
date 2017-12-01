#pragma once

class SamplerState
{
public:

	enum class MinFilter
	{
		MinNearest,                // The nearest texel to the sampled texel.
		MinLinear,                 // Linear average of the 4 closest texels.
	};

	enum class MagFilter
	{
		MagNearest,                // The nearest texel to the sampled texel.
		MagLinear,                 // Weighted average of the closest texels.
	};

	enum class MipFilter
	{
		MipNearest,                // Choose the nearest mip level.
		MipLinear,                 // Linear interpolate between the 2 nearest mip map levels.
	};

	enum class WrapMode
	{
		Repeat,                 // Texture is repeated when texture coordinates are out of range.
		Mirror,                 // Texture is mirrored when texture coordinates are out of range.
		Clamp,                  // Texture coordinate is clamped to [0, 1] 
		Border,                 // Texture border color is used when texture coordinates are out of range.
	};


	enum class CompareMode
	{
		None,                   // Don't perform any comparison
		CompareRefToTexture,    // Compare the reference value (usually the currently bound depth buffer) to the value in the texture.
	};

	enum class CompareFunc
	{
		Never,                  // Never pass the comparison function.
		Less,                   // Pass if the source data is less than the destination data.
		Equal,                  // Pass if the source data is equal to the destination data.
		LessEqual,              // Pass if the source data is less than or equal to the destination data.
		Greater,                // Pass if the source data is greater than the destination data.
		NotEqual,               // Pass if the source data is not equal to the destination data.
		GreaterEqual,           // Pass if the source data is greater than or equal to the destination data.
		Always,                 // Always pass the comparison function.
	};



protected:



};