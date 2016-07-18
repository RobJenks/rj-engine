#pragma once

#ifndef __CSQuartersTileDefinitionH__
#define __CSQuartersTileDefinitionH__

#include "ComplexShipTileDefinition.h"

class CSQuartersTileDefinition : public ComplexShipTileDefinition
{

public:

	// Default constructor
	CSQuartersTileDefinition(void)
	{
		// Set the tile class type
		m_classtype = D::TileClass::Quarters;
	}

	// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
	CMPINLINE void			ApplyClassSpecificDefinition(ComplexShipTile *tile) const { /* No subclass-specific properties to be set */ }

	// Virtual method to read class-specific XML data for the tile definition
	CMPINLINE void			ReadClassSpecificXMLData(TiXmlElement *node) { /* No subclass-specific data to be read */ }
};


#endif
