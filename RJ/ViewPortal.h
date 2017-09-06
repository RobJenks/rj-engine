#pragma once

#include "ALIGN16.h"
#include "DX11_Core.h"
#include "AABB.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ViewPortal
{
public:

	// Portals are represented as an AABB which encloses the potentially-arbitrary number of vertices making
	// up the actual opening.  We do this for efficiency since a small amount of overdraw is acceptable
	// Portals are one-way only, i.e. P0 is always top-top-left and vice versa for P1
	//const AABB									LocalBounds;		// Tile-local bounds
	AABB									Bounds;				// World-space bounds
	
	// Constructor for a new view portal.  Vertices are specified in world coordinates
	ViewPortal(const FXMVECTOR world_min_point, const FXMVECTOR world_max_point) noexcept;

	// Copy constructor
	ViewPortal(const ViewPortal & other) noexcept;

	// Copy assignment
	ViewPortal & operator=(const ViewPortal & other) noexcept;
	
	// Move constructor
	ViewPortal(ViewPortal && other) noexcept;

	// Move assignment
	ViewPortal & operator=(ViewPortal && other) noexcept;

	// Constructor for a new view portal.  Vertices are specified in local coordinates, with a 
	//ViewPortal(const FXMVECTOR local_min_point, const FXMVECTOR local_max_point, const FXMMATRIX world_matrix);
	
	// Centre point and bounding radius in world space
	CMPINLINE const AXMVECTOR					GetCentrePoint(void) const { return m_centre; }
	CMPINLINE float								GetBoundingSphereRadius(void) const { return m_bounding_sphere_radius; }

	// Recalculates internal data within the portal following a change to the vertex layout
	void										RecalculateData(void);

	// Destructor; deallocates all storage owned by the object
	~ViewPortal(void);


private:

	// We also store the centre point of the portal and a very upper-bound conservative
	// bounding sphere radius (i.e. to the radius of oru maximum extents) for
	// cheap initial intersection tests with the current frustum
	AXMVECTOR					m_centre;
	float						m_bounding_sphere_radius;

};

