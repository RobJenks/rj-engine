#pragma once

#ifndef __BasicProjectileH__
#define __BasicProjectileH__

#include "DX11_Core.h"
class BasicProjectileDefinition;

struct BasicProjectile
{
public:

	// Fields are exposed publicly
	const BasicProjectileDefinition *						Definition;
	D3DXVECTOR3												Position;
	D3DXVECTOR3												Velocity;
	int														Lifetime;
	float													SpeedSq;



};




#endif




