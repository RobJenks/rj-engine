#pragma once

#ifndef __BasicRayH__
#define __BasicRayH__

#include "DX11_Core.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class BasicRay : public ALIGN16<BasicRay>
{
public:

	AXMVECTOR				Origin;					// Origin point of the ray 
	AXMVECTOR				Direction;				// Ray direction in world space

	// Default constructor; accepts no data
	BasicRay(void) { }

	// Constructor; accepts value to initialise the ray
	BasicRay(const FXMVECTOR origin, const FXMVECTOR direction)
		: Origin(origin), Direction(direction)
	{
	}

	// Generates a string representation of the ray
	std::string				ToString(void) const;

};


#endif