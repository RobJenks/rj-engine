#include "Font.h"
#include "Utility.h"

const float Font::DEFAULT_CHARACTER_SEPARATION = 1.0f;


// Constructor
Font::Font(void)
	:
	Font(NullString)
{
}

// Constructor
Font::Font(const std::string & code)
	:
	m_code(code), 
	m_texture(NULL), 
	m_char_separation(Font::DEFAULT_CHARACTER_SEPARATION), 
	m_glyph_scale_factor(1.0f)
{
}


// Return glyph data for the given character
const FontGlyph & Font::GetGlyph(unsigned int ch) const
{
	if (ch < FIRST_CHARACTER || ch > LAST_CHARACTER) return m_map[DEFAULT_CHARACTER];

	return m_map[ch - FIRST_CHARACTER];
}

// Store glyph parameters for the given character
void Font::SetGlyphData(unsigned int ch, const FontGlyph & data)
{
	if (ch < FIRST_CHARACTER || ch > LAST_CHARACTER) return;

	m_map[ch - FIRST_CHARACTER] = data;
}




// Destructor
Font::~Font(void)
{
}
