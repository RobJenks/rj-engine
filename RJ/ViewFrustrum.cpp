#include "DX11_Core.h"

#include <cmath>
#include "ErrorCodes.h"
#include "Utility.h"
#include "iSpaceObject.h"
#include "BoundingObject.h"
#include "OrientedBoundingBox.h"

#include "ViewFrustrum.h"


// Default constructor
ViewFrustrum::ViewFrustrum(void)
{
	// Set all fields to defaults
	for (int i = 0; i < 6; ++i) m_planes[i].value = XMVectorZero();
	m_farplaneworld.TL = XMVectorZero();
	m_farplaneworld.TR = XMVectorZero();
	m_farplaneworld.BL = XMVectorZero();
	m_farplaneworld.BR = XMVectorZero();
	m_projection = m_frustrumproj = ID_MATRIX;
	m_clip_near = 1.0f; m_clip_far = 1000.0f;
	m_aspect = (1024.0f / 768.0f);
	m_fov = (PI * 0.25f);
	m_fovtan = tanf(m_fov * 0.5f);
}

// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
Result ViewFrustrum::Initialise(const FXMMATRIX projection, const float depth, const float FOV, const float aspect)
{
	// Record the key values within this class 
	m_projection = projection;
	m_clip_far = depth;
	m_aspect = aspect;

	// Precalculate values around FOV for efficiency at render time
	m_fov = FOV;
	m_fovtan = tanf(m_fov * 0.5f);

	// Calculate the minimum z distance in the frustrum and generate a frustrum-specific proj
	XMFLOAT4X4 fproj;
	XMStoreFloat4x4(&fproj, m_projection);
		m_clip_near = -fproj._43 / fproj._33;
		float r = m_clip_far / (m_clip_far - m_clip_near);
		fproj._33 = r;
		fproj._43 = -r * m_clip_near;
	m_frustrumproj = XMLoadFloat4x4(&fproj);

	// Return success
	return ErrorCodes::NoError;
}

void ViewFrustrum::ConstructFrustrum(const FXMMATRIX view, const CXMMATRIX invview)
{
	// Calculate the frustrum matrix based on the current view matrix and precalculated adjusted projection matrix
	XMFLOAT4X4 matrix;
	XMMATRIX fproj_view = XMMatrixMultiply(view, m_frustrumproj);
	XMStoreFloat4x4(&matrix, fproj_view);

	// Calculate near plane of frustum.
	m_planes[0].value = XMPlaneNormalize(XMVectorSet(	matrix._14 + matrix._13,
														matrix._24 + matrix._23, 
														matrix._34 + matrix._33, 
														matrix._44 + matrix._43));

	// Calculate far plane of frustum.
	m_planes[1].value = XMPlaneNormalize(XMVectorSet(	matrix._14 - matrix._13,
														matrix._24 - matrix._23,
														matrix._34 - matrix._33,
														matrix._44 - matrix._43));

	// Calculate left plane of frustum.
	m_planes[2].value = XMPlaneNormalize(XMVectorSet(	matrix._14 + matrix._11,
														matrix._24 + matrix._21,
														matrix._34 + matrix._31,
														matrix._44 + matrix._41));

	// Calculate right plane of frustum.
	m_planes[3].value = XMPlaneNormalize(XMVectorSet(	matrix._14 - matrix._11,
														matrix._24 - matrix._21,
														matrix._34 - matrix._31,
														matrix._44 - matrix._41));

	// Calculate top plane of frustum.
	m_planes[4].value = XMPlaneNormalize(XMVectorSet(	matrix._14 - matrix._12,
														matrix._24 - matrix._22,
														matrix._34 - matrix._32,
														matrix._44 - matrix._42));

	// Calculate bottom plane of frustum.
	m_planes[5].value = XMPlaneNormalize(XMVectorSet(	matrix._14 + matrix._12,
														matrix._24 + matrix._22,
														matrix._34 + matrix._32,
														matrix._44 + matrix._42));

	// Also calculate the world position of the far plane.  First, use trigonometry to determine the general world positioning
	XMMATRIX invproj = XMMatrixInverse(NULL, m_projection);
	m_farplaneworld.TR = XMVector3TransformCoord(ONE_VECTOR, invproj);

	// We can use these values to determine the position of the top-right vertex, and then use symmetry about the 
	// normal to infer the position of the other three vertices.  Note all coordinates are in view space
	XMFLOAT3 tr; XMStoreFloat3(&tr, m_farplaneworld.TR);
	m_farplaneworld.BR = XMVectorSet( tr.x, -tr.y, tr.z, 0.0f);
	m_farplaneworld.BL = XMVectorSet(-tr.x, -tr.y, tr.z, 0.0f);
	m_farplaneworld.TL = XMVectorSet(-tr.x, tr.y, tr.z, 0.0f);

	// Finally multiply these coordinates by the inverse view matrix to get them in world space
	m_farplaneworld.TR = XMVector3TransformCoord(m_farplaneworld.TR, invview);
	m_farplaneworld.BR = XMVector3TransformCoord(m_farplaneworld.BR, invview);
	m_farplaneworld.BL = XMVector3TransformCoord(m_farplaneworld.BL, invview);
	m_farplaneworld.TL = XMVector3TransformCoord(m_farplaneworld.TL, invview);
}

