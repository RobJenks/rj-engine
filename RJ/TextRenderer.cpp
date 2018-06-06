#include "TextRenderer.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "DecalRenderingManager.h"

// Default font size; character scaling will use this as the baseline
const float TextRenderer::DEFAULT_FONT_SIZE = 12.0f;
const float TextRenderer::DEFAULT_FONT_SIZE_RECIP = (1.0f / TextRenderer::DEFAULT_FONT_SIZE);
const float TextRenderer::DEFAULT_FONT_SIZE_HEIGHT_PX = 16.0f;

// Constructor
TextRenderer::TextRenderer(void)
	:
	m_font_count(0U),
	m_null_font(new Font("null"))
{
}

// Register a new font.  Returns a flag indicating whether the font could be registered
bool TextRenderer::RegisterFont(Font *font)
{
	// Sanity check
	if (!font)
	{
		Game::Log << LOG_WARN << "Cannot register null font\n";
		return false;
	}

	// We cannot register the font if one already exists with this name
	if (IsValidFont(font->GetCode()))
	{
		Game::Log << LOG_WARN << "Cannot register new font \"" << font->GetCode() << "\"; code is invalid or already registered\n";
		return false;
	}

	// The font is valid and can be registered.  Add it to both collections
	auto id = m_fonts.size();
	m_fonts.push_back(font);
	m_font_ids[font->GetCode()] = id;

	// Update our cached record of the font collection size
	m_font_count = m_fonts.size();

	// Return success
	return true;
}

// Retrieve a font based on its ID
const Font & TextRenderer::GetFont(Font::ID id) const
{
	if (!IsValidFont(id)) return *m_null_font;

	return *m_fonts[id];
}

// Get the ID of a font with the given name.  Returns 0 for the default font if no such
// font exists
Font::ID TextRenderer::GetFontID(const std::string & code) const
{
	const auto it = m_font_ids.find(code);
	if (it == m_font_ids.end()) return Font::DEFAULT_FONT_ID;

	return it->second;
}

// Get a font based on its string  name
const Font & TextRenderer::GetFont(const std::string & code) const
{
	const auto it = m_font_ids.find(code);
	if (it == m_font_ids.end()) return *m_null_font;

	return *m_fonts[it->second];
}

// Indicates whether the given identifier represents a valid registered font
bool TextRenderer::IsValidFont(Font::ID id) const
{
	return (id < m_font_count);
}

// Indicates whether the given identifier represents a valid registered font
bool TextRenderer::IsValidFont(const std::string & code) const
{
	return (m_font_ids.find(code) != m_font_ids.end());
}

// Returns the glyph scaling factor for the given font size
float TextRenderer::GlyphScalingFactor(const Font & font, float font_size)
{
	return ((font_size * TextRenderer::DEFAULT_FONT_SIZE_RECIP)		// (fontsize * default_size_recip) == 1.0 when fontsize == the default size
					  * font.GetGlyphScalingFactor());				// Scaling factor to convert unscaled glyph to 'default' size
}

// Renders the given character to the screen
void TextRenderer::RenderCharacter(	unsigned int ch, Font::ID font, DecalRenderingMode mode, XMVECTOR location, float font_size,
									const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, TextAnchorPoint anchorpoint, float outlineFactor, 
									float smoothingFactor, const FXMVECTOR orientation) const
{
	// Get the corresponding font and set decal rendering parameters accordingly
	const auto & font_data = GetFont(font);
	SetDecalRenderingParameters(font_data, basecolour, outlinecolour, outlineFactor, smoothingFactor);
	
	// Glyph scaling can be determined based on desired font size
	const auto & glyph = font_data.GetGlyph(ch);
	float glyph_scale = GlyphScalingFactor(font_data, font_size);

	// Account for alternative anchor points (default == centre in this case, since individual decals are rendered from their centre point)
	if (anchorpoint == TextAnchorPoint::TopLeft)
	{
		location = XMVectorAdd(location, 
			XMVectorSet(static_cast<float>(glyph.Size.x) * glyph_scale * 0.5f, static_cast<float>(glyph.Size.y) * glyph_scale * 0.5f, 0.0f, 0.0f));
	}

	// Push a request to the decal renderer
	RenderGlyphDecal(glyph, DecalRenderingMode::ScreenSpace, location, glyph_scale, orientation);
}

