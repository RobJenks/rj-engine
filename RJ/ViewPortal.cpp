#include "FastMath.h"
#include "DX11_Core.h"
#include "ViewPortal.h"

// Static instance representing a null portal that will have no effect; returned when supplied data
// is incorrect or insufficient to construct a valid portal
const ViewPortal ViewPortal::NullPortal = ViewPortal(NULL_VECTOR, NULL_VECTOR, FORWARD_VECTOR);

// Constructor for a new view portal, providing the vertices of the portal in parent-local (generally tile-
// local) space.  Portal properties are derived based on vertices; portal uses clockwise winding order
// to determine facing and target element
ViewPortal::ViewPortal(const std::vector<XMFLOAT3> & vertices) noexcept
	:
	Bounds(vertices)	// Construct an AABB that encloses all supplied vertices
{
	// We must have at least three vertices to determine a normal vector (and to define a valid portal)
	size_t n = vertices.size();
	if (n < 3)
	{
		Bounds = AABB(NULL_VECTOR, NULL_VECTOR);
		m_normal = FORWARD_VECTOR;
	}
	else
	{
		m_normal = XMVector3NormalizeEst(DetermineVectorNormal(XMLoadFloat3(&(vertices[0U])), XMLoadFloat3(&(vertices[1U])), XMLoadFloat3(&(vertices[2U]))));
	}

	// Portal target direction can be determined based on the vector normal
	m_target_direction = DeterminePortalTargetDirection(m_normal);

	// Current and target location are not known at construction-time
	m_location = m_target = 0U;

	// Calculate all remaining fields based on those derived above
	RecalculateData();
}

// Constructor for a new view portal, providing the vertices of the portal in parent-local (generally tile-
// local) space.  Portal properties are derived based on vertices; portal uses clockwise winding order
// to determine facing and target element
ViewPortal::ViewPortal(const FXMVECTOR min_point, const FXMVECTOR max_point, const FXMVECTOR normal) noexcept
	:
	Bounds(min_point, max_point), m_normal(normal),
	m_target_direction(DeterminePortalTargetDirection(normal)),
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
	// Transform the portal bounds and normal vector by the given transformation
	Bounds.Transform(transform);
	m_normal = XMVector3TransformCoord(m_normal, transform);
	m_target_direction = DeterminePortalTargetDirection(m_normal);

	// Recalculate all dependent data
	RecalculateData();
}

// Debug string representation of the portal
std::string ViewPortal::DebugString(void) const
{
	return concat("ViewPortal [Location=")(m_location)(", target_dir=")(m_target_direction)(", target=")(m_target)(", bounds=")(Vector3ToString(Bounds.MinPoint()))("-")(Vector3ToString(Bounds.MaxPoint()))
		(", normal=")(Vector3ToString(m_normal))(", centre=")(Vector3ToString(m_centre))(", bounding_radius=")(m_bounding_sphere_radius)("]").str();
}

// Destructor; deallocates all storage owned by the object
ViewPortal::~ViewPortal(void)
{
}
