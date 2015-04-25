#pragma once

#ifndef __ParticleDefinitionH__
#define __ParticleDefinitionH__


#include "DX11_Core.h"



class ParticleDefinition
{
public:
	D3DXVECTOR3						InitialColour;
	float							InitialSize;

	bool							HasSizeDecay;
	bool							HasColourDecay;
	float							SizeDecay;
	float							ColourDecay;

	float							SizeDecayThreshold;
	float							ColourDecayThreshold;


	ParticleDefinition(void);
	~ParticleDefinition(void);
};



#endif