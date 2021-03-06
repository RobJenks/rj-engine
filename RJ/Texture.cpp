#include "Texture.h"
#include "Utility.h"


// Translate texture dimension to its string representation
std::string Texture::TranslateDimensionToString(Texture::Dimension dim)
{
	switch (dim)
	{
		case Texture::Dimension::Texture1D: return "Texture1D";
		case Texture::Dimension::Texture2D: return "Texture2D";
		case Texture::Dimension::Texture3D: return "Texture3D";
		case Texture::Dimension::TextureCube: return "TextureCube";
		default: 
			return "Unknown";
	}
}

// Translate texture dimension from its string representation
Texture::Dimension Texture::TranslateDimensionFromString(const std::string & dim)
{
	if (dim == "Texture1D") return Texture::Dimension::Texture1D;
	if (dim == "Texture2D") return Texture::Dimension::Texture2D;
	if (dim == "Texture3D") return Texture::Dimension::Texture3D;
	else if (dim == "TextureCube") return Texture::Dimension::TextureCube;
	else
		return Texture::Dimension::_COUNT;
}

