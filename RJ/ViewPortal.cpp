#include "FastMath.h"
#include "DX11_Core.h"
#include "ViewPortal.h"

// Constructor must pass in at least the portal vertex count to allow allocation at construct-time
ViewPortal::ViewPortal(size_t vertex_count)
	:
	m_vertex_count(0U), m_vertices(NULL), m_verticesf(NULL), m_centre(NULL_VECTOR), m_bounding_sphere_radius(0.0f)
{
	// Must have >= 3 vertices.  Also place reasonable upper bound on vertex count
	assert(vertex_count >= 3U);
	assert(vertex_count < 36U);

	// Allocate space
	m_vertex_count = vertex_count;
	m_vertices = new AXMVECTOR[vertex_count];
	m_verticesf = new XMFLOAT3[vertex_count];
	
	// Initialise storage
	m_vertices = { 0 };
	m_verticesf = { 0 };	
}

// Set the vertex data for this portal; will read vertex_count sequential entries from the given pointer
void ViewPortal::SetVertices(const XMVECTOR *vertices)
{
	if (!vertices) return;

	for (int i = 0; i < m_vertex_count; ++i)
	{
		// Set both SSE- and float-based data
		m_vertices[i] = vertices[i];
		XMStoreFloat3(&(m_verticesf[i]), vertices[i]);
	}

	// Recalculate any dependent data
	RecalculateData();
}

// Set the vertex data for this portal; will read vertex_count sequential entries from the given pointer
void ViewPortal::SetVertices(const XMFLOAT3 *vertices)
{
	if (!vertices) return;

	for (int i = 0; i < m_vertex_count; ++i)
	{
		// Set both SSE- and float-based data
		m_verticesf[i] = vertices[i];
		m_vertices[i] = XMLoadFloat3(&vertices[i]);
	}

	// Recalculate any dependent data
	RecalculateData();
}

// Recalculates internal data within the portal following a change to the vertex layout
void ViewPortal::RecalculateData(void)
{
	// Determine an approximate world-space midpoint and bounding radius for the portal
	XMVECTOR midpoint = m_vertices[0];
	XMVECTOR min_coord = m_vertices[0];
	XMVECTOR max_coord = m_vertices[0];

	for (int i = 0; i < m_vertex_count; ++i)
	{
		midpoint = XMVectorAdd(midpoint, m_vertices[1]);
		min_coord = XMVectorMin(min_coord, m_vertices[1]);
		max_coord = XMVectorMax(max_coord, m_vertices[1]);
	}

	// Midpoint = sum(x1, x2, ..., xk) / k
	midpoint = XMVectorScale(midpoint, 1.0f / m_vertex_count);

	// Approx bounding radius = (distance from min_coords to max_coords) / 2
	m_bounding_sphere_radius = XMVectorGetX(XMVectorScale(XMVector3LengthEst(
		XMVectorSubtract(max_coord, min_coord)), 0.5f));
}

// Destructor; deallocates all storage owned by the object
ViewPortal::~ViewPortal(void)
{
	// Deallocate all owned storage
	if (m_vertices)		SafeDeleteArray(m_vertices);
	if (m_verticesf)	SafeDeleteArray(m_verticesf);
	m_vertex_count = 0U;
}
