#pragma once

#include "ALIGN16.h"
#include "DX11_Core.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ViewPortal
{
public:

	// Constructor must pass in at least the portal vertex count to allow allocation at construct-time
	ViewPortal(size_t vertex_count);
	
	// Return basic data 
	CMPINLINE size_t							GetVertexCount(void) const { return m_vertex_count; }
	CMPINLINE const AXMVECTOR *					GetVertices(void) const { return m_vertices; }
	CMPINLINE const XMFLOAT3 *					GetVerticesF(void) const { return m_verticesf; }
	CMPINLINE const AXMVECTOR					GetCentrePoint(void) const { return m_centre; }
	CMPINLINE float								GetBoundingSphereRadius(void) const { return m_bounding_sphere_radius; }

	// Set the vertex data for this portal; will read vertex_count sequential entries from the given pointer
	void										SetVertices(const XMVECTOR *vertices);
	void										SetVertices(const XMFLOAT3 *vertices);

	// Recalculates internal data within the portal following a change to the vertex layout
	void										RecalculateData(void);

	// Destructor; deallocates all storage owned by the object
	~ViewPortal(void);


private:

	// Portals are defined as N-vertex 2D shapes (with N >= 3) in world space.  Vertices are stored
	// in both SEE-aligned and basic float forms to allow more efficient per-frame calculation
	AXMVECTOR					*m_vertices;
	XMFLOAT3					*m_verticesf;
	size_t						m_vertex_count;

	// We also store the centre point of the portal and a very upper-bound conservative
	// bounding sphere radius (i.e. assuming portal is facing the camera head-on) for
	// cheap initial intersection tests with the current frustum
	AXMVECTOR					m_centre;
	float						m_bounding_sphere_radius;

};

