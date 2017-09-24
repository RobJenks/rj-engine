#include "DX11_Core.h"
#include "FastMath.h"
#include "iObject.h"
#include "Frustum.h"
using namespace DirectX;

// Static constant data
const size_t Frustum::NEAR_PLANE = 0U;						// Index into the plane collection
const size_t Frustum::FAR_PLANE = 1U;						// Index into the plane collection
const size_t Frustum::FIRST_SIDE = 2U;						// Index into the plane collection

// Temporary storage for construction of cuboid vertices during visibility testing
AXMVECTOR_P Frustum::m_working_cuboidvertices[8];


// Construct a new frustum with the specified number of sides (not including the near- & far-planes)
Frustum::Frustum(const size_t frustum_side_count)
{
	// Must have at least 3 sides.  Also impose a reasonable upper limit on complexity of the frustum
	assert(frustum_side_count >= 3U);
	assert(frustum_side_count < 36U);

	// Allocate space for the data
	m_planecount = frustum_side_count + 2U;			// +2 for the near- and far-planes
	m_planes = new AXMVECTOR[m_planecount];

	// Initialise all data
	for (size_t i = 0U; i < m_planecount; ++i)
	{
		m_planes[i] = NULL_VECTOR;
	}
}

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

// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
// Generally only applicable for the primary view frustum
Result XM_CALLCONV Frustum::InitialiseAsViewFrustum(const FXMMATRIX projection, const float depth, const float FOV, const float aspect)
{
	// Use default near & far clip planes for now, until we need to do otherwise
	m_clip_near = Game::C_DEFAULT_CLIP_NEAR_DISTANCE;
	m_clip_far = Game::C_DEFAULT_CLIP_FAR_DISTANCE;

	// Calculate the minimum z distance in the frustrum and generate a frustrum-specific proj
	XMFLOAT4X4 fproj;
	XMStoreFloat4x4(&fproj, projection);
		m_clip_far = depth;
		m_clip_near = -fproj._43 / fproj._33;
		float r = m_clip_far / (m_clip_far - m_clip_near);
		fproj._33 = r;
		fproj._43 = -r * m_clip_near;
	m_frustrumproj = XMLoadFloat4x4(&fproj);

	// Return success
	return ErrorCodes::NoError;
}

