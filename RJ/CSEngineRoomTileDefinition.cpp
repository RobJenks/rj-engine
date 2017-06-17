#include "CSEngineRoomTileDefinition.h"

// Default constructor
CSEngineRoomTileDefinition::CSEngineRoomTileDefinition(void)
{
	// Set the tile class type
	m_classtype = D::TileClass::EngineRoom;

	// We do have class-specific data for this tile type
	RegisterClassSpecificDataForTileDefinition();

	// Initialise default values


}

// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
void CSEngineRoomTileDefinition::ApplyClassSpecificDefinition(ComplexShipTile *tile) const
{

}

// Virtual method to read class-specific XML data for the tile
void CSEngineRoomTileDefinition::ReadClassSpecificXMLData(TiXmlElement *node)
{

}


// Default destructor
CSEngineRoomTileDefinition::~CSEngineRoomTileDefinition(void)
{

}



