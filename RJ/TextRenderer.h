#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Font.h"

class TextRenderer
{
public:

	// Constructor
	TextRenderer(void);

	// Register a new font.  Returns a flag indicating whether the font could be registered
	bool										RegisterFont(Font *font);

	// Retrieve a font based on its ID
	Font &										GetFont(Font::ID id);

	// Get the ID of a font with the given name.  Returns 0 for the default font if no such
	// font exists
	Font::ID									GetFontID(const std::string & name) const;

	// Get a font based on its string name
	Font &										GetFont(const std::string & name);

	// Indicates whether the given identifier represents a valid registered font
	bool										IsValidFont(Font::ID id) const;
	bool										IsValidFont(const std::string & name) const;


	// Shutdown and release all owned resources
	void										Shutdown(void);

	// Destructor
	~TextRenderer(void);

private:

	std::vector<Font*>							m_fonts;
	std::unordered_map<std::string, Font::ID>	m_font_ids;

	std::vector<Font>::size_type				m_font_count;
	

};