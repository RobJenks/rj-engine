#pragma once

#ifndef __ParticleBaseH__
#define __ParticleBaseH__


#include "DX11_Core.h"


class ParticleBase
{
public:
	D3DXVECTOR3						Location;
	D3DXVECTOR4						Colour;
	float							Size;

	
	ParticleBase(void);
	~ParticleBase(void);
};




#endif