#include "TextRenderer.h"
#include "Logging.h"

// Constructor
TextRenderer::TextRenderer(void)
	:
	m_font_count(0U)
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
Font & TextRenderer::GetFont(Font::ID id)
{
	if (!IsValidFont(id)) return Font();

	return *m_fonts[id];
}

// Get the ID of a font with the given name.  Returns 0 for the default font if no such
// font exists
Font::ID TextRenderer::GetFontID(const std::string & code) const
{
	const auto it = m_font_ids.find(code);
	if (it == m_font_ids.end()) return 0U;

	return it->second;
}

// Get a font based on its string  name
Font & TextRenderer::GetFont(const std::string & code)
{
	const auto it = m_font_ids.find(code);
	if (it == m_font_ids.end()) return Font();

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
