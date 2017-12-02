#pragma once

#include <cstdint>

class RenderTarget
{
public:

	enum class AttachmentPoint : uint8_t
	{
		Color0,         // Must be a uncompressed color format.
		Color1,         // Must be a uncompressed color format.
		Color2,         // Must be a uncompressed color format.
		Color3,         // Must be a uncompressed color format.
		Color4,         // Must be a uncompressed color format.
		Color5,         // Must be a uncompressed color format.
		Color6,         // Must be a uncompressed color format.
		Color7,         // Must be a uncompressed color format.
		Depth,          // Must be a texture with a depth format.
		DepthStencil,   // Must be a texture with a depth/stencil format.
		NumAttachmentPoints
	};


protected:


};