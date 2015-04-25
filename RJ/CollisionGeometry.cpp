#include <malloc.h>
#include "DX11_Core.h"

#include "FastMath.h"

#include "CollisionGeometry.h"

// Default constructor
CollisionGeometry::CollisionGeometry(void)
{
	// Initialise to default values
	m_numvolumes = 0;
	m_position = NULL;
	m_size = NULL;
	m_hasoffset = false;
}

// Initialises the collision geometry with space for a certain number of collision volumes
void CollisionGeometry::Initialise(int volumecount)
{
	// If there is already memory allocated then delete it here
	if (m_position) SafeFree(m_position); 
	if (m_size) SafeFree(m_size);
	m_numvolumes = 0;
	m_hasoffset = false;

	// Make sure this is a valid number of volumes being requested; if not, quit now
	if (volumecount <= 0 || volumecount > CollisionGeometry::MAX_COLLISION_VOLUMES) return;

	// Store the new volume count and allocate memory for each volume
	m_numvolumes = volumecount;
	m_position = (D3DXVECTOR3*)malloc(sizeof(D3DXVECTOR3) * volumecount);
	m_size =	 (D3DXVECTOR3*)malloc(sizeof(D3DXVECTOR3) * volumecount);
}

// Adds a new collision volume to the geometry
void CollisionGeometry::AddCollisionVolume(int index, const D3DXVECTOR3 & localpos, const D3DXVECTOR3 & size)
{
	// Make sure the index is valid 
	if (index < 0 || index >= m_numvolumes) return;

	// Store the values
	m_position[index] = localpos;
	m_size[index] = size;

	// Determine whether this adds a non-zero offset, in which case we need to set the flag so that collision tests account for it
	if (!IsZeroVector(m_position[index])) m_hasoffset = true;
}


// Default destructor
CollisionGeometry::~CollisionGeometry(void)
{
	// If memory has been allocated then free it here
	if (m_position) SafeFree(m_position); 
	if (m_size) SafeFree(m_size);
}
