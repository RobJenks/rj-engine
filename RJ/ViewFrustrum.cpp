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
	memset(&m_planes, 0, sizeof(D3DXPLANE) * 6);
	memset(&m_farplaneworld, 0, sizeof(D3DXFINITEPLANE));
	m_projection = m_frustrumproj = ID_MATRIX;
	m_clip_near = 1.0f; m_clip_far = 1000.0f;
	m_aspect = (1024.0f / 768.0f);
	m_fov = (PI * 0.25f);
	m_fovtan = tanf(m_fov * 0.5f);
}

// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
Result ViewFrustrum::Initialise(const D3DXMATRIX *projection, const float depth, const float FOV, const float aspect)
{
	// Record the key values within this class 
	m_projection = D3DXMATRIX(*(projection));
	m_clip_far = depth;
	m_aspect = aspect;

	// Precalculate values around FOV for efficiency at render time
	m_fov = FOV;
	m_fovtan = tanf(m_fov * 0.5f);

	// Calculate the minimum z distance in the frustrum and generate a frustrum-specific proj
	m_frustrumproj = D3DXMATRIX(m_projection);
	m_clip_near = -m_frustrumproj._43 / m_frustrumproj._33;
	float r = m_clip_far / (m_clip_far - m_clip_near);
	m_frustrumproj._33 = r;
	m_frustrumproj._43 = -r * m_clip_near;

	// Return success
	return ErrorCodes::NoError;
}

void ViewFrustrum::ConstructFrustrum(const D3DXMATRIX *view, const D3DXMATRIX *invview)
{
	D3DXMATRIX matrix;

	// Calculate the frustrum matrix based on the current view matrix and precalculated adjusted projection matrix
	D3DXMatrixMultiply(&matrix, view, &m_frustrumproj);

	// Calculate near plane of frustum.
	m_planes[0].a = matrix._14 + matrix._13;
	m_planes[0].b = matrix._24 + matrix._23;
	m_planes[0].c = matrix._34 + matrix._33;
	m_planes[0].d = matrix._44 + matrix._43;
	D3DXPlaneNormalize(&m_planes[0], &m_planes[0]);

	// Calculate far plane of frustum.
	m_planes[1].a = matrix._14 - matrix._13; 
	m_planes[1].b = matrix._24 - matrix._23;
	m_planes[1].c = matrix._34 - matrix._33;
	m_planes[1].d = matrix._44 - matrix._43;
	D3DXPlaneNormalize(&m_planes[1], &m_planes[1]);

	// Calculate left plane of frustum.
	m_planes[2].a = matrix._14 + matrix._11; 
	m_planes[2].b = matrix._24 + matrix._21;
	m_planes[2].c = matrix._34 + matrix._31;
	m_planes[2].d = matrix._44 + matrix._41;
	D3DXPlaneNormalize(&m_planes[2], &m_planes[2]);

	// Calculate right plane of frustum.
	m_planes[3].a = matrix._14 - matrix._11; 
	m_planes[3].b = matrix._24 - matrix._21;
	m_planes[3].c = matrix._34 - matrix._31;
	m_planes[3].d = matrix._44 - matrix._41;
	D3DXPlaneNormalize(&m_planes[3], &m_planes[3]);

	// Calculate top plane of frustum.
	m_planes[4].a = matrix._14 - matrix._12; 
	m_planes[4].b = matrix._24 - matrix._22;
	m_planes[4].c = matrix._34 - matrix._32;
	m_planes[4].d = matrix._44 - matrix._42;
	D3DXPlaneNormalize(&m_planes[4], &m_planes[4]);

	// Calculate bottom plane of frustum.
	m_planes[5].a = matrix._14 + matrix._12;
	m_planes[5].b = matrix._24 + matrix._22;
	m_planes[5].c = matrix._34 + matrix._32;
	m_planes[5].d = matrix._44 + matrix._42;
	D3DXPlaneNormalize(&m_planes[5], &m_planes[5]);

	// Also calculate the world position of the far plane.  First, use trigonometry to determine the general world positioning
	D3DXMATRIX invproj;
	D3DXMatrixInverse(&invproj, 0, &m_projection);
	m_farplaneworld.TR = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	D3DXVec3TransformCoord(&m_farplaneworld.TR, &m_farplaneworld.TR, &invproj);

	// We can use these values to determine the position of the top-right vertex, and then use symmetry about the 
	// normal to infer the position of the other three vertices.  Note all coordinates are in view space
	//m_farplaneworld.TR = D3DXVECTOR3(tanX, tanY, -1) * fFar;
	m_farplaneworld.BR = D3DXVECTOR3( m_farplaneworld.TR.x, -m_farplaneworld.TR.y, m_farplaneworld.TR.z);
	m_farplaneworld.BL = D3DXVECTOR3(-m_farplaneworld.TR.x, -m_farplaneworld.TR.y, m_farplaneworld.TR.z);
	m_farplaneworld.TL = D3DXVECTOR3(-m_farplaneworld.TR.x,  m_farplaneworld.TR.y, m_farplaneworld.TR.z);

	// Finally multiply these coordinates by the inverse view matrix to get them in world space
	D3DXVec3TransformCoord(&m_farplaneworld.TR, &m_farplaneworld.TR, invview);
	D3DXVec3TransformCoord(&m_farplaneworld.BR, &m_farplaneworld.BR, invview);
	D3DXVec3TransformCoord(&m_farplaneworld.BL, &m_farplaneworld.BL, invview);
	D3DXVec3TransformCoord(&m_farplaneworld.TL, &m_farplaneworld.TL, invview);
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
	m_working_cuboidvertices[0] = D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter - zSize));
	m_working_cuboidvertices[1] = D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize));
	m_working_cuboidvertices[2] = D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize));
	m_working_cuboidvertices[3] = D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize));
	m_working_cuboidvertices[4] = D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize));
	m_working_cuboidvertices[5] = D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize));
	m_working_cuboidvertices[6] = D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize));
	m_working_cuboidvertices[7] = D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize));

	// Now call the internal function to check these vertices against the viewing frustrum
	return CheckRectangleVertices();
}