bool ViewFrustrum::CheckCube(const FXMVECTOR centre, float radius)
{
	// Call the rectangle method with equal x/y/z radii
	XMFLOAT3 fcentre; XMStoreFloat3(&fcentre, centre);
	return CheckRectangle(fcentre.x, fcentre.y, fcentre.z, radius, radius, radius);

}
bool ViewFrustrum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	// Call the rectangle method with equal x/y/z radii
	return CheckRectangle(xCenter, yCenter, zCenter, radius, radius, radius);
}

bool ViewFrustrum::CheckOBB(const OrientedBoundingBox & obb)
{
	// Generate the vertices for this OBB based on its current position and orientation in world space
	obb.DetermineVertices(m_working_cuboidvertices);

	// Test these vertices for visibility
	return CheckRectangleVertices();
}

bool ViewFrustrum::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
	// Calculate each plane of this rectangle based on supplied parameters. Saves doing this 6x in the loop
	m_working_cuboidvertices[0].value = XMVectorSet((xCenter - xSize), (yCenter - ySize), (zCenter - zSize), 0.0f);
	m_working_cuboidvertices[1].value = XMVectorSet((xCenter + xSize), (yCenter - ySize), (zCenter - zSize), 0.0f);
	m_working_cuboidvertices[2].value = XMVectorSet((xCenter - xSize), (yCenter + ySize), (zCenter - zSize), 0.0f);
	m_working_cuboidvertices[3].value = XMVectorSet((xCenter - xSize), (yCenter - ySize), (zCenter + zSize), 0.0f);
	m_working_cuboidvertices[4].value = XMVectorSet((xCenter + xSize), (yCenter + ySize), (zCenter - zSize), 0.0f);
	m_working_cuboidvertices[5].value = XMVectorSet((xCenter + xSize), (yCenter - ySize), (zCenter + zSize), 0.0f);
	m_working_cuboidvertices[6].value = XMVectorSet((xCenter - xSize), (yCenter + ySize), (zCenter + zSize), 0.0f);
	m_working_cuboidvertices[7].value = XMVectorSet((xCenter + xSize), (yCenter + ySize), (zCenter + zSize), 0.0f);

	// Now call the internal function to check these vertices against the viewing frustrum
	return CheckRectangleVertices();
}

bool ViewFrustrum::CheckRectangle(const FXMMATRIX world, float xsize, float ysize, float zsize)
{
	// Calculate each plane of this rectangle relative to the origin, in model space, then transform into world space
	m_working_cuboidvertices[0].value = XMVector2TransformCoord(XMVectorSet(-xsize, -ysize, -zsize, 0.0f), world);
	m_working_cuboidvertices[1].value = XMVector2TransformCoord(XMVectorSet(xsize, -ysize, -zsize, 0.0f), world);
	m_working_cuboidvertices[2].value = XMVector2TransformCoord(XMVectorSet(-xsize, ysize, -zsize, 0.0f), world);
	m_working_cuboidvertices[3].value = XMVector2TransformCoord(XMVectorSet(-xsize, -ysize, zsize, 0.0f), world);
	m_working_cuboidvertices[4].value = XMVector2TransformCoord(XMVectorSet(xsize, ysize, -zsize, 0.0f), world);
	m_working_cuboidvertices[5].value = XMVector2TransformCoord(XMVectorSet(xsize, -ysize, zsize, 0.0f), world);
	m_working_cuboidvertices[6].value = XMVector2TransformCoord(XMVectorSet(-xsize, ysize, zsize, 0.0f), world);
	m_working_cuboidvertices[7].value = XMVector2TransformCoord(XMVectorSet(xsize, ysize, zsize, 0.0f), world);

	// Call the internal function to check these world-space coordinates against the view frustrum
	return CheckRectangleVertices();
}


bool ViewFrustrum::CheckRectangleVertices(void)
{
	// Check if any of the planes of the rectangle are inside the view frustum.
	for(int i = 0; i < 6; i++)
	{
		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[0].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[1].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[2].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[3].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[4].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[5].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[6].value), NULL_VECTOR2))
			continue;

		if(XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, m_working_cuboidvertices[7].value), NULL_VECTOR2))
			continue;

		return false;
	}

	return true;
}

bool ViewFrustrum::CheckRectangleVertices(const AXMVECTOR_P rectverts[8])
{
	// Check if any of the planes of the rectangle are inside the view frustum.
	for(int i = 0; i < 6; i++)
	{
		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[0].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[1].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[2].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[3].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[4].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[5].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[6].value), NULL_VECTOR2))
			continue;

		if (XMVector2GreaterOrEqual(XMPlaneDotCoord(m_planes[i].value, rectverts[7].value), NULL_VECTOR2))
			continue;

		return false;
	}

	return true;
}

// Performs the relevant checks depending on bounding object type
bool ViewFrustrum::CheckBoundingObject(const FXMVECTOR pos, const CXMMATRIX world, BoundingObject *obj)
{
	if (!obj) return false;
	switch (obj->GetType())
	{
		case BoundingObject::Type::Point:
			return CheckPoint(pos);
			break;

		case BoundingObject::Type::Cube:
			return CheckCube(pos, obj->GetCubeRadius());
			break;

		case BoundingObject::Type::Sphere:
			return CheckSphere(pos, obj->GetSphereRadius());
			break;

		case BoundingObject::Type::Cuboid:
			return CheckRectangle(world, obj->GetCuboidXSize(), obj->GetCuboidYSize(), obj->GetCuboidZSize());
			break;

		default:
			return false;
	}
}

// Default destructor
ViewFrustrum::~ViewFrustrum(void)
{
	// Deallocate all memory held within this object

}
