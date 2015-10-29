#pragma once

#ifndef __VolumetricLineH__
#define __VolumetricLineH__

#include "DX11_Core.h"

struct VolumetricLine
{
	// Line endpoints P1 and P2
	D3DXVECTOR3				P1;
	D3DXVECTOR3				P2;


	// Default constructor
	VolumetricLine(void) { }

	// Constructor
	VolumetricLine(const D3DXVECTOR3 & _P1, const D3DXVECTOR3 & _P2)
		:
		P1(_P1), P2(_P2)
	{
	}

};


#endif