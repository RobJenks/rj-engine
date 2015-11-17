#pragma once

#ifndef __ParticleDefinitionH__
#define __ParticleDefinitionH__


#include "DX11_Core.h"


// This class has no special alignment requirements
class ParticleDefinition
{
public:
	XMFLOAT3						InitialColour;
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