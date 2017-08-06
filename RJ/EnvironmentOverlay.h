#pragma once

#include <vector>
#include "DX11_Core.h"
class iSpaceObjectEnvironment;

class EnvironmentOverlay
{

protected:

	// Render the calculated data for a specific environment overlay
	static void RenderOverlayData(
		iSpaceObjectEnvironment & environment,														// Environment for which the overlay is being rendered
		std::vector<XMFLOAT4>::const_iterator begin, std::vector<XMFLOAT4>::const_iterator end,		// Iterators into the range of overlay data
		int start_element																			// Starting element within the environment that this data relates to
	);


};
