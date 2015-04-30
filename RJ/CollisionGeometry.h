#pragma once

#ifndef __CollisionGeometryH__
#define __CollisionGeometryH__

#include "DX11_Core.h"



class CollisionGeometry
{
public:
	// Constant specifying the maximum number of volumes that are permitted within one set of bounding geometry
	static const int		MAX_COLLISION_VOLUMES		= 1000;

	// Default constructor
	CollisionGeometry(void);

	// Initialises the collision geometry with space for a certain number of collision volumes
	void					Initialise(int volumecount);

	// Adds a new collision volume to the geometry
	void					AddCollisionVolume(int index, const D3DXVECTOR3 & localpos, const D3DXVECTOR3 & size);

	// Tests for collision against this geometry in world space
	//bool					TestCollision(const D3DXMATRIX *world, [OTHER BOUNDING VOLUME/GEOMETRY]

	// Default destructor
	~CollisionGeometry(void);

private:
	int						m_numvolumes;		// The number of collision volumes 
	D3DXVECTOR3 *			m_position;			// Local position, e.g. offset relative to parent object position, of each volume
	D3DXVECTOR3 *			m_size;				// Size of each collision volume, in world units

	bool					m_hasoffset;		// Flag determining whether we actually have any nonzero positions, i.e. false == at (0,0,0) relative 
												// to parent.  This allows us to skip one matrix transform (of the offset into world space) per volume
	
};



#endif