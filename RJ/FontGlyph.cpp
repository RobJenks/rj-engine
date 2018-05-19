#include "FontGlyph.h"
#include "FastMath.h"


// Constructor
FontGlyph::FontGlyph(void)
	:
	FontGlyph(NULL_UINTVECTOR2, NULL_UINTVECTOR2)
{
}

// Constructor
FontGlyph::FontGlyph(UINTVECTOR2 location, UINTVECTOR2 size)
	:
	Location(location), 
	Size(size)
{
}


// Destructor
FontGlyph::~FontGlyph(void)
{

}