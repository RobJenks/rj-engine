#pragma once

#include <string>
#include "DX11_Core.h"


class Texture
{
public:

	enum class Dimension
	{
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture3D,
		TextureCube,
		_COUNT
	};

	// The number of components used to create the texture.
	enum class Components
	{
		R,              // One red component.
		RG,             // Red, and green components.
		RGB,            // Red, green, and blue components.
		RGBA,           // Red, green, blue, and alpha components.
		Depth,          // Depth component.
		DepthStencil    // Depth and stencil in the same texture.
	};

	// The type of components in the texture.
	enum class Type
	{
		Typeless,           // Typeless formats.
							// TODO: sRGB type
		UnsignedNormalized, // Unsigned normalized (8, 10, or 16-bit unsigned integer values mapped to the range [0..1])
		SignedNormalized,   // Signed normalized (8, or 16-bit signed integer values mapped to the range [-1..1])
		Float,              // Floating point format (16, or 32-bit).
		UnsignedInteger,    // Unsigned integer format (8, 16, or 32-bit unsigned integer formats).
		SignedInteger,      // Signed integer format (8, 16, or 32-bit signed integer formats).
	};

	struct TextureFormat
	{
		Texture::Components Components;
		Texture::Type Type;

		// For multi-sample textures, we can specify how many samples we want 
		// to use for this texture. Valid values are usually in the range [1 .. 16]
		// depending on hardware support.
		// A value of 1 will effectively disable multisampling in the texture.
		uint8_t NumSamples;

		// Components should commonly be 8, 16, or 32-bits but some texture formats
		// support 1, 10, 11, 12, or 24-bits per component.
		uint8_t RedBits;
		uint8_t GreenBits;
		uint8_t BlueBits;
		uint8_t AlphaBits;
		uint8_t DepthBits;
		uint8_t StencilBits;

		// By default create a 4-component unsigned normalized texture with 8-bits per component and no multisampling.
		TextureFormat(Texture::Components components = Components::RGBA,
			Texture::Type type = Type::UnsignedNormalized,
			uint8_t numSamples = 1,
			uint8_t redBits = 8,
			uint8_t greenBits = 8,
			uint8_t blueBits = 8,
			uint8_t alphaBits = 8,
			uint8_t depthBits = 0,
			uint8_t stencilBits = 0)
			: Components(components)
			, Type(type)
			, NumSamples(numSamples)
			, RedBits(redBits)
			, GreenBits(greenBits)
			, BlueBits(blueBits)
			, AlphaBits(alphaBits)
			, DepthBits(depthBits)
			, StencilBits(stencilBits)
		{}

		// TODO: Define some commonly used texture formats.
	};

	// For cube maps, we may need to access a particular face of the cube map.
	enum class CubeFace
	{
		Right,  // +X
		Left,   // -X
		Top,    // +Y
		Bottom, // -Y
		Front,  // +Z
		Back,   // -Z
	};

	// Translate texture dimension to/from its string representation
	static std::string TranslateDimensionToString(Dimension dim);
	static Dimension TranslateDimensionFromString(const std::string & dim);




protected:




};