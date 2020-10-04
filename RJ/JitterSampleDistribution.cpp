#include "JitterSampleDistribution.h"


// Generate a Halton-2-3 distribution of the given size.  Elements will be pushed into the target vector
void JitterSampleDistribution::GenerateHalton23Distribution(std::vector<XMFLOAT2> & outDistribution, unsigned int N)
{
	for (unsigned int i = 0; i != N; ++i)
	{
		outDistribution.push_back(XMFLOAT2
		(
			HaltonSequence(2, i + 1) - 0.5f,
			HaltonSequence(3, i + 1) - 0.5f
		));
	}
}

// Generate an element of the Halton sequence based on the given prime seed and index.  Index
// cannot be zero-based, i.e must be >= 1, per the definition of the sequence
float JitterSampleDistribution::HaltonSequence(int prime, int index /* Cannot be zero-based */)
{
	float r = 0.0f, f = 1.0f;

	int i = index;
	while (i > 0)
	{
		f /= prime;
		r += f * (i % prime);
		i = static_cast<int>( floor(i / static_cast<float>(prime)) );
	}
	return r;
}