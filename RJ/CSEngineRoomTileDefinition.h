#pragma once

#include "ComplexShipTileDefinition.h"
class ComplexShipTile;



class CSEngineRoomTileDefinition : public ComplexShipTileDefinition
{

public:

	// Default constructor
	CSEngineRoomTileDefinition(void);

	// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
	void						ApplyClassSpecificDefinition(ComplexShipTile *tile) const;

	// Virtual method to read class-specific XML data for the tile
	void						ReadClassSpecificXMLData(TiXmlElement *node);

	// Key properties of the engine room tile


	// Default destructor
	~CSEngineRoomTileDefinition(void);

protected:




};
