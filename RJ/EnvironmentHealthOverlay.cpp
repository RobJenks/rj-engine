#include "iSpaceObjectEnvironment.h"
#include "EnvironmentHealthOverlay.h"

void EnvironmentHealthOverlay::Render(iSpaceObjectEnvironment & environment, int start, int end)
{
	// Parameter check; this must be a contiguous range within the set of environment elements
	if (start > end) std::swap(start, end);
	if (start < 0 || end >= environment.m_elementcount) return;
	int count = (end - start + 1);

	// Allocate an array for the rendering data.  We only need to calculate values for the specified range
	std::vector<XMFLOAT4>::size_type data_index = 0U;
	std::vector<XMFLOAT4> data((std::vector<XMFLOAT4>::size_type)count);
	for (int i = start; i <= end; ++i, ++data_index)
	{
		if (environment.m_elements[i].IsDestroyed())
			data[data_index] = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.85f);		// Gray
		else
		{
			float x = environment.m_elements[i].GetHealth();
			data[data_index] = XMFLOAT4(1.0f - x, x, 0.0f, 0.75f);		// Green -> Red based on health
		}
	}

	// Overwrite any elements which contain a hull breach
	EnvironmentHullBreaches::size_type breach_count = environment.HullBreaches.GetBreachCount();
	for (EnvironmentHullBreaches::size_type b = 0U; b < breach_count; ++b)
	{
		EnvironmentHullBreach & breach = environment.HullBreaches.Get(b);
		std::vector<XMFLOAT4>::size_type adj_index = (breach.GetElementIndex() - start);
		if (adj_index >= count) continue;								// Unsigned; no need to check < 0

		data[adj_index] = XMFLOAT4(0.16f, 0.16f, 1.0f, 0.9f);
	}


	// Render this overlay on the environment
	RenderOverlayData(environment, data.begin(), data.end(), start);
}



