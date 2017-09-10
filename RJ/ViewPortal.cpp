#include "FastMath.h"
#include "DX11_Core.h"
#include "ViewPortal.h"

// Constructor for a new view portal.  Vertices are specified in parent-local (generally tile-local) coordinates
ViewPortal::ViewPortal(const FXMVECTOR min_point, const FXMVECTOR max_point) noexcept
	:
	Bounds(min_point, max_point)
{
	RecalculateData();
}

// Copy constructor
ViewPortal::ViewPortal(const ViewPortal & other) noexcept
	:
	Bounds(other.Bounds), m_centre(other.m_centre), m_bounding_sphere_radius(other.m_bounding_sphere_radius)
{
}

// Copy assignment
ViewPortal & ViewPortal::operator=(const ViewPortal & other) noexcept
{
	Bounds = other.Bounds;
	m_centre = other.m_centre;
	m_bounding_sphere_radius = other.m_bounding_sphere_radius;
	return *this;
}

// Move constructor
ViewPortal::ViewPortal(ViewPortal && other) noexcept
	:
	Bounds(std::move(other.Bounds)), m_centre(other.m_centre), m_bounding_sphere_radius(other.m_bounding_sphere_radius)
{
}

// Move assignment
ViewPortal & ViewPortal::operator=(ViewPortal && other) noexcept
{
	Bounds = std::move(other.Bounds);
	m_centre = other.m_centre;
	m_bounding_sphere_radius = other.m_bounding_sphere_radius;
	return *this;
}

// Recalculates internal data within the portal following a change to the vertex layout
void ViewPortal::RecalculateData(void)
{
	// Determine an approximate local-space midpoint and bounding radius for the portal
	m_centre = XMVectorMultiply(XMVectorAdd(Bounds.P0, Bounds.P1), HALF_VECTOR);
	m_bounding_sphere_radius = XMVectorGetX(XMVectorScale(XMVector3LengthEst(
		XMVectorSubtract(Bounds.P0, Bounds.P1)), 0.5f));
}

// Destructor; deallocates all storage owned by the object
ViewPortal::~ViewPortal(void)
{
}
