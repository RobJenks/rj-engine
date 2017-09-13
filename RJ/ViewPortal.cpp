#include "FastMath.h"
#include "DX11_Core.h"
#include "ViewPortal.h"


// Constructor for a new view portal, providing only its position in parent-local space and direction
// of the portal target.  Used at load-time when we do not have any details of the parent object and 
// therefore where the portal is currently located in the environment
ViewPortal::ViewPortal(const FXMVECTOR min_point, const FXMVECTOR max_point, Direction target_direction) noexcept
	:
	Bounds(min_point, max_point), m_target_direction(target_direction), 
	m_location(0U), m_target(0U)
{
	RecalculateData();
}

// Copy constructor
ViewPortal::ViewPortal(const ViewPortal & other) noexcept
	:
	Bounds(other.Bounds), m_location(other.m_location), m_target(other.m_target), m_target_direction(other.m_target_direction), 
	m_centre(other.m_centre), m_bounding_sphere_radius(other.m_bounding_sphere_radius), m_normal(other.m_normal)
{
}

// Copy assignment
ViewPortal & ViewPortal::operator=(const ViewPortal & other) noexcept
{
	Bounds = other.Bounds;
	m_location = other.m_location;
	m_target = other.m_target;
	m_target_direction = other.m_target_direction;
	m_centre = other.m_centre;
	m_bounding_sphere_radius = other.m_bounding_sphere_radius;
	m_normal = other.m_normal;
	return *this;
}

// Move constructor
ViewPortal::ViewPortal(ViewPortal && other) noexcept
	:
	Bounds(std::move(other.Bounds)), m_location(other.m_location), m_target(other.m_target), m_target_direction(other.m_target_direction), 
	m_centre(other.m_centre), m_bounding_sphere_radius(other.m_bounding_sphere_radius), m_normal(other.m_normal)
{
}

// Move assignment
ViewPortal & ViewPortal::operator=(ViewPortal && other) noexcept
{
	Bounds = std::move(other.Bounds);
	m_location = other.m_location;
	m_target = other.m_target;
	m_target_direction = other.m_target_direction;
	m_centre = other.m_centre;
	m_bounding_sphere_radius = other.m_bounding_sphere_radius;
	m_normal = other.m_normal;
	return *this;
}

// Recalculates internal data within the portal following a change to the vertex layout
void ViewPortal::RecalculateData(void)
{
	// Determine an approximate local-space midpoint and bounding radius for the portal
	m_centre = XMVectorMultiply(XMVectorAdd(Bounds.P0, Bounds.P1), HALF_VECTOR);
	m_bounding_sphere_radius = XMVectorGetX(XMVectorScale(XMVector3LengthEst(
		XMVectorSubtract(Bounds.P0, Bounds.P1)), 0.5f));

	// Calculate the normal vector 
	
}

// Debug string representation of the portal
std::string ViewPortal::DebugString(void) const
{
	return concat("ViewPortal [Location=")(m_location)(", target=")(m_target)(", bounds=")(Vector3ToString(Bounds.MinPoint()))("-")(Vector3ToString(Bounds.MaxPoint()))
		(", centre=")(Vector3ToString(m_centre))(", bounding_radius=")(m_bounding_sphere_radius)("]").str();
}

// Destructor; deallocates all storage owned by the object
ViewPortal::~ViewPortal(void)
{
}
