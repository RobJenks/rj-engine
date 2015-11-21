#pragma once

#ifndef __BasicRayH__
#define __BasicRayH__

#include "DX11_Core.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class BasicSphere : ALIGN16<BasicSphere>
{
public:

	AXMVECTOR				Centre;					// Centre point of the sphere
	float					Radius;					// Radius of the sphere
	float					RadiusSq;				// Squared radius of the sphere, for efficiency





};


#endif