#include "DX11_Core.h"
#include "Frustum.h"
using namespace DirectX;

// Static constant data
const size_t Frustum::NEAR_PLANE = 0U;						// Index into the plane collection
const size_t Frustum::FAR_PLANE = 1U;						// Index into the plane collection
const size_t Frustum::FIRST_SIDE = 2U;						// Index into the plane collection

// Constant control vector for combining ABC and D components of each plane
const AXMVECTOR Frustum::PLANE_CONTROL_VECTOR = XMVectorSelectControl(0U, 0U, 0U, 1U);

// Construct a new frustum with the specified number of sides (not including the near- & far-planes)
// Pass the near- and far-planes in during construction since they do not need to be calculated again
Frustum::Frustum(const size_t frustum_side_count, const FXMVECTOR near_plane, const FXMVECTOR far_plane)
{
	// Must have at least 3 sides.  Also impose a reasonable upper limit on complexity of the frustum
	assert(frustum_side_count >= 3U);
	assert(frustum_side_count < 36U);
	
	// Allocate space for the data
	m_planecount = frustum_side_count + 2U;			// +2 for the near- and far-planes
	m_planes = new AXMVECTOR[m_planecount];

	// Store or initialise all data
	m_planes[Frustum::NEAR_PLANE] = near_plane;
	m_planes[Frustum::FAR_PLANE] = far_plane;
	for (size_t i = Frustum::FIRST_SIDE; i < m_planecount; ++i)
	{
		m_planes[i] = NULL_VECTOR;
	}
}

// Add a new side to the frustum, based upon a viewer position and two further points in the world
// Result will be a triangular plane from the viewer with far edge between the two world vertices, 
// forming one side of the eventual frustum pyramid
// Info here: https://www.flipcode.com/archives/Building_a_3D_Portal_Engine-Issue_06_Hidden_Surface_Removal.shtml
void Frustum::SetPlane(size_t plane, const FXMVECTOR view_position, const FXMVECTOR p0, const FXMVECTOR p1)
{
	assert(plane >= 0 && plane < m_planecount);

	// double rx1 = c2->x - c1->x; double ry1 = c2->y - c1->y; double rz1 = c2->z - c1->z;
	// double rx2 = c3->x - c1->x; double ry2 = c3->y - c1->y; double rz2 = c3->z - c1->z;
	XMVECTOR r1 = XMVectorSubtract(p0, view_position);
	XMVECTOR r2 = XMVectorSubtract(p1, view_position);

	// A = ry1*rz2 - ry2*rz1;
	// B = rz1*rx2 - rz2*rx1;
	// C = rx1*ry2 - rx2*ry1;
	// Take the cross product
	XMVECTOR ABC = XMVector3Cross(r1, r2);

	// double len=sqrt(A*A+B*B+C*C);
	// A = A / len; B = B / len; C = C / len;
	// Normalise.  TODO: use NormalizeEst in future if loss of precision is acceptable
	ABC = XMVector3Normalize(ABC);

	// D=A*c2->x+B*c2->y+C*c2->z;
	// Substitute one point into the equation to derive D (which will need to be summed from each component)
	XMVECTOR D_components = XMVectorMultiply(ABC, p0);

	// Retrieve the components of D and add to the ABC vector to give the full set of plane coefficients
	// Use a rotate/add here to keep things within the SSE pipeline and hopefully be more efficient than a manual store
	XMVECTOR D_sum = XMVectorAdd(XMVectorAdd(
		XMVectorRotateLeft<1U>(D_components),				// Add yzwX
		XMVectorRotateLeft<2U>(D_components)),				// to  zwxY
		XMVectorRotateLeft<3U>(D_components));				// to  wxyZ

	SetPlane(plane, XMVectorSelect(ABC, D_components, Frustum::PLANE_CONTROL_VECTOR));
}

// Add a new side to the frustum by providing the plane coefficients directly
void Frustum::SetPlane(size_t plane, const FXMVECTOR plane_coeff)
{
	assert(plane >= 0 && plane < m_planecount);

	m_planes[plane] = plane_coeff;
}

// Test whether the given bounding sphere lies within the frustum
bool Frustum::CheckSphere(const FXMVECTOR sphere_centre, float sphere_radius)
{
	// If the sphere is 'behind' any plane of the view frustum then return false immediately
	XMVECTOR radius = XMVectorReplicate(-sphere_radius);
	
	if (XMVector2Less(XMPlaneDotCoord(m_planes[Frustum::NEAR_PLANE], sphere_centre), radius)) return false;
	if (XMVector2Less(XMPlaneDotCoord(m_planes[Frustum::FAR_PLANE], sphere_centre), radius)) return false;
	for (size_t i = Frustum::FIRST_SIDE; i < m_planecount; ++i)
	{
		if (XMVector2Less(XMPlaneDotCoord(m_planes[i], sphere_centre), radius)) return false;
	}

	// The object was not behind any planes in the frustum so return true
	return true;
}



// Destructor
Frustum::~Frustum(void)
{
	// Release any allocated resources
	if (m_planes) SafeDeleteArray(m_planes);
	m_planecount = 0U;
}