// Renders the given text string to the screen.  No wrapping is performed
void TextRenderer::RenderString(const std::string & str, Font::ID font, DecalRenderingMode mode, XMVECTOR location, float font_size,
										const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, TextAnchorPoint anchorpoint, 
										float outlineFactor, float smoothingFactor) const
{
	// Make sure the string is non-empty
	if (str.empty()) return;

	// Get the corresponding font and set decal rendering parameters accordingly
	const auto & font_data = GetFont(font);
	SetDecalRenderingParameters(font_data, basecolour, outlinecolour, outlineFactor, smoothingFactor);

	// Glyph scaling can be determined based on desired font size
	float glyph_scale = GlyphScalingFactor(font_data, font_size);
	float separation = font_data.GetCharacterSeparation();

	// Account for alternative anchor points (default == top-left)
	if (anchorpoint == TextAnchorPoint::Centre)
	{
		XMFLOAT2 dimensions = CalculateTextDimensions(str, font, font_size);
		location = XMVectorSubtract(location, XMVectorSet(dimensions.x * 0.5f, dimensions.y * 0.5f, 0.0f, 0.0f));
	}

	// String starting point should be the top-left - not centre - of the first glyph so add a delta.  We know the string is non-empty so this is safe
	const auto & first_glyph = font_data.GetGlyph(str[0]);
	location = XMVectorAdd(location,
		XMVectorSet(static_cast<float>(first_glyph.Size.x) * glyph_scale, static_cast<float>(first_glyph.Size.y) * glyph_scale, 0.0f, 0.0f));

	// Push consecutive requests to render each glyph in turn
	float xpos = 0.0f;
	for (unsigned int ch : str)
	{
		const auto & glyph = font_data.GetGlyph(ch);

		// Determine a render position for this glyph
		XMVECTOR delta = XMVectorSetX(NULL_VECTOR, xpos + (static_cast<float>(glyph.Size.x) * glyph_scale * 0.5f));
		RenderGlyphDecal(glyph, mode, XMVectorAdd(location, delta), glyph_scale);

		// Move along the string
		xpos += ((glyph.Size.x + separation) * glyph_scale);
	}
}

// Renders the given text string to the screen.  No wrapping is performed.  Allows orientation of 
// text blocks calculated with respect to the centre point of the string
void TextRenderer::RenderString(const std::string & str, Font::ID font, DecalRenderingMode mode, XMVECTOR location, float font_size, const FXMVECTOR orientation,
										const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, TextAnchorPoint anchorpoint,
										TextAnchorPoint rotationpoint, float outlineFactor, float smoothingFactor) const
{
	// Make sure the string is non-empty
	if (str.empty()) return;

	// Get the corresponding font and set decal rendering parameters accordingly
	const auto & font_data = GetFont(font);
	SetDecalRenderingParameters(font_data, basecolour, outlinecolour, outlineFactor, smoothingFactor);

	// Glyph scaling can be determined based on desired font size
	float glyph_scale = GlyphScalingFactor(font_data, font_size);
	float separation = font_data.GetCharacterSeparation();

	// Calculate overall text bounds so we can identify the centre point of the rendered string
	XMFLOAT2 dimensions = CalculateTextDimensions(str, font, font_size);
	XMVECTOR half_dim = XMVectorSet(dimensions.x * 0.5f, dimensions.y * 0.5f, 0.0f, 0.0f);

	// Account for alternative text anchor point if required (default == top-left)
	if (anchorpoint == TextAnchorPoint::Centre)
	{
		location = XMVectorSubtract(location, half_dim);
	}

	// String rotation quaternion differs between screen- and world-space rendering, likely due to +z/-z coord system in use
	XMVECTOR invorient = XMQuaternionInverse(orientation);
	XMMATRIX string_rotation_transform = XMMatrixRotationQuaternion(mode == DecalRenderingMode::ScreenSpace ? orientation : invorient);

	// Derive transforms required to transform all characters of the string in a consistent rotation
	if (rotationpoint == TextAnchorPoint::Centre)
	{
		string_rotation_transform = XMMatrixMultiply(XMMatrixMultiply(		// Adjust existing (top-left) rotation transformation by:
			XMMatrixTranslationFromVector(XMVectorNegate(half_dim)),		// 1. Translating target point to string centre point
			string_rotation_transform),										// 2. Applying the original rotation
			XMMatrixTranslationFromVector(half_dim)							// 3. Translating target point back by same translation, in rotated reference frame
		);
	}

	// String starting point should be the top-left - not centre - of the first glyph so add a delta.  We know the string is non-empty so this is safe
	const auto & first_glyph = font_data.GetGlyph(str[0]);
	location = XMVectorAdd(location,
		XMVectorSet(static_cast<float>(first_glyph.Size.x) * glyph_scale, static_cast<float>(first_glyph.Size.y) * glyph_scale, 0.0f, 0.0f));

	// Push consecutive requests to render each glyph in turn
	float xpos = 0.0f;
	for (unsigned int ch : str)
	{
		const auto & glyph = font_data.GetGlyph(ch);

		// Determine a render position based upon the given rotation transform
		XMVECTOR delta = XMVector3TransformCoord(XMVectorSetX(NULL_VECTOR, xpos + (static_cast<float>(glyph.Size.x) * glyph_scale * 0.5f)), string_rotation_transform);
		RenderGlyphDecal(glyph, mode, XMVectorAdd(location, delta), glyph_scale, invorient);

		// Move along the string
		xpos += ((glyph.Size.x + separation) * glyph_scale);
	}
}


