#include "CoreEngine.h"
#include "OverlayRenderer.h"
#include "EnvironmentOverlay.h"

// Render the calculated data for a specific environment overlay
void EnvironmentOverlay::RenderOverlayData(
		iSpaceObjectEnvironment & environment,														// Environment for which the overlay is being rendered
		std::vector<XMFLOAT4>::const_iterator begin, std::vector<XMFLOAT4>::const_iterator end,		// Iterators into the range of overlay data
		int start_element)																			// Starting element within the environment that this data relates to
{
	Game::Engine->GetOverlayRenderer()->RenderEnvironment3DOverlay(environment, begin, end, start_element);
}