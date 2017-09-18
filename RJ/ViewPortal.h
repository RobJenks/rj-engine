#pragma once

#include "ALIGN16.h"
#include "DX11_Core.h"
#include "AABB.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ViewPortal
{
public:

	// Portals are represented by exactly four vertices that define a finite 2D plane completely enclosing 
	// the actual opening.  We do this for efficiency since a small amount of overdraw is acceptable
	// Portals are one-way only with facing determined by clockwise winding order
	AXMVECTOR Vertices[4];				// Parent-local portal vertices (generally tile-local)
	
	// Constructor for a new view portal, providing the vertices of the portal in parent-local (generally tile-
	// local) space.  Portal properties are derived based on vertices; portal uses clockwise winding order
	// to determine facing and target element
	ViewPortal(const AXMVECTOR(&vertices)[4]) noexcept;
	ViewPortal(const FXMVECTOR p0, const FXMVECTOR p1, const FXMVECTOR p2, const FXMVECTOR p3);

	// Default constructor; not used
	CMPINLINE ViewPortal(void) noexcept { };

	// Copy constructor
	ViewPortal(const ViewPortal & other) noexcept;

	// Copy assignment
	ViewPortal & operator=(const ViewPortal & other) noexcept;
	
	// Move constructor
	ViewPortal(ViewPortal && other) noexcept;

	// Move assignment
	ViewPortal & operator=(ViewPortal && other) noexcept;
	
	// Element index of the portal parent
	CMPINLINE int								GetLocation(void) const						{ return m_location; }
	CMPINLINE void								SetLocation(int location)					{ m_location = location; }

	// Target element direction
	CMPINLINE Direction							GetTargetDirection(void) const				{ return m_target_direction; }
	CMPINLINE void								SetTargetDirection(Direction direction)		{ m_target_direction = direction; }

	// Target element location
	CMPINLINE int								GetTargetLocation(void) const				{ return m_target; }
	CMPINLINE void								SetTargetLocation(int target_location)		{ m_target = target_location; }

	// Centre point and bounding radius in parent-local space (generally tile-local)
	CMPINLINE const AXMVECTOR					GetCentrePoint(void) const					{ return m_centre; }
	CMPINLINE float								GetBoundingSphereRadius(void) const			{ return m_bounding_sphere_radius; }

	// Normal vector in local space
	CMPINLINE XMVECTOR							GetNormal(void) const						{ return m_normal; }

	// Transform the portal by the given transformation matrix
	void										Transform(const FXMMATRIX transform);

	// Debug string representation of the portal
	std::string									DebugString(void) const;

	// Destructor; deallocates all storage owned by the object
	~ViewPortal(void);

	// Static instance representing a null portal that will have no effect; returned when supplied data
	// is incorrect or insufficient to construct a valid portal
	static const ViewPortal						NullPortal;

private:

	// Location of the view portal and its destination, both specified as an element index (which
	// can then easily be translated to a tile reference).  Also store the direction of the portal
	// target so that the portal target can be recalculated whenever added to a parent
	int							m_location;
	int							m_target;
	Direction					m_target_direction;

	// We also store the centre point of the portal and a very upper-bound conservative
	// bounding sphere radius (i.e. to the radius of oru maximum extents) for
	// cheap initial intersection tests with the current frustum
	float						m_bounding_sphere_radius; 
	AXMVECTOR					m_centre;

	// Store the portal normal vector to allow fast testing of forward/back-facing
	AXMVECTOR					m_normal;

	// Determines the adjacent element direction that this portal connects to, based upon the portal normal vector
	Direction					DeterminePortalTargetDirection(const FXMVECTOR normal_vector);

	// Recalculates internal data within the portal following a change to the vertex layout
	void						RecalculateData(void);

};

