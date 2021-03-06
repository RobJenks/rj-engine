#pragma once

#include "DX11_Core.h"
#include "FastMath.h"
class iObject;
class OrientedBoundingBox;

class Frustum
{
public:

	// Static constant data
	static const size_t					NEAR_PLANE;						// Index into the plane collection
	static const size_t					FAR_PLANE;						// Index into the plane collection
	static const size_t					FIRST_SIDE;						// Index into the plane collection


	// Construct a new frustum with the specified number of sides (not including the near- & far-planes)
	Frustum(const size_t frustum_side_count);

	// Construct a new frustum with the specified number of sides (not including the near- & far-planes)
	// Pass the near- and far-planes in during construction since they do not need to be calculated again
	Frustum(const size_t frustum_side_count, const FXMVECTOR near_plane, const FXMVECTOR far_plane);

	// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
	// Generally only applicable for the primary view frustum
	Result RJ_XM_CALLCONV					InitialiseAsViewFrustum(const FXMMATRIX projection, const float far_plane_distance, 
																const float FOV, const float aspect);

	// Copies view frustum data from an existing frustum
	void								CopyViewFrustumData(const Frustum & view_frustum);

	// Builds a new view frustrum based on the current view & inverse view matrices.  Generally only applicable for the primary view frustum
	// Note frustum must have either been initialised as a view frustum via InitialiseAsViewFrustum, or must have copied relevant data from 
	// a valid view frustum via CopyViewFrustumData, before it can construct a valid frustum from view data
	void RJ_XM_CALLCONV					ConstructViewFrustrum(const FXMMATRIX view, const CXMMATRIX invview);

	// Add a new side to the frustum, based upon a viewer position and two further points in the world
	// Result will be a triangular plane from the viewer with far edge between the two world vertices, 
	// forming one side of the eventual frustum pyramid
	void								SetPlane(size_t plane, const FXMVECTOR view_position, const FXMVECTOR p0, const FXMVECTOR p1);

	// Add a new side to the frustum by providing the plane coefficients directly
	void								SetPlane(size_t plane, const FXMVECTOR plane_coeff);

	// Retrieve data on the planes that make up this frustum
	CMPINLINE size_t					GetPlaneCount(void) const		{ return m_planecount; }
	CMPINLINE size_t					GetSideCount(void) const		{ return (m_planecount - 2U); }
	CMPINLINE const XMVECTOR *			GetPlanes(void) const			{ return m_planes; }
	CMPINLINE const XMVECTOR &			GetPlane(size_t plane) const	{ return m_planes[plane]; }
	CMPINLINE const XMVECTOR &			GetNearPlane(void) const		{ return m_planes[Frustum::NEAR_PLANE]; }
	CMPINLINE const XMVECTOR &			GetFarPlane(void) const			{ return m_planes[Frustum::FAR_PLANE]; }

	
	// Transform the frustum by the given matrix
	void								Transform(const FXMMATRIX transform);

	// Sets this frustum to a transformed version of the given frustum.  This can only 
	// be performed between frustums of the same cardinatlity
	void								SetTransformed(const Frustum & frustum, const FXMMATRIX transform);

	/*** Intersection testing methods ***/

	// Test whether the given bounding sphere lies within the frustum
	bool								CheckSphere(const FXMVECTOR sphere_centre, float sphere_radius) const;

	// Check whether a point lies within the frustum
	bool								CheckPoint(const FXMVECTOR pt) const;

	// Check whether an object lies within the frustum, based upon its collision sphere
	bool								TestObjectVisibility(const iObject *obj) const;

	// Check whether the given cuboid lies within the frustum
	bool								CheckCuboid(const FXMVECTOR centre, const FXMVECTOR size) const;
	
	// Check whether the given OBB lies within the frustum
	bool								CheckOBB(const OrientedBoundingBox & obb) const;

	// Determine the world-space coordinates of the frustum corners.  Relevant ONLY for a view frustum
	void								DetermineWorldSpaceCorners(XMVECTOR(&pOutVertices)[8]) const;

	// Return auxilliary data on the view frustum
	CMPINLINE float						GetNearClipPlaneDistance(void) const { return m_clip_near; }
	CMPINLINE float						GetFarClipPlaneDistance(void) const { return m_clip_far; }
	CMPINLINE XMMATRIX					GetFrustumProjectionMatrix(void) const { return m_proj; }
	CMPINLINE XMMATRIX					GetFrustumViewProjectionMatrix(void) const { return m_viewproj; }

	// Destructor
	~Frustum(void);


private:

	// Collection of planes that make up the frustum; [0] is always near plane 
	// and [1] is the far plane.  Plane count will be N+2 for an N-sided frustum
	AXMVECTOR *							m_planes;
	size_t								m_planecount;

	// Checks for the intersection of a centre point and negative-vectorised-radius with
	// the frustum.  Internal method used as the basis for many public method above
	bool								CheckSphereInternal(const FXMVECTOR centre_point, const FXMVECTOR negated_radius_v) const;

	// Other auxilliary frustum data
	float								m_clip_near, m_clip_far;
	AXMMATRIX							m_proj;						// Frustrum-specific proj matrix, preacalculated at initialisation
	AXMMATRIX							m_viewproj;					// View-projection for the frustum, calculated on each ConstructFrustum

	// Temporary storage for construction of cuboid vertices during visibility testing
	static AXMVECTOR_P					m_working_cuboidvertices[8];

};