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

	CMPINLINE static AttachmentPoint TranslateAttachmentPointFromString(const std::string & attach)
	{
		if (attach == "Color0") return AttachmentPoint::Color0;
		else if (attach == "Color1") return AttachmentPoint::Color1;
		else if (attach == "Color2") return AttachmentPoint::Color2;
		else if (attach == "Color3") return AttachmentPoint::Color3;
		else if (attach == "Color4") return AttachmentPoint::Color4;
		else if (attach == "Color5") return AttachmentPoint::Color5;
		else if (attach == "Color6") return AttachmentPoint::Color6;
		else if (attach == "Color7") return AttachmentPoint::Color7;
		else if (attach == "Depth") return AttachmentPoint::Depth;
		else if (attach == "DepthStencil") return AttachmentPoint::DepthStencil;
		
		else return AttachmentPoint::NumAttachmentPoints;
	}

	CMPINLINE static std::string TranslateAttachmentPointToString(AttachmentPoint attach)
	{
		switch (attach)
		{
			case AttachmentPoint::Color0: return "Color0";
			case AttachmentPoint::Color1: return "Color1";
			case AttachmentPoint::Color2: return "Color2";
			case AttachmentPoint::Color3: return "Color3";
			case AttachmentPoint::Color4: return "Color4";
			case AttachmentPoint::Color5: return "Color5";
			case AttachmentPoint::Color6: return "Color6";
			case AttachmentPoint::Color7: return "Color7";
			case AttachmentPoint::Depth: return "Depth";
			case AttachmentPoint::DepthStencil: return "DepthStencil";

			default: return "Unknown";
		}
	}

protected:


};