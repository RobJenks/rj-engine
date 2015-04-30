#pragma once

#ifndef __ViewFrustrumH__
#define __ViewFrustrumH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "Utility.h"
#include "iObject.h"
class BoundingObject;

// Algorithm in "D3DXPlaneDotCoord" defined as a macro here for efficiency, since the visibility check method 
// is used thousands of times per frame and needs to be as fast as possible
#define DOT_PLANE_COORD(plane, coord) (plane.a * coord.x + plane.b * coord.y + plane.c * coord.z + plane.d)
#define DOT_PLANE_PCOORD(plane, coord) (plane.a * coord->x + plane.b * coord->y + plane.c * coord->z + plane.d)

class ViewFrustrum
{
public:
	enum FrustrumPlane {
		NearPlane = 0,
		FarPlane = 1,
		LeftPlane = 2,
		RightPlane = 3,
		TopPlane = 4,
		BottomPlane = 5
	};

	ViewFrustrum(void);
	~ViewFrustrum(void);

	// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
	Result Initialise(const D3DXMATRIX *projection, const float depth, const float FOV, const float aspect);

	// Builds a new frustrum for the current frame
	void ConstructFrustrum(const D3DXMATRIX *view, const D3DXMATRIX *invview);

	CMPINLINE float			GetFOV(void)				{ return m_fov; }
	CMPINLINE float			GetTanOfHalfFOV(void)		{ return m_fovtan; }

	// Primary method for object visibility testing
	//CMPINLINE bool TestObjectVisibility(iObject *obj) { return (obj ? CheckSphere(&(obj->GetPosition()), obj->GetCollisionSphereRadius()) : false); }
	CMPINLINE bool TestObjectVisibility(const iObject *obj)
	{
		// Get the object centre and collision radius
		if (!obj) return false;
		const D3DXVECTOR3 & centre = obj->GetPosition();
		float neg_radius = -(obj->GetCollisionSphereRadius());

		// If the sphere is 'behind' any plane of the view frustum then return false immediately
		// Otherwise, it is in view.  Loop unrolled for efficiency
		if (DOT_PLANE_COORD(m_planes[0], centre) < neg_radius) return false;
		if (DOT_PLANE_COORD(m_planes[1], centre) < neg_radius) return false;
		if (DOT_PLANE_COORD(m_planes[2], centre) < neg_radius) return false;
		if (DOT_PLANE_COORD(m_planes[3], centre) < neg_radius) return false;
		if (DOT_PLANE_COORD(m_planes[4], centre) < neg_radius) return false;
		if (DOT_PLANE_COORD(m_planes[5], centre) < neg_radius) return false;
		return true;
	}

	bool CheckPoint(const D3DXVECTOR3 *pt);
	bool CheckCube(float x, float y, float z, float radius);

	bool CheckOBB(const OrientedBoundingBox & obb);

	bool CheckRectangle(float x, float y, float z, float xsize, float ysize, float zsize);
	bool CheckRectangle(const D3DXMATRIX *world, float xsize, float ysize, float zsize);

	bool CheckBoundingObject(const D3DXVECTOR3 *pos, const D3DXMATRIX *world, BoundingObject *obj);

	CMPINLINE bool CheckSphere(const D3DXVECTOR3 & centre, float radius)
	{
		// If the sphere is 'behind' any plane of the view frustum then return false immediately
		// Loop unrolled for efficiency
		radius = -radius;
		if (DOT_PLANE_COORD(m_planes[0], centre) < radius) return false;
		if (DOT_PLANE_COORD(m_planes[1], centre) < radius) return false;
		if (DOT_PLANE_COORD(m_planes[2], centre) < radius) return false;
		if (DOT_PLANE_COORD(m_planes[3], centre) < radius) return false;
		if (DOT_PLANE_COORD(m_planes[4], centre) < radius) return false;
		if (DOT_PLANE_COORD(m_planes[5], centre) < radius) return false;
		return true;
	}

	CMPINLINE bool CheckSphere(const D3DXVECTOR3 * centre, float radius)
	{
		// If the sphere is 'behind' any plane of the view frustum then return false immediately
		// Loop unrolled for efficiency
		radius = -radius;
		if (DOT_PLANE_PCOORD(m_planes[0], centre) < radius) return false;
		if (DOT_PLANE_PCOORD(m_planes[1], centre) < radius) return false;
		if (DOT_PLANE_PCOORD(m_planes[2], centre) < radius) return false;
		if (DOT_PLANE_PCOORD(m_planes[3], centre) < radius) return false;
		if (DOT_PLANE_PCOORD(m_planes[4], centre) < radius) return false;
		if (DOT_PLANE_PCOORD(m_planes[5], centre) < radius) return false;
		return true;
	}

	/*CMPINLINE bool CheckSphere(const D3DXVECTOR3 *centre, float radius)
	{
		// If the sphere is 'behind' any plane of the view frustum then return false immediately
		for (int i = 0; i < 6; ++i)
			if (D3DXPlaneDotCoord(&m_planes[i], centre) < -radius)
				return false;

		return true;
	}*/

	const CMPINLINE D3DXPLANE *GetPlane(FrustrumPlane plane) { return &m_planes[(int)plane]; }

	const CMPINLINE D3DXPLANE *GetNearPlane(void)const		{ return &m_planes[(int)FrustrumPlane::NearPlane]; }
	const CMPINLINE D3DXPLANE *GetFarPlane(void) const		{ return &m_planes[(int)FrustrumPlane::FarPlane]; }
	const CMPINLINE D3DXPLANE *GetLeftPlane(void) const		{ return &m_planes[(int)FrustrumPlane::LeftPlane]; }
	const CMPINLINE D3DXPLANE *GetRightPlane(void) const	{ return &m_planes[(int)FrustrumPlane::RightPlane]; }
	const CMPINLINE D3DXPLANE *GetTopPlane(void) const		{ return &m_planes[(int)FrustrumPlane::TopPlane]; }
	const CMPINLINE D3DXPLANE *GetBottomPlane(void) const	{ return &m_planes[(int)FrustrumPlane::BottomPlane]; }

	const CMPINLINE D3DXFINITEPLANE *GetFiniteFarPlane(void) const { return &m_farplaneworld; }

private:
	// The pure mathematical planes that represent this frustrum (stored as ax+by+cz+dw = 0)
	D3DXPLANE			m_planes[6];

	// The finite world plane that represents the visible far frustrum plane
	D3DXFINITEPLANE		m_farplaneworld;

	// Projection matrix and other viewport data
	D3DXMATRIX			m_projection;
	float				m_depth, m_aspect;
	float				m_fov, m_fovtan;			// m_fovtan = tanf(FOV * 0.5f)
	D3DXMATRIX			m_frustrumproj;				// Frustrum-specific proj matrix, preacalculate at initialisation


	// Private method to perform the visibility test on pre-transformed rectangle vertices
	bool CheckRectangleVertices(void);
	bool CheckRectangleVertices(const D3DXVECTOR3 rectverts[8]);

	// Private temporary storage for storing cuboid vertices during visibility testing
	D3DXVECTOR3			m_working_cuboidvertices[8];
};



#endif