#include "iSpaceObjectEnvironment.h"
#include "EnvironmentOxygenOverlay.h"

void EnvironmentOxygenOverlay::Render(iSpaceObjectEnvironment & environment, int start, int end)
{
	const XMFLOAT4 render_source = XMFLOAT4(1.0f, 0.745f, 0.078f, 0.85f);

	// Parameter check; this must be a contiguous range within the set of environment elements
	if (start > end) std::swap(start, end);
	if (start < 0 || end >= environment.m_elementcount) return;
	unsigned int count = (unsigned int)(end - start + 1);

	// Allocate an array for the rendering data.  We only need to calculate values for the specified range
	std::vector<XMFLOAT4>::size_type data_index = 0U;
	std::vector<XMFLOAT4> data(count);
	for (int i = start; i <= end; ++i, ++data_index)
	{
		Oxygen::Type x = environment.m_oxygenmap.GetOxygenLevel(i);
		data[data_index] = XMFLOAT4(1.0f - x, x, 0.0f, 0.75f);
	}

	// Also render each source.  Redundant call to DetermineSources but this is for debug only
	std::vector<EnvironmentOxygenMap::OxygenMap::MapCell> sources;
	environment.m_oxygenmap.DetermineOxygenSources(0.0f, sources);					// TimeFactor == 0.0f since not relevant here
	for (EnvironmentOxygenMap::OxygenMap::MapCell source : sources)
	{
		std::vector<XMFLOAT4>::size_type adj_index = (source.Index - start);
		if (adj_index >= count) continue;											// Unsigned; no need to check < 0

		data[adj_index] = render_source;
	}

	// Render this overlay on the environment
	RenderOverlayData(environment, data.begin(), data.end(), start);
}


