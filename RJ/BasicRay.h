#pragma once

#ifndef __BasicRayH__
#define __BasicRayH__

#include "DX11_Core.h"

class BasicRay
{
public:

	D3DXVECTOR3				Origin;					// Origin point of the ray 
	D3DXVECTOR3				Direction;				// Ray direction in world space

	// Constructor; accepts value to initialise the ray
	BasicRay(const D3DXVECTOR3 & origin, const D3DXVECTOR3 & direction)
		: Origin(origin), Direction(direction)
	{
	}

};


#endif