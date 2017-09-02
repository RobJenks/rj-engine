#include <cstddef>
#include "PortalRenderingStep.h"


// Constructor
PortalRenderingStep::PortalRenderingStep(void) noexcept
	:
	Cell(NULL), VisibilityFrustum(NULL), TraversalCount(0U)
{
}

// Constructor
PortalRenderingStep::PortalRenderingStep(ComplexShipTile *cell, Frustum *visibility_frustum) noexcept
	:
	Cell(cell), VisibilityFrustum(visibility_frustum), TraversalCount(0U)
{
}


// Move constructor
PortalRenderingStep::PortalRenderingStep(PortalRenderingStep && other)
	:
	Cell(other.Cell), VisibilityFrustum(other.VisibilityFrustum), TraversalCount(other.TraversalCount)
{
}

// Move assignment
PortalRenderingStep & PortalRenderingStep::operator=(PortalRenderingStep && other)
{
	Cell = other.Cell;
	VisibilityFrustum = other.VisibilityFrustum;
	TraversalCount = other.TraversalCount;
	return *this;
}

// Destructor
PortalRenderingStep::~PortalRenderingStep(void)
{
}
