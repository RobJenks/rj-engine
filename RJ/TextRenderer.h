#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Font.h"

class TextRenderer
{
public:

	// Default font size; character scaling will use this as the baseline
	static const float							DEFAULT_FONT_SIZE;
	
	// Constructor
	TextRenderer(void);

	// Register a new font.  Returns a flag indicating whether the font could be registered
	bool										RegisterFont(Font *font);

	// Retrieve a font based on its ID
	const Font &								GetFont(Font::ID id) const;

	// Get the ID of a font with the given name.  Returns Font::DEFAULT_FONT_ID if no such
	// font exists
	Font::ID									GetFontID(const std::string & name) const;

	// Get a font based on its string name
	const Font &								GetFont(const std::string & name) const;

	// Indicates whether the given identifier represents a valid registered font
	bool										IsValidFont(Font::ID id) const;
	bool										IsValidFont(const std::string & name) const;

	// Returns the default font ID
	CMPINLINE Font::ID							GetDefaultFontId(void) const { return Font::DEFAULT_FONT_ID; }

	// Returns the glyph scaling factor for the given font size
	static float								GlyphScalingFactor(const Font & font, float font_size);

	// Renders the given character to the screen
	void										RenderCharacterToScreen(unsigned int ch, Font::ID font, const FXMVECTOR screen_location, float font_size,
																		const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, 
																		float outlineFactor = 0.0f) const;

	// Renders the given text string to the screen.  No wrapping is performed
	void										RenderStringToScreen(const std::string & str, Font::ID font, XMVECTOR screen_location, float font_size,
																	 const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, 
																	 float outlineFactor = 0.0f) const;

	// Calculates the dimensions of a text string with the given properties
	XMFLOAT2									CalculateTextDimensions(const std::string & text, Font::ID font, float font_size) const;


	// Shutdown and release all owned resources
	void										Shutdown(void);

	// Destructor
	~TextRenderer(void);


private:

	// Pass parameters to the decal renderer that will be used for all subsequent text rendering calls
	void										SetDecalRenderingParameters(const Font & font, const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour, float outlineWidth) const;

	// Perform glyph calculation and dispatch a render call to the decal renderer
	void										RenderGlyphDecal(const FontGlyph & glyph, const FXMVECTOR location, float glyph_scale) const;


private:

	// Primary collections of font rendering data
	std::vector<Font*>							m_fonts;
	std::unordered_map<std::string, Font::ID>	m_font_ids;

	std::vector<Font>::size_type				m_font_count;
	

	// Reciprocal of default font size, for runtime efficiency
	static const float							DEFAULT_FONT_SIZE_RECIP;

	// Height (in pixels) of a font glyph at DEFAULT_FONT_SIZE
	static const float							DEFAULT_FONT_SIZE_HEIGHT_PX;

	// Null font object, to be returned in case of invalid requests
	std::unique_ptr<Font>						m_null_font;

};