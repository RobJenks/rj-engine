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
	AABB										Bounds;				// Parent-local bounds (generally tile-local)
	
	// Constructor for a new view portal.  Vertices are specified in parent-local (usually tile-local) 
	// coordinates.  Location and target location are element indices (which can then easily be translated
	// to the relevant parent cell/tile)
	ViewPortal(int location, const FXMVECTOR min_point, const FXMVECTOR max_point, int target_location) noexcept;

	// Copy constructor
	ViewPortal(const ViewPortal & other) noexcept;

	// Copy assignment
	ViewPortal & operator=(const ViewPortal & other) noexcept;
	
	// Move constructor
	ViewPortal(ViewPortal && other) noexcept;

	// Move assignment
	ViewPortal & operator=(ViewPortal && other) noexcept;
	
	// Return the element index of the portal parent
	CMPINLINE int								GetLocation(void) const { return m_location; }

	// Returns the element index of the target element
	CMPINLINE int								GetTargetLocation(void) const { return m_target; }

	// Centre point and bounding radius in parent-local space (generally tile-local)
	CMPINLINE const AXMVECTOR					GetCentrePoint(void) const { return m_centre; }
	CMPINLINE float								GetBoundingSphereRadius(void) const { return m_bounding_sphere_radius; }

	// Recalculates internal data within the portal following a change to the vertex layout
	void										RecalculateData(void);

	// Destructor; deallocates all storage owned by the object
	~ViewPortal(void);


private:

	// Location of the view portal and its destination, both specified as an element index (which
	// can then easily be translated to a tile reference)
	int							m_location;
	int							m_target;

	// We also store the centre point of the portal and a very upper-bound conservative
	// bounding sphere radius (i.e. to the radius of oru maximum extents) for
	// cheap initial intersection tests with the current frustum
	AXMVECTOR					m_centre;
	float						m_bounding_sphere_radius;

};

