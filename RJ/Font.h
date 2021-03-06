#pragma once

#include <string>
#include <array>
#include "CompilerSettings.h"
#include "FontGlyph.h"
class TextureDX11;


class Font
{
public:

	// ID type used to reference fonts.  Unique IDs maintained by the TextRenderer component
	typedef size_t ID;
	static const Font::ID	  DEFAULT_FONT_ID = 0U;

	// For now, we only support characters in the ASCII and Latin Supplemental set.  All characters from 32/0x20/' ' 
	// to 255/FF/'y-with-umlaut' inclusive are printable and can be stored in the character map
	// We can extend to a larger set of unicode characters in the future if required.  May require map (ccode -> glyph instead)
	static const unsigned int FIRST_CHARACTER = 32U;
	static const unsigned int LAST_CHARACTER = 255U;
	static const unsigned int CHARACTER_MAP_SIZE = (LAST_CHARACTER - FIRST_CHARACTER + 1U);
	static const unsigned int DEFAULT_CHARACTER = '?';		// If the requested glyph is not available

	// Other constants
	static const float DEFAULT_CHARACTER_SEPARATION;


	// Constructor
	Font(void);
	Font(const std::string & code);

	// Basic font data
	CMPINLINE std::string						GetCode(void) const { return m_code; }
	CMPINLINE void								SetCode(const std::string & name) { m_code = name; }
	

	// Texture map containing all glyphs for this font
	CMPINLINE const TextureDX11 *				GetTextureMap(void) const { return m_texture; }
	CMPINLINE void								SetTextureMap(const TextureDX11 * texture) { m_texture = texture; }

	// Separation between adjactent chars (pixels) at default size
	CMPINLINE float								GetCharacterSeparation(void) const { return m_char_separation; }
	CMPINLINE void								SetCharacterSeparation(float px) { m_char_separation = px; }

	// Scaling factor that will convert an unscaled glyph to the 'default' font size, to account for different-sized source glyphs
	CMPINLINE float								GetGlyphScalingFactor(void) const { return m_glyph_scale_factor; }
	CMPINLINE void								SetGlyphScalingFactor(float factor) { m_glyph_scale_factor = factor; }

	// Return glyph data for the given character
	const FontGlyph &							GetGlyph(unsigned int ch) const;

	// Store glyph parameters for the given character
	void										SetGlyphData(unsigned int ch, const FontGlyph & data);




	// Destructor
	~Font(void);


private:

	std::string									m_code;

	const TextureDX11 *							m_texture;
	std::array<FontGlyph, CHARACTER_MAP_SIZE>	m_map;
	float										m_char_separation;	// Separation between adjactent chars (pixels) at default size
	float										m_glyph_scale_factor;


};