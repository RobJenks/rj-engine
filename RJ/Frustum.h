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
	// Pass the near- and far-planes in during construction since they do not need to be calculated again
	Frustum(const size_t frustum_side_count, const FXMVECTOR near_plane, const FXMVECTOR far_plane);

	// Add a new side to the frustum, based upon a viewer position and two further points in the world
	// Result will be a triangular plane from the viewer with far edge between the two world vertices, 
	// forming one side of the eventual frustum pyramid
	void								SetPlane(size_t plane, const FXMVECTOR view_position, const FXMVECTOR p0, const FXMVECTOR p1);

	// Add a new side to the frustum by providing the plane coefficients directly
	void								SetPlane(size_t plane, const FXMVECTOR plane_coeff);

	// Retrieve data on the planes that make up this frustum
	CMPINLINE size_t					GetPlaneCount(void) const		{ return m_planecount; }
	CMPINLINE const XMVECTOR *			GetPlanes(void) const			{ return m_planes; }
	CMPINLINE const XMVECTOR &			GetPlane(size_t plane)			{ return m_planes[plane]; }
	CMPINLINE const XMVECTOR &			GetNearPlane(void) const		{ return m_planes[Frustum::NEAR_PLANE]; }
	CMPINLINE const XMVECTOR &			GetFarPlane(void) const			{ return m_planes[Frustum::FAR_PLANE]; }


	/*** Intersection testing methods ***/

	// Test whether the given bounding sphere lies within the frustum
	bool								CheckSphere(const FXMVECTOR sphere_centre, float sphere_radius);

	// Check whether a point lies within the frustum
	bool								CheckPoint(const FXMVECTOR pt);

	// Check whether an object lies within the frustum, based upon its collision sphere
	bool								TestObjectVisibility(const iObject *obj);

	// Check whether the given cuboid lies within the frustum
	bool								CheckCuboid(const FXMVECTOR centre, const FXMVECTOR size);
	
	// Check whether the given OBB lies within the frustum
	bool								CheckOBB(const OrientedBoundingBox & obb);

	// Destructor
	~Frustum(void);


private:

	// Collection of planes that make up the frustum; [0] is always near plane 
	// and [1] is the far plane.  Plane count will be N+2 for an N-sided frustum
	AXMVECTOR *							m_planes;
	size_t								m_planecount;

	// Checks for the intersection of a centre point and negative-vectorised-radius with
	// the frustum.  Internal method used as the basis for many public method above
	bool								CheckSphereInternal(const FXMVECTOR centre_point, const FXMVECTOR negated_radius_v);

	// Temporary storage for construction of cuboid vertices during visibility testing
	static AXMVECTOR_P					m_working_cuboidvertices[8];

};