// Pass parameters to the decal renderer that will be used for all subsequent text rendering calls
void TextRenderer::SetDecalRenderingParameters(const Font & font, const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, 
											   float outlineWidth, float smoothingFactor) const
{
	auto * renderer = Game::Engine->GetDecalRenderer();

	renderer->SetTexture(font.GetTextureMap());
	renderer->SetBaseColour(basecolour);
	renderer->SetOutlineColour(outlinecolour);
	renderer->SetOutlineWidthFactor(outlineWidth);
	renderer->SetSmoothingFactor(smoothingFactor);
}

// Perform glyph calculation and dispatch a render call to the decal renderer
void TextRenderer::RenderGlyphDecal(const FontGlyph & glyph, DecalRenderingMode mode, const FXMVECTOR location, float glyph_scale, const FXMVECTOR orientation) const
{
	XMFLOAT2 size(static_cast<float>(glyph.Size.x) * glyph_scale, static_cast<float>(glyph.Size.y) * glyph_scale);

	Game::Engine->GetDecalRenderer()->RenderDecal(mode, location, size, glyph.Location, (glyph.Location + glyph.Size), orientation);
}

// Calculates the dimensions of a text string with the given properties
XMFLOAT2 TextRenderer::CalculateTextDimensions(const std::string & text, Font::ID font, float font_size) const
{
	XMFLOAT2 dimensions(0.0f, 0.0f);

	// Retrieve font details
	const Font & font_data = GetFont(font);
	float separation = font_data.GetCharacterSeparation();

	// Determine a glyph scaling factor based on this font size
	float scalefactor = GlyphScalingFactor(font_data, font_size);

	// Determine size of each glyph and separators in turn
	for (auto ch : text)
	{
		const auto & size = font_data.GetGlyph((unsigned int)ch).Size;

		dimensions.x += (scalefactor * (size.x + separation));
		dimensions.y = max(dimensions.y, (size.y * scalefactor));
	}

	// Return overall dimensions
	return dimensions;
}


// Shutdown and release all owned resources
void TextRenderer::Shutdown(void)
{
	// Text renderer owns all font data, so deallocate all fonts now
	for (size_t i = 0U; i < m_fonts.size(); ++i)
	{
		if (m_fonts[i]) SafeDelete(m_fonts[i]);
	}

	// Clear all font data collections given that they now contain junk
	m_fonts.clear();
	m_font_ids.clear();
	m_font_count = 0U;
}

// Destructor
TextRenderer::~TextRenderer(void)
{
}
