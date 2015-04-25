/* *** NO LONGER IN USE ***

#pragma once

#ifndef __ComplexShipDetailsH__
#define __ComplexShipDetailsH__

#include "ShipDetails.h"
#include <string>
#include <vector>
#include "CompilerSettings.h"
#include "iContainsComplexShipElements.h"
#include "iContainsComplexShipTiles.h"
#include "ComplexShipElement.h"
#include "ComplexShipSection.h"
class HardpointsInterface;
class NavNetwork;
using namespace std;


class ComplexShipDetails : public ShipDetails, public iContainsComplexShipElements, public iContainsComplexShipTiles
{
public:
	typedef vector<ComplexShipSection*> ComplexShipSectionCollection;

	ComplexShipDetails(void);
	~ComplexShipDetails(void);

	CMPINLINE string				GetCode(void) { return m_code; }
	CMPINLINE void					SetCode(string code) { m_code = code; }

	// Hardpoint-related methods
	CMPINLINE iHardpoints *			GetHardpoints(void) { return (iHardpoints*)m_hardpoints; }
	CMPINLINE HardpointsInterface *	GetShipHardpoints(void) { return m_hardpoints; }
	void							ReplaceShipHardpoints(HardpointsInterface *hp);

	// Loads the contents of a ship section element space into the ship itself.  Any subsequent ship element definitions are a delta on this standard data
	Result							ApplyComplexShipSectionToShip(ComplexShipSection *section);


	// Returns a reference to the SHIP SECTION element at this location in the ship blueprint.  Contrast with GetElement(...)
	ComplexShipElement *			GetShipSectionElement(INTVECTOR3 location);
	ComplexShipElement *			GetShipSectionElement(int x, int y, int z);

	// Methods to add/remove/retrieve the sections that make up this complex ship
	ComplexShipSection *			GetSection(int index);
	CMPINLINE ComplexShipSectionCollection *	
									GetSections(void) { return &(m_sections); }
	CMPINLINE int					GetSectionCount(void) { return m_sections.size(); }

	Result							AddShipSection(ComplexShipSection *section);
	void							RemoveShipSection(ComplexShipSection *section);
	void							RemoveShipSection(int index);

	// Methods triggered based on major events impacting this object
	void							ShipTileAdded(ComplexShipTile *tile);		// When a tile is added.  Virtual inherited from interface.
	void							ShipTileRemoved(ComplexShipTile *tile);		// When a tile is removed.  Virtual inherited from interface.
	void							ElementLayoutChanged(void);					// When the layout (e.g. active/walkable state, connectivity) of elements is changed

	// Get a reference to the navigation network assigned to this ship
	CMPINLINE NavNetwork *			GetNavNetwork(void) { return m_navnetwork; }

	// Updates the ship navigation network based on the set of elements and their properties
	void							UpdateNavigationNetwork(void);

	// Methods to get and set the ship designer offset, determining where the ship will appear within the SD
	INTVECTOR3						GetSDOffset(void) { return m_sdoffset; }
	void							SetSDOffset(INTVECTOR3 offset) { m_sdoffset = offset; }

	// Methods to set and check the flag that determines whether a ship has been directly generated from the SD
	bool							HasBeenDirectlyGeneratedFromSD(void)				{ return m_directlygeneratedfromSD; }
	void							FlagShipAsDirectlyGeneratedFromShipDesigner(bool b)	{ m_directlygeneratedfromSD = b; }

	// Shutdown methods
	void							Shutdown(void);
	void							Shutdown(bool ShutdownSections, bool UnlinkTiles, bool IncludeStandardSections);

	// Methods to return the standard path / filename where this ship data should be stored, if it is following the standard convention
	string							DetermineXMLDataPath(void);
	string							DetermineXMLDataFilename(void);
	string							DetermineXMLDataFullFilename(void);

	// Make a copy of the ship details and return it
	static ComplexShipDetails *		Copy(ComplexShipDetails *ship);

	// Static method that attempts to load section details from the global collection
	static ComplexShipDetails *		Get(string code);


private:

	// Key properties of the complex ship details
	string							m_code;

	// The sections that make up this ship
	ComplexShipSectionCollection	m_sections;

	// Interface to all the hardpoints on this ship (and owned by its component ship sections)
	HardpointsInterface *			m_hardpoints;

	// The navigation network that actors will use to move around this ship
	NavNetwork *					m_navnetwork;

	// Ship designer offset; determines where the ship will appear when loaded into the SD
	INTVECTOR3						m_sdoffset;

	// Flag that indicates the ship was directly generated from the SD
	bool							m_directlygeneratedfromSD;
};


#endif

*/