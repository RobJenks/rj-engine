#include "DX11_Core.h"

#include <cmath>
#include "ErrorCodes.h"
#include "Utility.h"
#include "iSpaceObject.h"
#include "BoundingObject.h"
#include "OrientedBoundingBox.h"

#include "ViewFrustrum.h"


// Default constructor; call Frustum::Frustum with NULL_VECTOR for near and far planes
// since they will be constructed from the view/proj matrices in ViewFrustum::ConstructFrustum
ViewFrustrum::ViewFrustrum(void) : Frustum(4U, NULL_VECTOR, NULL_VECTOR)
{
	// Set all fields to defaults
	m_farplaneworld.TL = XMVectorZero();
	m_farplaneworld.TR = XMVectorZero();
	m_farplaneworld.BL = XMVectorZero();
	m_farplaneworld.BR = XMVectorZero();
	m_projection = m_frustrumproj = ID_MATRIX;
	m_clip_near = 1.0f; m_clip_far = 1000.0f;
	m_aspect = (1024.0f / 768.0f);				// TODO: Fix to use actual resolution
	m_fov = (PI * 0.25f);
	m_fovtan = tanf(m_fov * 0.5f);
}

// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
Result XM_CALLCONV ViewFrustrum::Initialise(const FXMMATRIX projection, const float depth, const float FOV, const float aspect)
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

void XM_CALLCONV ViewFrustrum::ConstructFrustrum(const FXMMATRIX view, const CXMMATRIX invview)
{
	// Calculate the frustrum matrix based on the current view matrix and precalculated adjusted projection matrix
	XMFLOAT4X4 matrix;
	XMMATRIX fproj_view = XMMatrixMultiply(view, m_frustrumproj);
	XMStoreFloat4x4(&matrix, fproj_view);

	// Calculate near plane of frustum.
	SetPlane(Frustum::NEAR_PLANE,	XMPlaneNormalize(XMVectorSet(	matrix._14 + matrix._13,
																	matrix._24 + matrix._23, 
																	matrix._34 + matrix._33, 
																	matrix._44 + matrix._43)));

	// Calculate far plane of frustum.
	SetPlane(Frustum::FAR_PLANE,	XMPlaneNormalize(XMVectorSet(	matrix._14 - matrix._13,
																	matrix._24 - matrix._23,
																	matrix._34 - matrix._33,
																	matrix._44 - matrix._43)));

	// Calculate left plane of frustum.
	SetPlane(2U,					XMPlaneNormalize(XMVectorSet(	matrix._14 + matrix._11,
																	matrix._24 + matrix._21,
																	matrix._34 + matrix._31,
																	matrix._44 + matrix._41)));

	// Calculate right plane of frustum.
	SetPlane(3U,					XMPlaneNormalize(XMVectorSet(	matrix._14 - matrix._11,
																	matrix._24 - matrix._21,
																	matrix._34 - matrix._31,
																	matrix._44 - matrix._41)));

	// Calculate top plane of frustum.
	SetPlane(4U,					XMPlaneNormalize(XMVectorSet(	matrix._14 - matrix._12,
																	matrix._24 - matrix._22,
																	matrix._34 - matrix._32,
																	matrix._44 - matrix._42)));

	// Calculate bottom plane of frustum.
	SetPlane(5U,					XMPlaneNormalize(XMVectorSet(	matrix._14 + matrix._12,
																	matrix._24 + matrix._22,
																	matrix._34 + matrix._32,
																	matrix._44 + matrix._42)));

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

// Default destructor
ViewFrustrum::~ViewFrustrum(void)
{
	// Deallocate all memory held within this object

}
