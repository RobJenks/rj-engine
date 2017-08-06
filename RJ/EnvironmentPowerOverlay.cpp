#include "iSpaceObjectEnvironment.h"
#include "EnvironmentPowerOverlay.h"

void EnvironmentPowerOverlay::Render(iSpaceObjectEnvironment & environment, int start, int end)
{
	const XMFLOAT4 render_power = XMFLOAT4(0.29f, 1.0f, 1.0f, 0.75f);
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
		Power::Type x = environment.m_powermap.GetPowerLevel(i);
		if (x != 0) data[data_index] = render_power;
	}

	// Also render each source.  Redundant call to DetermineSources but this is for debug only
	std::vector<EnvironmentPowerMap::PowerMap::MapCell> sources;
	environment.m_powermap.DeterminePowerSources(sources);
	for (EnvironmentPowerMap::PowerMap::MapCell source : sources)
	{
		std::vector<XMFLOAT4>::size_type adj_index = (source.Index - start);
		if (adj_index >= count) continue;										// Unsigned; no need to check < 0

		data[adj_index] = render_source;
	}

	// Render this overlay on the environment
	RenderOverlayData(environment, data.begin(), data.end(), start);
}



