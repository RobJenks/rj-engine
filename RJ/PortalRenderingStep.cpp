#include <cstddef>
#include "PortalRenderingStep.h"
#include "ComplexShipTile.h"

// Constructor
PortalRenderingStep::PortalRenderingStep(void) noexcept
	:
	Cell(NULL), VisibilityFrustum(0U), TraversalCount(0U)
{
}

// Constructor
PortalRenderingStep::PortalRenderingStep(ComplexShipTile *cell, std::vector<Frustum*>::size_type visibility_frustum, size_t traversal_count) noexcept
	:
	Cell(cell), VisibilityFrustum(visibility_frustum), TraversalCount(traversal_count)
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

// Debug string output
std::string PortalRenderingStep::DebugString(void) const
{
	return concat("PortalRenderingStep [Cell=")(Cell ?
		concat(Cell->GetID())("\"")(Cell->GetCode())("\" at ")(Cell->GetElementLocation().ToString()).str().c_str() : "<NULL>")
		(", TraversalCount=")(TraversalCount)("]").str();
}

// Destructor
PortalRenderingStep::~PortalRenderingStep(void)
{
}
