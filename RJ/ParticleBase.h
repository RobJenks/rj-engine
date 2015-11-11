#pragma once

#ifndef __ParticleBaseH__
#define __ParticleBaseH__


#include "DX11_Core.h"


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ParticleBase : public ALIGN16<ParticleBase>
{
public:
	AXMVECTOR						Location;
	AXMVECTOR						Colour;
	float							Size;

	
	ParticleBase(void);
	~ParticleBase(void);
};




#endif