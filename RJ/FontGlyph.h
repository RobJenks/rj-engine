#pragma once

#include "IntVector.h"


struct FontGlyph
{
public:

	// Glpyh data
	UINTVECTOR2					Location;
	UINTVECTOR2					Size;


	// Constructor
	FontGlyph(void);
	FontGlyph(UINTVECTOR2 location, UINTVECTOR2 size);


	// Destructor
	~FontGlyph(void);
};