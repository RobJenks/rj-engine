#pragma once

#ifndef __CSCorridorTileH__
#define __CSCorridorTileH__

#include "CompilerSettings.h"
#include "GameDataExtern.h"
#include "ComplexShipTile.h"
class TiXmlElement;

class CSCorridorTile : public ComplexShipTile
{
public:
	// Public inherited virtual method to return the type of ship tile this object represents
	CMPINLINE D::TileClass			GetClass(void) const	{ return D::TileClass::Corridor; }

	// Static method to return the class; allows this to be determined based on the class and not an instance
	CMPINLINE static D::TileClass	GetClassStatic(void) { return D::TileClass::Corridor; }

	// Corridor tiles require no simulation method
	CMPINLINE void					PerformTileSimulation(unsigned int delta_ms) { }

	// The pre- and post-parent-link events exposed by the base class
	void						BeforeLinkToParent(ComplexShip *ship);
	void						AfterLinkToParent(ComplexShip *ship);

	// The pre- and post-parent-unlink events exposed by the base class
	void						BeforeUnlinkFromParent(void);
	void						AfterUnlinkFromParent(ComplexShip *oldship);

	// Apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship.  Inherited virtual.
	void						ApplyTileSpecific(ComplexShipElement *el);

	// Virtual inherited method to make a copy of this tile and return it
	ComplexShipTile *			Copy(void) const;

	// Constructor/copy constructor/destructor
	CSCorridorTile(void);
	CSCorridorTile(const CSCorridorTile &C);
	~CSCorridorTile(void);

	// Virtual & static methods respectively to generate and read XML data representing the tile
	TiXmlElement *				GenerateXML(void);						// Virtual inherited since is called on an instance of a tile
	
	// Virtual method to read any class-specific data for this tile type
	CMPINLINE void				ReadClassSpecificXMLData(TiXmlElement *node) { /* No class-specific data for this tile class */ }

private:

};


#endif