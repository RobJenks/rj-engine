#pragma once

#ifndef __CSCorridorTileDefinitionH__
#define __CSCorridorTileDefinitionH__

#include "ComplexShipTileDefinition.h"

class CSCorridorTileDefinition : public ComplexShipTileDefinition
{

public:

	// Default constructor
	CSCorridorTileDefinition(void)
	{
		// Set the tile class type
		m_classtype = D::TileClass::Corridor;
	}

	// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
	CMPINLINE void			ApplyClassSpecificDefinition(ComplexShipTile *tile) const { /* No subclass-specific properties to be set */ }

	// Virtual method to read class-specific XML data for the tile definition
	CMPINLINE void			ReadClassSpecificXMLData(TiXmlElement *node) { /* No subclass-specific data to be read */ }

};


#endif
