/* *** NO LONGER IN USE *** 
#pragma once

#ifndef __ComplexShipSectionDetailsH__
#define __ComplexShipSectionDetailsH__

#include <string>
#include <unordered_map>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "iContainsComplexShipElements.h"
#include "iContainsComplexShipTiles.h"
#include "ComplexShipElement.h"
#include "ComplexShipTile.h"
#include "Utility.h"
class Model;
class Texture;
class ComplexShipDetails;
class Hardpoints;
using namespace std;
using namespace std::tr1;


class ComplexShipSectionDetails :	public iContainsComplexShipElements, 
									public iContainsComplexShipTiles
{
public:
	ComplexShipSectionDetails(void);
	~ComplexShipSectionDetails(void);

	// Recalculates the ship section details based on components that are loaded.  Called after loading from file
	void							RecalculateShipSectionDetails(void);

	CMPINLINE string				GetCode(void) { return m_code; }
	CMPINLINE void					SetCode(string code) { m_code = code; }

	CMPINLINE string				GetName(void) { return m_name; }
	CMPINLINE void					SetName(string name) { m_name = name; }

	CMPINLINE bool					IsStandardSection(void) { return m_standardsection; }
	CMPINLINE void					SetIsStandardSection(bool standard) { m_standardsection = standard; }
	
	CMPINLINE Model *				GetModel(void) { return m_model; }
	CMPINLINE void					SetModel(Model *m) { m_model = m; }

	CMPINLINE ComplexShipDetails *	GetParent(void) { return m_parent; }
	CMPINLINE void					SetParent(ComplexShipDetails *ship) { m_parent = ship; }

	CMPINLINE float					GetMass(void) { return m_mass; }
	CMPINLINE void					SetMass(float mass) { m_mass = mass; }

	CMPINLINE float					GetVelocityLimit(void) { return m_velocitylimit; }
	CMPINLINE void					SetVelocityLimit(float velocitylimit) { m_velocitylimit = velocitylimit; }

	CMPINLINE float					GetAngularVelocityLimit(void) { return m_angularvelocitylimit; }
	CMPINLINE void					SetAngularVelocityLimit(float velocitylimit) { m_angularvelocitylimit = velocitylimit; }

	CMPINLINE float					GetBrakeFactor(void) { return m_brakefactor; }
	CMPINLINE void					SetBrakeFactor(float factor) { m_brakefactor = factor; }
	
	CMPINLINE float					GetTurnRate(void) { return m_turnrate; }
	CMPINLINE void					SetTurnRate(float turnrate) { m_turnrate = turnrate; }
	
	CMPINLINE float					GetTurnAngle(void) { return m_turnangle; }
	CMPINLINE void					SetTurnAngle(float turnangle) { m_turnangle = turnangle; }

	CMPINLINE float					GetBankRate(void) { return m_bankrate; }
	CMPINLINE void					SetBankRate(float bankrate) { m_bankrate = bankrate; }

	CMPINLINE D3DXVECTOR3			GetBankExtents(void) { return m_bankextent; }
	CMPINLINE void					SetBankExtents(D3DXVECTOR3 bankextent) { m_bankextent = bankextent; }

	CMPINLINE ComplexShipElement*** GetElements(void) { return m_elements; }
	CMPINLINE ComplexShipElement****GetElementsPointer(void) { return &m_elements; }
	CMPINLINE void					SetElements(ComplexShipElement ***elements) { m_elements = elements; }
	
	CMPINLINE ComplexShipElement *	GetElement(INTVECTOR3 loc) { return GetElement(loc.x, loc.y, loc.z); }
	CMPINLINE ComplexShipElement *	GetElement(int x, int y, int z);
	CMPINLINE void					SetElement(int x, int y, int z, const ComplexShipElement *e);

	CMPINLINE ComplexShipElement *	GetElementDirect(int x, int y, int z) { return &(m_elements[x][y][z]); }
	CMPINLINE void					SetElementDirect(int x, int y, int z, const ComplexShipElement *e) { m_elements[x][y][z] = *e; }

	//CMPINLINE int					GetNumBoundingObjects(void) { return m_numboundingobjects; }
	//void							AllocateSpaceForBoundingObjects(int capacity);
	//void							DeallocateSpaceForBoundingObjects(void);
	//void							ClearPointerToBoundingObjectData(void) { m_bounds = NULL; m_numboundingobjects = 0; }

	//CMPINLINE BoundingObject *		GetBoundingObject(int index) { if (index < 0 || index >= m_numboundingobjects) return NULL; else return &(m_bounds[index]); }
	//CMPINLINE BoundingObject **		GetBoundingObjects(void) { return &m_bounds; }
	//CMPINLINE void					SetBoundingObject(int index, BoundingObject *obj) { if (index >= 0 && index < m_numboundingobjects) m_bounds[index] = (*obj); }
	

	CMPINLINE Hardpoints *			GetHardpoints(void) { return m_hardpoints; }
	CMPINLINE void					SetHardpoints(Hardpoints *hp) { m_hardpoints = hp; }

	Result							RotateShipSection(Rotation90Degree rot);

	CMPINLINE INTVECTOR3 			GetElementSize(void) { return m_elementsize; }
	CMPINLINE int					GetElementSizeX(void) { return m_elementsize.x; }
	CMPINLINE int					GetElementSizeY(void) { return m_elementsize.y; }
	CMPINLINE int					GetElementSizeZ(void) { return m_elementsize.z; }
	CMPINLINE INTVECTOR3 *			GetElementSizePointer(void) { return &m_elementsize; }
	CMPINLINE void					SetElementSize(INTVECTOR3 size) { m_elementsize = size; }
	CMPINLINE void					SetElementSize(int x, int y, int z) { m_elementsize = INTVECTOR3(x, y, z); }
	CMPINLINE void					SetElementSizeX(int x) { m_elementsize.x = x; }
	CMPINLINE void					SetElementSizeY(int y) { m_elementsize.y = y; }
	CMPINLINE void					SetElementSizeZ(int z) { m_elementsize.z = z; }

	// Methods triggered when a tile is added or removed from this object
	void							ShipTileAdded(ComplexShipTile *tile);
	void							ShipTileRemoved(ComplexShipTile *tile);

	// Methods for manipulating the section preview images that are rendered by the ship designer
	Result							LoadPreviewImage(string path);
	CMPINLINE string				GetPreviewImagePath(void) { return m_previewimagepath; }
	CMPINLINE string				GetPreviewImageRelPath(void) { return m_previewimagerelpath; }
	CMPINLINE Texture *				GetPreviewImage(void) const { return m_previewimage; }
	CMPINLINE INTVECTOR2			GetPreviewImageSize(void) { return m_previewimagesize; }
	CMPINLINE void					SetPreviewImageSize(INTVECTOR2 size) { m_previewimagesize = size; }
	void							SetPreviewImageParametersDirect(Texture *texture, INTVECTOR2 previewsize, string previewrelpath, string previewpath);

	// Query or set the visibility-testing mode for this object type (applied to underlying objects)
	CMPINLINE VisibilityTestingModeType		GetVisibilityTestingMode(void) const						{ return m_visibilitytestingmode; }
	CMPINLINE void							SetVisibilityTestingMode(VisibilityTestingModeType mode)	{ m_visibilitytestingmode = mode; }

	// Shutdown method to deallocate all ship section data
	void							Shutdown(void);

	// Methods to return the standard path / filename where this ship section data should be stored, if it is following the standard convention
	string							DetermineXMLDataPath(void);
	string							DetermineXMLDataFilename(void);
	string							DetermineXMLDataFullFilename(void);
	

	// Static method that attempts to load section details from the global collection
	static ComplexShipSectionDetails *			Get(string code);

	// Static method to copy a ship section details instance 
	static ComplexShipSectionDetails *			Copy(ComplexShipSectionDetails *details);
		
	// Static methods to manage the collection of preview images for each ship section
	CMPINLINE static bool						HaveSectionPreviewImage(string filename) { return (__previewimages.count(filename) > 0); }
	CMPINLINE static Texture *					GetSectionPreviewImage(string filename) { if (__previewimages.count(filename) > 0) return __previewimages[filename]; else return NULL; }
	CMPINLINE static void						AddSectionPreviewImage(string filename, Texture *texture) { __previewimages[filename] = texture; }
	static void									TerminateSectionPreviewData(void);

	// Allocates and initialises storage for elements based on the element size properties.  Also deallocates any existing memory.
	Result							InitialiseAllElements(void);

																				
private:
	string							m_code;						// Uniquely-identifying code for this ship section
	string							m_name;						// String display name for this ship section
	bool							m_standardsection;			// Flag determining whether this is a standard (or custom) section
	Model *							m_model;					// Model to be rendered for this ship section
																// TOOD: Change once deformation is implemented?
	ComplexShipDetails *			m_parent;					// Pointer to the parent ship holding this ship section
	
	float							m_mass;						// Mass of the ship section
	float							m_velocitylimit;			// Max velocity limit of the section
	float							m_angularvelocitylimit;		// Max angular velocity limit of the section
	float							m_turnrate;					// Base turn rate of the section (rad/s)
	float							m_turnangle;				// Max turning angle for the section (rad)
	float							m_bankrate;					// Base bank rate of the section
	D3DXVECTOR3						m_bankextent;				// Banking extents for the section in each dimension
	float							m_brakefactor;				// Percentage of total velocity limit we can brake per second

	Hardpoints *					m_hardpoints;				// The collection of external hardpoints on this ship section

	// The individual elements that make up this ship section
	ComplexShipElement ***			m_elements;					// E[x][y][z]

	// Size of this ship section, in elements
	INTVECTOR3						m_elementsize;				

	// The preview image for this ship section
	Texture *						m_previewimage;
	string							m_previewimagepath;
	string							m_previewimagerelpath;
	INTVECTOR2						m_previewimagesize;

	// Method for performing visibility detection on this class of ship section
	VisibilityTestingModeType		m_visibilitytestingmode;
	
	// Fields for managing the ship tiles linked to this section
	//iContainsComplexShipTiles::ComplexShipTileCollection
									m_tiles;					// The collection of tiles linked to this section
	//int								m_tilecount;				// The number of tiles that are linked to this section (precalc)
	//bool							m_hastiles;					// Flag to indicate whether this section contains any tiles (precalc)
	//bool							m_multitile;				// Flag to indicate whether this section contains >1 tiles (precalc)
	

	// Static collection of section preview images
	typedef unordered_map<string, Texture*> __PreviewImageCollection;
	static __PreviewImageCollection __previewimages;

};


CMPINLINE ComplexShipElement *ComplexShipSectionDetails::GetElement(int x, int y, int z)
{
	// Make sure the coordinates provided are valid
	if (x < 0 || y < 0 || z < 0 || x >= m_elementsize.x || y >= m_elementsize.y || z >= m_elementsize.z) return NULL;

	// Return the element at this location
	return &(m_elements[x][y][z]);
}

CMPINLINE void ComplexShipSectionDetails::SetElement(int x, int y, int z, const ComplexShipElement *e)
{
	// Make sure the coordinates provided are valid
	if (x < 0 || y < 0 || z < 0 || x >= m_elementsize.x || y >= m_elementsize.y || z >= m_elementsize.z) return;

	// Set the element to a copy of this object
	m_elements[x][y][z] = (*e);
}



#endif


*/