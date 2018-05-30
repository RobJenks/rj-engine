#include "TextRenderer.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "DecalRenderingManager.h"

// Default font size; character scaling will use this as the baseline
const float TextRenderer::DEFAULT_FONT_SIZE = 12.0f;
const float TextRenderer::DEFAULT_FONT_SIZE_RECIP = (1.0f / TextRenderer::DEFAULT_FONT_SIZE);

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
float TextRenderer::GlyphScalingFactor(float font_size)
{
	return (font_size * TextRenderer::DEFAULT_FONT_SIZE_RECIP);
}

// Renders the given character to the screen
void TextRenderer::RenderCharacterToScreen(unsigned int ch, Font::ID font, const FXMVECTOR screen_location, float font_size,
										   const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, float outlineFactor) const
{
	// Get the corresponding font and set decal rendering parameters accordingly
	const auto & font_data = GetFont(font);
	SetDecalRenderingParameters(font_data, basecolour, outlinecolour, outlineFactor);
	
	// Glyph scaling can be determined based on desired font size
	float glyph_scale = GlyphScalingFactor(font_size);

	// Get glyph data and push a request to the decal renderer
	const auto & glyph = font_data.GetGlyph(ch);
	RenderGlyphDecal(glyph, screen_location, glyph_scale);
}

// Renders the given text string to the screen.  No wrapping is performed
void TextRenderer::RenderStringToScreen(const std::string & str, Font::ID font, XMVECTOR screen_location, float font_size,
										const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, float outlineFactor) const
{
	// Get the corresponding font and set decal rendering parameters accordingly
	const auto & font_data = GetFont(font);
	SetDecalRenderingParameters(font_data, basecolour, outlinecolour, outlineFactor);

	// Glyph scaling can be determined based on desired font size
	float glyph_scale = GlyphScalingFactor(font_size);
	float separation = font_data.GetCharacterSeparation();

	// Push consecutive requests to render each glyph in turn
	for (unsigned int ch : str)
	{
		const auto & glyph = font_data.GetGlyph(ch);
		RenderGlyphDecal(glyph, screen_location, glyph_scale);

		screen_location = XMVectorAdd(screen_location, XMVectorSetX(NULL_VECTOR, (glyph.Size.x + separation) * glyph_scale));
	}
}

// Pass parameters to the decal renderer that will be used for all subsequent text rendering calls
void TextRenderer::SetDecalRenderingParameters(const Font & font, const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, float outlineWidth) const
{
	auto * renderer = Game::Engine->GetDecalRenderer();

	renderer->SetTexture(font.GetTextureMap());
	renderer->SetBaseColour(basecolour);
	renderer->SetOutlineColour(outlinecolour);
	renderer->SetOutlineWidthFactor(outlineWidth);
}

// Perform glyph calculation and dispatch a render call to the decal renderer
void TextRenderer::RenderGlyphDecal(const FontGlyph & glyph, const FXMVECTOR location, float glyph_scale) const
{
	XMFLOAT2 size(static_cast<float>(glyph.Size.x) * glyph_scale, static_cast<float>(glyph.Size.y) * glyph_scale);

	Game::Engine->GetDecalRenderer()->RenderDecalScreen(location, size, glyph.Location, (glyph.Location + glyph.Size));
}

// Calculates the dimensions of a text string with the given properties
XMFLOAT2 TextRenderer::CalculateTextDimensions(const std::string & text, Font::ID font, float font_size) const
{
	XMFLOAT2 dimensions(0.0f, 0.0f);

	// Retrieve font details
	const Font & fontdata = GetFont(font);
	float separation = fontdata.GetCharacterSeparation();

	// Determine a glyph scaling factor based on this font size
	float scalefactor = GlyphScalingFactor(font_size);

	// Determine size of each glyph and separators in turn
	for (auto ch : text)
	{
		const auto & size = fontdata.GetGlyph((unsigned int)ch).Size;

		dimensions.x += (scalefactor * (size.x + separation));
		dimensions.y = max(dimensions.y, size.y);
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
