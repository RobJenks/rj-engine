#pragma once

#include <vector>
#include "CompilerSettings.h"
#include "DX11_Core.h"


class JitterSampleDistribution
{
public:

	// Generates a Halton-2-3 distribution of the requested size.  Uses prime seed factors of 2 and 3
	// Destination must be a pre-allocated XMFLAOT2 array of size N
	static void								GenerateHalton23Distribution(std::vector<XMFLOAT2> & outDistribution, unsigned int N);



	// Generate an element of the Halton sequence based on the given prime seed and index.  Index
	// cannot be zero-based, i.e must be >= 1, per the definition of the sequence
	static float							HaltonSequence(int prime, int index = 1 /* Cannot be zero-based */);
	

};
