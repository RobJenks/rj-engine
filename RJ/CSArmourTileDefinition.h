#pragma once

#include "ComplexShipTileDefinition.h"
class ComplexShipTile;


class CSArmourTileDefinition: public ComplexShipTileDefinition
{

public:

	// Default constructor
	CSArmourTileDefinition(void);

	// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
	CMPINLINE void				ApplyClassSpecificDefinition(ComplexShipTile *tile) const { }

	// Virtual method to read class-specific XML data for the tile
	CMPINLINE void				ReadClassSpecificXMLData(TiXmlElement *node) { }

	// Key properties of the tile


	// Default destructor
	~CSArmourTileDefinition(void);

protected:




};
