#include "FastMath.h"
#include "DX11_Core.h"
#include "ViewPortal.h"

// Static instance representing a null portal that will have no effect; returned when supplied data
// is incorrect or insufficient to construct a valid portal
const ViewPortal ViewPortal::NullPortal = ViewPortal(NULL_VECTOR, NULL_VECTOR, NULL_VECTOR, NULL_VECTOR);

// Constructor for a new view portal, providing the vertices of the portal in parent-local (generally tile-
// local) space.  Portal properties are derived based on vertices; portal uses clockwise winding order
// to determine facing and target element
ViewPortal::ViewPortal(const AXMVECTOR(&vertices)[4]) noexcept
	:
	ViewPortal(vertices[0], vertices[1], vertices[2], vertices[3])
{
}

// Constructor for a new view portal, providing the vertices of the portal in parent-local (generally tile-
// local) space.  Portal properties are derived based on vertices; portal uses clockwise winding order
// to determine facing and target element
ViewPortal::ViewPortal(const FXMVECTOR p0, const FXMVECTOR p1, const FXMVECTOR p2, const FXMVECTOR p3)
{
	Vertices[0] = p0; Vertices[1] = p1; Vertices[2] = p2; Vertices[3] = p3;
	m_normal = XMVector3NormalizeEst(DetermineVectorNormal(Vertices[0], Vertices[1], Vertices[2]));

	// Portal target direction can be determined based on the vector normal
	m_target_direction = DeterminePortalTargetDirection(m_normal);

	// Current and target location are not known at construction-time
	m_location = m_target = 0U;

	// Calculate all remaining fields based on those derived above
	RecalculateData();
}

// Copy constructor
ViewPortal::ViewPortal(const ViewPortal & other) noexcept
	:
	m_location(other.m_location), m_target(other.m_target), m_target_direction(other.m_target_direction), 
	m_centre(other.m_centre), m_bounding_sphere_radius(other.m_bounding_sphere_radius), m_normal(other.m_normal)
{
	Vertices[0] = other.Vertices[0]; Vertices[1] = other.Vertices[1]; Vertices[2] = other.Vertices[2]; Vertices[3] = other.Vertices[3];
}

// Copy assignment
ViewPortal & ViewPortal::operator=(const ViewPortal & other) noexcept
{
	Vertices[0] = other.Vertices[0]; Vertices[1] = other.Vertices[1]; Vertices[2] = other.Vertices[2]; Vertices[3] = other.Vertices[3];
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
	m_location(other.m_location), m_target(other.m_target), m_target_direction(other.m_target_direction), 
	m_centre(other.m_centre), m_bounding_sphere_radius(other.m_bounding_sphere_radius), m_normal(other.m_normal)
{
	Vertices[0] = other.Vertices[0]; Vertices[1] = other.Vertices[1]; Vertices[2] = other.Vertices[2]; Vertices[3] = other.Vertices[3];
}

// Move assignment
ViewPortal & ViewPortal::operator=(ViewPortal && other) noexcept
{
	Vertices[0] = other.Vertices[0]; Vertices[1] = other.Vertices[1]; Vertices[2] = other.Vertices[2]; Vertices[3] = other.Vertices[3];
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
	XMVECTOR vmin = Vertices[0]; XMVECTOR vmax = Vertices[0];
	for (int i = 1; i < 4; ++i)
	{
		vmin = XMVectorMin(vmin, Vertices[i]);
		vmax = XMVectorMax(vmax, Vertices[i]);
	}

	m_centre = XMVectorMultiply(XMVectorAdd(vmin, vmax), HALF_VECTOR);
	m_bounding_sphere_radius = XMVectorGetX(XMVectorScale(XMVector3LengthEst(
		XMVectorSubtract(vmin, vmax)), 0.5f));
}

// Determines the adjacent element direction that this portal connects to, based upon the portal normal vector
Direction ViewPortal::DeterminePortalTargetDirection(const FXMVECTOR normal_vector)
{
	// Get the direction most closely aligned to our normal vector, then take the opposite
	// since portals face inwards.  A portal facing down (0, 0, -1) is providing a window
	// on the element above the current cell (i.e. 0, 0, +1)
	return GetOppositeDirection(DetermineClosestDirectionToVector(m_normal));
}

// Transform the portal by the given transformation matrix
void ViewPortal::Transform(const FXMMATRIX transform)
{
	// Transform the portal vertices and normal vector by the given transformation
	Vertices[0] = XMVector3TransformCoord(Vertices[0], transform);
	Vertices[1] = XMVector3TransformCoord(Vertices[1], transform);
	Vertices[2] = XMVector3TransformCoord(Vertices[2], transform);
	Vertices[3] = XMVector3TransformCoord(Vertices[3], transform);
	m_normal = XMVector3TransformCoord(m_normal, transform);
	m_target_direction = DeterminePortalTargetDirection(m_normal);

	// Recalculate all dependent data
	RecalculateData();
}

// Debug string representation of the portal
std::string ViewPortal::DebugString(void) const
{
	return concat("ViewPortal [Location=")(m_location)(", target_dir=")(m_target_direction)(", target=")(m_target)
		(", normal=")(Vector3ToString(m_normal))(", centre=")(Vector3ToString(m_centre))(", bounding_radius=")(m_bounding_sphere_radius)
		(", p0=")(Vector3ToString(Vertices[0]))(", p1=")(Vector3ToString(Vertices[1]))(", p2=")(Vector3ToString(Vertices[2]))(", p3=")(Vector3ToString(Vertices[3]))
		("]").str();
}

// Destructor; deallocates all storage owned by the object
ViewPortal::~ViewPortal(void)
{
}
