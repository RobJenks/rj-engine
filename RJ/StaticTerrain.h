#pragma once

#ifndef __StaticTerrainH__
#define __StaticTerrainH__

#include "DX11_Core.h"
#include "GameDataExtern.h"
#include "Utility.h"
#include "OrientedBoundingBox.h"
class Model;
class StaticTerrainDefinition;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class StaticTerrain : public ALIGN16<StaticTerrain>
{
public:

	// Static record of the highest ID value in existence, for assigning to new tiles upon registration
	static Game::ID_TYPE							InstanceCreationCount;

	// Method to generate a new unique ID, called for each new tile being instantiated
	static Game::ID_TYPE							GenerateNewUniqueID(void)	{ return (++InstanceCreationCount); }

	// Default constructor
	StaticTerrain();

	// Static method to create a new terrain object.  If no definition is provided, a null-model (pure collision volume) will be created
	CMPINLINE static StaticTerrain *				Create(void) { return StaticTerrain::Create(NULL); }
	CMPINLINE static StaticTerrain *				Create(const std::string & def) { return StaticTerrain::Create(D::GetStaticTerrain(def)); }
	static StaticTerrain *							Create(const StaticTerrainDefinition *def);

	// Method to create a copy of a terrain object
	StaticTerrain *									Clone(void);

	// Get/set methods for key fields
	CMPINLINE Game::ID_TYPE							GetID(void) const								{ return m_id; }
	CMPINLINE void									SetID(Game::ID_TYPE id)							{ m_id = id; }

	CMPINLINE const StaticTerrainDefinition *		GetDefinition(void) const						{ return m_definition; }
	void											SetDefinition(const StaticTerrainDefinition *d);

	CMPINLINE const iSpaceObjectEnvironment *		GetParentEnvironment(void) const				{ return m_parent; }
	void											SetParentEnvironment(const iSpaceObjectEnvironment *env);
	
	CMPINLINE XMVECTOR								GetPosition(void) const							{ return m_data.Centre; }
	void											SetPosition(const FXMVECTOR pos);

	CMPINLINE XMVECTOR								GetOrientation(void) const						{ return m_orientation; }
	void											SetOrientation(const FXMVECTOR orient);

	CMPINLINE const OrientedBoundingBox::CoreOBBData &	GetOBBData(void) const						{ return m_data; }

	CMPINLINE XMMATRIX								GetWorldMatrix(void)							{ return m_worldmatrix; }
	CMPINLINE void XM_CALLCONV						SetWorldMatrix(const FXMMATRIX m)				{ m_worldmatrix = m; }

	CMPINLINE XMFLOAT3								GetExtentF(void) const							{ return m_data.ExtentF; }
	CMPINLINE void									GetExtent(AXMVECTOR_P(&outExtent)[3]) const		
	{ 
		outExtent[0] = m_data.Extent[0]; 
		outExtent[1] = m_data.Extent[1];
		outExtent[2] = m_data.Extent[2];
	}
	void											SetExtent(const FXMVECTOR e);

	CMPINLINE float									GetCollisionRadius(void) const					{ return m_collisionradius; }
	CMPINLINE float									GetCollisionRadiusSq(void) const				{ return m_collisionradiussq; }		// Read-only; derived from extent

	CMPINLINE INTVECTOR3							GetElementRangeMin(void) const					{ return m_element_min; }
	CMPINLINE INTVECTOR3							GetElementRangeMax(void) const					{ return m_element_max; }
	
	// Returns a value indicating whether this terrain object spans multiple elements
	CMPINLINE bool									SpansMultipleElements(void) const				{ return m_multielement; }
	
	// Return or set the link to a parent tile, if one exists.  An ID of zero indicates there is no link
	CMPINLINE Game::ID_TYPE							GetParentTileID(void) const						{ return m_parenttile; }
	CMPINLINE void									SetParentTileID(Game::ID_TYPE ID)				{ m_parenttile = ID; }

	// Retrieve or modify the health of this terrain object
	CMPINLINE float									GetHealth(void) const							{ return m_health; }
	void											SetHealth(float h)								{ m_health = h; }

	// Determines the vertices of the surrounding collision volume
	void											DetermineCollisionBoxVertices(iSpaceObjectEnvironment *parent, AXMVECTOR_P(&pOutVertices)[8]) const;

	// Method to postpone object updates; can be used to postpone updates until the end of multiple small adjustments
	void											PostponeUpdates(void)							{ m_postponeupdates = true; }

	// Method to resume updates following a postponement.  Will perform a recalculation upon resume
	void											ResumeUpdates(void)								
	{
		m_postponeupdates = false; 
		RecalculatePositionalData();
	}

	// Recalculates the positional data for this terrain following a change to its primary pos/orient data
	void											RecalculatePositionalData(void);

	// Creates a copy of the terrain object and returns a pointer.  Uses default copy constructor and modifies result
	StaticTerrain *									Copy(void) const;

	// Default destructor
	~StaticTerrain();

protected:

	Game::ID_TYPE							m_id;								// Unique ID of the terrain object
	const iSpaceObjectEnvironment *			m_parent;							// Parent environment that contains this terrain object
	const StaticTerrainDefinition *			m_definition;						// Pointer to the definition of this terrain type, which includes model details etc.  
																				// Can be null for collision regions that have no associated visible terrain

	OrientedBoundingBox::CoreOBBData		m_data;								// Core OBB data, used for terrain collision.  Also contains object position (as 'Centre')
	AXMVECTOR								m_orientation;						// Orientation relative to the environment
	AXMMATRIX								m_worldmatrix;						// Relative world matrix for the terrain object

	float									m_collisionradius;					// Collision radius is derived based upon the object size
	float									m_collisionradiussq;				// Precalculated squared collision radius for broadphase testing

	INTVECTOR3								m_element_min, m_element_max;		// Store the range of elements that this terrain object spans
	bool									m_multielement;						// Flag indicating whether we span >1 element, for render-time efficiency

	Game::ID_TYPE							m_parenttile;						// Store the ID of the tile that 'owns' this object, if relevant

	float									m_health;							// Current state of the terrain object

	bool									m_postponeupdates;					// Flag indicating whether the terrain object should postpone updates until it is released again
																				// Used to make multiple adjustments without recalculating derived data each time
};



#endif