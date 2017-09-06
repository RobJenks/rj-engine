#pragma once

#include <vector>
#include "CompilerSettings.h"
class ComplexShipTile;
class Frustum;


struct PortalRenderingStep
{
	// The cell that is currently being rendered
	ComplexShipTile *						Cell;

	// Index of the portal-clipped view frustum for this rendering step
	std::vector<Frustum*>::size_type		VisibilityFrustum;

	// Traversal count to this point, to prevent circular rendering loops (possible?)
	size_t									TraversalCount;


	// Constructor
	PortalRenderingStep(void) noexcept;
	PortalRenderingStep(ComplexShipTile *cell, std::vector<Frustum*>::size_type visibility_frustum) noexcept;

	// Copy constructor and copy assignment are deleted
	CMPINLINE PortalRenderingStep(const PortalRenderingStep & other) = delete;
	CMPINLINE PortalRenderingStep & operator=(const PortalRenderingStep & other) = delete;

	// Move constructor
	PortalRenderingStep(PortalRenderingStep && other);

	// Move assignment
	PortalRenderingStep & operator=(PortalRenderingStep && other);

	// Destructor
	~PortalRenderingStep(void);

};
