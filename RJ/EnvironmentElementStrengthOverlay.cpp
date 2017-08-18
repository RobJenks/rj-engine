#include "iSpaceObjectEnvironment.h"
#include "EnvironmentElementStrengthOverlay.h"

void EnvironmentElementStrengthOverlay::Render(iSpaceObjectEnvironment & environment, int start, int end)
{
	// Parameter check; this must be a contiguous range within the set of environment elements
	if (start > end) std::swap(start, end);
	if (start < 0 || end >= environment.m_elementcount) return;
	unsigned int count = (unsigned int)(end - start + 1);

	// Allocate an array for the rendering data.  We only need to calculate values for the specified range
	std::vector<XMFLOAT4>::size_type data_index = 0U;
	std::vector<XMFLOAT4> data(count);

	// First pass: store strength value in w component and keep track of the highest value
	float highest = 1.0f;
	for (int i = start; i <= end; ++i, ++data_index)
	{
		float strength = environment.DetermineTotalElementImpactStrength(i);
		data[data_index].w = strength;
		if (strength > highest) highest = strength;
	}

	// Second pass: divide values through by highest value such that all elements are scaled R>G from lowest>highest
	for (std::vector<XMFLOAT4>::size_type i = 0; i < count; ++i)
	{
		float pc = (data[i].w / highest);
		data[i] = XMFLOAT4(1.0f - pc, pc, 0.0f, 0.75f);
	}

	// Render this overlay on the environment
	RenderOverlayData(environment, data.begin(), data.end(), start);
}


