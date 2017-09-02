#pragma once

#ifndef __ViewFrustrumH__
#define __ViewFrustrumH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "Utility.h"
#include "Frustum.h"
#include "iObject.h"
class BoundingObject;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ViewFrustrum : public ALIGN16<ViewFrustrum>, public Frustum
{
public:

	// Constructor
	ViewFrustrum(void);

	// Should be run each time the projection/viewport settings change, to recalcuate cached information on the view frustrum
	Result XM_CALLCONV					Initialise(const FXMMATRIX projection, const float depth, const float FOV, const float aspect);

	// Builds a new frustrum for the current frame
	void XM_CALLCONV					ConstructFrustrum(const FXMMATRIX view, const CXMMATRIX invview);

	// Return basic data on the view frustum
	CMPINLINE float						GetFOV(void) const				{ return m_fov; }
	CMPINLINE float						GetTanOfHalfFOV(void) const		{ return m_fovtan; }
	CMPINLINE float						GetNearClipPlane(void) const	{ return m_clip_near; }
	CMPINLINE float						GetFarClipPlane(void) const		{ return m_clip_far; }

	const CMPINLINE D3DXFINITEPLANE *	GetFiniteFarPlane(void) const	{ return &m_farplaneworld; }

	// Destructor
	~ViewFrustrum(void);

private:

	// Projection matrix and other viewport data
	AXMMATRIX			m_projection;
	float				m_clip_near, m_clip_far, m_aspect;
	float				m_fov, m_fovtan;			// m_fovtan = tanf(FOV * 0.5f)
	AXMMATRIX			m_frustrumproj;				// Frustrum-specific proj matrix, preacalculate at initialisation

	D3DXFINITEPLANE		m_farplaneworld;			// The finite world plane that represents the visible far frustrum plane

};



#endif