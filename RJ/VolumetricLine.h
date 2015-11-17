#pragma once

#ifndef __VolumetricLineH__
#define __VolumetricLineH__

#include "DX11_Core.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct VolumetricLine : public ALIGN16<VolumetricLine>
{
	// Line endpoints P1 and P2
	AXMVECTOR				P1;
	AXMVECTOR				P2;


	// Default constructor
	VolumetricLine(void) { }

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2)
		:
		P1(_P1), P2(_P2)
	{
	}

};


#endif