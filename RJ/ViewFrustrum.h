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

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
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
	Result Initialise(const FXMMATRIX projection, const float depth, const float FOV, const float aspect);

	// Builds a new frustrum for the current frame
	void ConstructFrustrum(const FXMMATRIX view, const CXMMATRIX invview);

	CMPINLINE float			GetFOV(void) const				{ return m_fov; }
	CMPINLINE float			GetTanOfHalfFOV(void) const		{ return m_fovtan; }
	CMPINLINE float			GetNearClipPlane(void) const	{ return m_clip_near; }
	CMPINLINE float			GetFarClipPlane(void) const		{ return m_clip_far; }

	// Primary method for object visibility testing
	//CMPINLINE bool TestObjectVisibility(iObject *obj) { return (obj ? CheckSphere(&(obj->GetPosition()), obj->GetCollisionSphereRadius()) : false); }
	CMPINLINE bool TestObjectVisibility(const iObject *obj)
	{
		// Get the object centre and collision radius
		if (!obj) return false;
		XMVECTOR centre = XMLoadFloat3(obj->GetPosition());
		XMVECTOR neg_radius = XMVectorReplicate(-(obj->GetCollisionSphereRadius()));
		
		// If the sphere is 'behind' any plane of the view frustum then return false immediately
		// Otherwise, it is in view.  Loop unrolled for efficiency
		
		if (XMVector2Less(XMPlaneDotCoord(m_planes[0], centre), neg_radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[0], centre), neg_radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[1], centre), neg_radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[2], centre), neg_radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[3], centre), neg_radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[4], centre), neg_radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[5], centre), neg_radius)) return false;
		return true;
	}

	// Check whether a point lies within the frustum
	bool ViewFrustrum::CheckPoint(const FXMVECTOR pt)
	{
		// Loop unrolled for efficiency; if the point lies outside any of the six frustum planes then return false immediately
		if (XMVector2Less(XMPlaneDotCoord(m_planes[0], pt), NULL_VECTOR2)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[1], pt), NULL_VECTOR2)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[2], pt), NULL_VECTOR2)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[3], pt), NULL_VECTOR2)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[4], pt), NULL_VECTOR2)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[5], pt), NULL_VECTOR2)) return false;

		// Point lines inside all planes, therefore is inside the frustum
		return true;
	}

	bool CheckCube(const FXMVECTOR centre, float radius);
	bool CheckCube(float x, float y, float z, float radius);

	bool CheckOBB(const OrientedBoundingBox & obb);

	bool CheckRectangle(float x, float y, float z, float xsize, float ysize, float zsize);
	bool CheckRectangle(const FXMMATRIX world, float xsize, float ysize, float zsize);

	bool CheckBoundingObject(const FXMVECTOR pos, const CXMMATRIX world, BoundingObject *obj);

	CMPINLINE bool CheckSphere(const FXMVECTOR centre, float f_radius)
	{
		// If the sphere is 'behind' any plane of the view frustum then return false immediately
		// Loop unrolled for efficiency
		XMVECTOR radius = XMVectorReplicate(-f_radius);
		if (XMVector2Less(XMPlaneDotCoord(m_planes[0], centre), radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[1], centre), radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[2], centre), radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[3], centre), radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[4], centre), radius)) return false;
		if (XMVector2Less(XMPlaneDotCoord(m_planes[5], centre), radius)) return false;
		return true;
	}

	CMPINLINE XMVECTOR GetPlane(FrustrumPlane plane) { return m_planes[(int)plane]; }

	CMPINLINE XMVECTOR GetNearPlane(void)const		{ return m_planes[(int)FrustrumPlane::NearPlane]; }
	CMPINLINE XMVECTOR GetFarPlane(void) const		{ return m_planes[(int)FrustrumPlane::FarPlane]; }
	CMPINLINE XMVECTOR GetLeftPlane(void) const		{ return m_planes[(int)FrustrumPlane::LeftPlane]; }
	CMPINLINE XMVECTOR GetRightPlane(void) const	{ return m_planes[(int)FrustrumPlane::RightPlane]; }
	CMPINLINE XMVECTOR GetTopPlane(void) const		{ return m_planes[(int)FrustrumPlane::TopPlane]; }
	CMPINLINE XMVECTOR GetBottomPlane(void) const	{ return m_planes[(int)FrustrumPlane::BottomPlane]; }

	const CMPINLINE D3DXFINITEPLANE *GetFiniteFarPlane(void) const { return &m_farplaneworld; }

private:
	// The pure mathematical planes that represent this frustrum (stored as ax+by+cz+dw = 0)
	XMVECTOR			m_planes[6];

	// The finite world plane that represents the visible far frustrum plane
	D3DXFINITEPLANE		m_farplaneworld;

	// Projection matrix and other viewport data
	XMMATRIX			m_projection;
	float				m_clip_near, m_clip_far, m_aspect;
	float				m_fov, m_fovtan;			// m_fovtan = tanf(FOV * 0.5f)
	XMMATRIX			m_frustrumproj;				// Frustrum-specific proj matrix, preacalculate at initialisation


	// Private method to perform the visibility test on pre-transformed rectangle vertices
	bool CheckRectangleVertices(void);
	bool CheckRectangleVertices(const XMVECTOR rectverts[8]);

	// Private temporary storage for storing cuboid vertices during visibility testing
	XMVECTOR			m_working_cuboidvertices[8];
};



#endif