// Builds a new view frustrum based on the current view & inverse view matrices.  Generally only applicable for the primary view frustum
void XM_CALLCONV Frustum::ConstructViewFrustrum(const FXMMATRIX view, const CXMMATRIX invview)
{
	// Calculate the frustrum matrix based on the current view matrix and precalculated adjusted projection matrix
	XMFLOAT4X4 matrix;
	XMMATRIX fproj_view = XMMatrixMultiply(view, m_frustrumproj);
	XMStoreFloat4x4(&matrix, fproj_view);

	// Calculate near plane of frustum.
	SetPlane(Frustum::NEAR_PLANE, XMPlaneNormalize(XMVectorSet(matrix._14 + matrix._13,
		matrix._24 + matrix._23,
		matrix._34 + matrix._33,
		matrix._44 + matrix._43)));

	// Calculate far plane of frustum.
	SetPlane(Frustum::FAR_PLANE, XMPlaneNormalize(XMVectorSet(matrix._14 - matrix._13,
		matrix._24 - matrix._23,
		matrix._34 - matrix._33,
		matrix._44 - matrix._43)));

	// Calculate left plane of frustum.
	SetPlane(2U, XMPlaneNormalize(XMVectorSet(matrix._14 + matrix._11,
		matrix._24 + matrix._21,
		matrix._34 + matrix._31,
		matrix._44 + matrix._41)));

	// Calculate right plane of frustum.
	SetPlane(3U, XMPlaneNormalize(XMVectorSet(matrix._14 - matrix._11,
		matrix._24 - matrix._21,
		matrix._34 - matrix._31,
		matrix._44 - matrix._41)));

	// Calculate top plane of frustum.
	SetPlane(4U, XMPlaneNormalize(XMVectorSet(matrix._14 - matrix._12,
		matrix._24 - matrix._22,
		matrix._34 - matrix._32,
		matrix._44 - matrix._42)));

	// Calculate bottom plane of frustum.
	SetPlane(5U, XMPlaneNormalize(XMVectorSet(matrix._14 + matrix._12,
		matrix._24 + matrix._22,
		matrix._34 + matrix._32,
		matrix._44 + matrix._42)));
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

// Transform the frustum by the given matrix
void Frustum::Transform(const FXMMATRIX transform)
{
	for (size_t i = 0U; i < m_planecount; ++i)
	{
		m_planes[i] = XMPlaneTransform(m_planes[i], transform);
	}
}

// Checks for the intersection of a centre point and negative-vectorised-radius with
// the frustum.  Internal method used as the basis for many public method above
bool Frustum::CheckSphereInternal(const FXMVECTOR centre_point, const FXMVECTOR negated_radius_v) const
{
	// If the sphere is 'behind' any plane of the view frustum then return false immediately
	if (XMVector2Less(XMPlaneDotCoord(m_planes[Frustum::NEAR_PLANE], centre_point), negated_radius_v)) return false;
	if (XMVector2Less(XMPlaneDotCoord(m_planes[Frustum::FAR_PLANE], centre_point), negated_radius_v)) return false;
	for (size_t i = Frustum::FIRST_SIDE; i < m_planecount; ++i)
	{
		if (XMVector2Less(XMPlaneDotCoord(m_planes[i], centre_point), negated_radius_v)) return false;
	}

	// The object was not behind any planes in the frustum so return true
	return true;
}

// Test whether the given bounding sphere lies within the frustum
bool Frustum::CheckSphere(const FXMVECTOR sphere_centre, float sphere_radius) const
{
	// Simply call the internal method with correctly negated & vectorised radius
	XMVECTOR radius = XMVectorReplicate(-sphere_radius);
	return CheckSphereInternal(sphere_centre, radius);
}

// Check whether a point lies within the frustum
bool Frustum::CheckPoint(const FXMVECTOR pt) const
{
	// This is idential to the sphere intersection test with radius == 0
	return CheckSphereInternal(pt, NULL_VECTOR);
}

// Check whether an object lies within the frustum, based upon its collision sphere
bool Frustum::TestObjectVisibility(const iObject *obj) const
{
	// Call the internal method with collision sphere data from the object
	if (!obj) return false;
	return CheckSphereInternal(obj->GetPosition(), XMVectorReplicate(-(obj->GetCollisionSphereRadius())));
}

// Check whether the given cuboid lies within the frustum
bool Frustum::CheckCuboid(const FXMVECTOR centre, const FXMVECTOR size) const
{
	XMVECTOR neg_size = XMVectorNegate(size);

	// The cuboid intersects this frustum if any vertex is visible
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_0000)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_1000)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_0100)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_0010)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_1100)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_1010)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_0110)))) return true;
	if (CheckPoint(XMVectorAdd(centre, XMVectorSelect(size, neg_size, VCTRL_1110)))) return true;

	// No vertices are visible
	return false;
}

// Check whether the given OBB lies within the frustum
bool Frustum::CheckOBB(const OrientedBoundingBox & obb) const
{
	obb.DetermineVertices(Frustum::m_working_cuboidvertices);

	// If any vertex intersects this frustum then the OBB is visible
	if (CheckPoint(Frustum::m_working_cuboidvertices[0].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[1].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[2].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[3].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[4].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[5].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[6].value)) return true;
	if (CheckPoint(Frustum::m_working_cuboidvertices[7].value)) return true;

	// No vertex is visible
	return false;
}



// Destructor
Frustum::~Frustum(void)
{
	// Release any allocated resources
	if (m_planes) SafeDeleteArray(m_planes);
	m_planecount = 0U;
}