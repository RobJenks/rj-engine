#pragma once

#ifndef __DustParticleH__
#define __DustParticleH__


#include "ParticleBase.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class DustParticle : public ALIGN16<DustParticle>, public ParticleBase
{
public:


	DustParticle(void);
	~DustParticle(void);
};



#endif