bool ViewFrustrum::CheckRectangle(const D3DXMATRIX *world, float xsize, float ysize, float zsize)
{
	// Calculate each plane of this rectangle relative to the origin, in model space
	m_working_cuboidvertices[0] = D3DXVECTOR3(-xsize, -ysize, -zsize);
	m_working_cuboidvertices[1] = D3DXVECTOR3( xsize, -ysize, -zsize);
	m_working_cuboidvertices[2] = D3DXVECTOR3(-xsize,  ysize, -zsize);
	m_working_cuboidvertices[3] = D3DXVECTOR3(-xsize, -ysize,  zsize);
	m_working_cuboidvertices[4] = D3DXVECTOR3( xsize,  ysize, -zsize);
	m_working_cuboidvertices[5] = D3DXVECTOR3( xsize, -ysize,  zsize);
	m_working_cuboidvertices[6] = D3DXVECTOR3(-xsize,  ysize,  zsize);
	m_working_cuboidvertices[7] = D3DXVECTOR3( xsize,  ysize,  zsize);

	// Transform the coordinates into world space
	D3DXVec3TransformCoordArray(m_working_cuboidvertices, sizeof(D3DXVECTOR3), m_working_cuboidvertices, sizeof(D3DXVECTOR3), world, 8);

	// Call the internal function to check these world-space coordinates against the view frustrum
	return CheckRectangleVertices();
}


bool ViewFrustrum::CheckRectangleVertices(void)
{
	// Check if any of the planes of the rectangle are inside the view frustum.
	for(int i = 0; i < 6; i++)
	{
		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[0]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[1]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[2]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[3]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[4]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[5]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[6]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &m_working_cuboidvertices[7]) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool ViewFrustrum::CheckRectangleVertices(const D3DXVECTOR3 rectverts[8])
{
	// Check if any of the planes of the rectangle are inside the view frustum.
	for(int i = 0; i < 6; i++)
	{
		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[0]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[1]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[2]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[3]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[4]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[5]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[6]) >= 0.0f)
			continue;

		if(D3DXPlaneDotCoord(&m_planes[i], &rectverts[7]) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

// Performs the relevant checks depending on bounding object type
bool ViewFrustrum::CheckBoundingObject(const D3DXVECTOR3 *pos, const D3DXMATRIX *world, BoundingObject *obj)
{
	if (!obj) return false;
	switch (obj->GetType())
	{
		case BoundingObject::Type::Point:
			return CheckPoint(pos);
			break;

		case BoundingObject::Type::Cube:
			return CheckCube(pos->x, pos->y, pos->z, obj->GetCubeRadius());
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
