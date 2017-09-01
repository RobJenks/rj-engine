#include "DX11_Core.h"
#include "Frustum.h"
using namespace DirectX;

// Static constant data
const size_t Frustum::NEAR_PLANE = 0U;						// Index into the plane collection
const size_t Frustum::FAR_PLANE = 1U;						// Index into the plane collection
const size_t Frustum::FIRST_SIDE = 2U;						// Index into the plane collection

// Construct a new frustum with the specified number of sides (not including the near- & far-planes)
// Pass the near- and far-planes in during construction since they do not need to be calculated again
// TODO: In future,  allow near/far planes to be derived as well so the frustum can be reused
// for non-viewer frustum tests as well (e.g. other actors, or light/portal testing)
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

	// Construct a plane from the given points and add it to the plane vector
	SetPlane(plane, ConstructPlaneFromPoints(view_position, p0, p1));
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
	
	for (size_t i = 0U; i < m_planecount; ++i)
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