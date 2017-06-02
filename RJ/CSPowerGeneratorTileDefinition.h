#pragma once

#include "ComplexShipTileDefinition.h"
#include "Power.h"

class CSPowerGeneratorTileDefinition : public ComplexShipTileDefinition
{

public:

	// Default constructor
	CSPowerGeneratorTileDefinition(void);

	// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
	void						ApplyClassSpecificDefinition(ComplexShipTile *tile) const;

	// Virtual method to read class-specific XML data for the tile
	void						ReadClassSpecificXMLData(TiXmlElement *node);

	
	// Default destructor
	~CSPowerGeneratorTileDefinition(void);

protected:

	Power::Type					m_max_output;
	Power::Type					m_change_rate;
	float						m_max_overload_modifier;
	
};





