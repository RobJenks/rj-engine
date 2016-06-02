#pragma once

#ifndef __SpaceSystemH__
#define __SpaceSystemH__

#include <string>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Texture.h"
#include "Octree.h"
#include "ObjectReference.h"
#include "iSpaceObject.h"
#include "BasicProjectileSet.h"

using namespace std;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class SpaceSystem : public ALIGN16<SpaceSystem>
{
public:
	// Default constructor
	SpaceSystem(void);

	// Default destructor
	~SpaceSystem(void);

	// Performs all initialisation for the system
	Result						InitialiseSystem(ID3D11Device *device);

	// Handles the entry of an object into the system, adding it to the system collections and updating the simulation state accordingly
	Result						AddObjectToSystem(iSpaceObject * object, const FXMVECTOR location);

	// Handles the exit of an object from the system, removing it from the system collections and updating the simulation state accordingly
	Result						RemoveObjectFromSystem(iSpaceObject * object);

	// Adds a basic object to the system; no logic besides adding to the relevant collections and SP tree
	Result						AddBaseObject(iObject *object, const FXMVECTOR location);

	// Removes a basic object from the system; no logic besides removing from the relevant collections and SP tree
	Result						RemoveBaseObject(iObject *object);

	// Shuts down the system and deallocates resources.  Expect this to only ever be executed upon application shutdown
	void						TerminateSystem(void);

	// Methods to get and set the unique string code, and descriptive string name, for this system
	CMPINLINE string			GetName(void) const			{ return m_name; }
	CMPINLINE string			GetCode(void) const			{ return m_code; }
	CMPINLINE void				SetName(string & name)		{ m_name = name; }
	CMPINLINE void				SetCode(string & code)		{ m_code = code; }

	// Returns the size of the system; should not be changed following system initialisation, and will have no effect if it is
	CMPINLINE XMVECTOR			GetSize(void) const			{ return m_size; }
	CMPINLINE XMFLOAT3			GetSizeF(void) const		{ return m_sizef; }
	void 						SetSize(const FXMVECTOR size);
	void						SetSize(const XMFLOAT3 & size);

	// Each system maintains a collection of objects that it currently holds
	std::vector<ObjectReference<iSpaceObject>>	Objects;

	// Each system maintains a collection of active projectiles that are present in the environment
	BasicProjectileSet			Projectiles;

	// Each system maintains an Octree for tracking the position of its objects
	Octree<iObject*> *			SpatialPartitioningTree;

	// Methods to retrieve objects in the system
	CMPINLINE std::vector<ObjectReference<iSpaceObject>> * 	GetObjects(void)					{ return &Objects; }

	// Methods to get or change the system backdrop texture
	CMPINLINE string						GetBackdropLocation(void) { return m_backdroplocation; }
	CMPINLINE void							SetBackdropLocation(string loc) { m_backdroplocation = loc; }
	CMPINLINE ID3D11ShaderResourceView *	GetBackdropTextureResource(void) { return m_backdrop->GetTexture(); }

private:
	string						m_name;
	string						m_code;
	bool						m_initialised;			// Flag indicating whether the system has been initialised

	AXMVECTOR					m_size;					// Size of the system; size of X --> will extend from -X/2 to +X/2 in each dimension
	AXMVECTOR					m_minbounds;			// Minimum bounds of the system; will be -X/2 assuming default cubic dimensions centred on the origin
	AXMVECTOR					m_maxbounds;			// Maximum bounds of the system; will be +X/2 assuming default cubic dimensions centred on the origin

	XMFLOAT3					m_sizef;				// Local float representation
	XMFLOAT3					m_minboundsf;			// Local float representation
	XMFLOAT3					m_maxboundsf;			// Local float representation

	// Space backdrop texture
	string						m_backdroplocation;
	Texture						*m_backdrop;
};


#endif