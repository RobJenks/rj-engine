#pragma once

#include "ComplexShipTile.h"
class CSArmourTileDefinition;


class CSArmourTile : public ComplexShipTile
{
public:


	// Default constructor
	CSArmourTile(void);

	// Public inherited virtual method to return the type of ship tile this object represents
	CMPINLINE D::TileClass								GetClass(void) const { return D::TileClass::Armour; }

	// Static method to return the class; allows this to be determined based on the class and not an instance
	CMPINLINE static D::TileClass						GetClassStatic(void) { return D::TileClass::Armour; }

	// Simulation method for this tile; not required for armour tiles
	CMPINLINE void										PerformTileSimulation(unsigned int delta_ms) { /* Not required */ };

	// We store a direct reference to the power generator definition for more efficient runtime access
	const CSArmourTileDefinition *						GetArmourTileDefinition(void) const { return m_armourtiledef; }
	void												StoreArmourTileDefinition(const CSArmourTileDefinition *def) { m_armourtiledef = def; }

	// Recalculates all values following a change to our parameters; not currently required for armour riles
	CMPINLINE void										RecalculateParameters(void) { /* Not required */ }

	// Apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship.  Inherited virtual.
	CMPINLINE void										ApplyTileSpecific(void) { }

	// Virtual inherited method to make a copy of this tile and return it
	ComplexShipTile *									Copy(void) const;

	// Virtual & static methods respectively to generate and read XML data representing the tile
	TiXmlElement *										GenerateXML(void);						// Virtual inherited since is called on an instance of a tile

																								// Virtual method to read any class-specific data for this tile type
	void												ReadClassSpecificXMLData(TiXmlElement *node);

	// Processes a debug tile command from the console
	void												ProcessDebugTileCommand(GameConsoleCommand & command);

	// Default destructor
	~CSArmourTile(void);

private:

	const CSArmourTileDefinition *						m_armourtiledef;


};