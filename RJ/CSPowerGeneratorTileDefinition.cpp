#include "FileInput.h"
#include "CSPowerGeneratorTile.h"
#include "CSPowerGeneratorTileDefinition.h"


// Default constructor
CSPowerGeneratorTileDefinition::CSPowerGeneratorTileDefinition(void)
	: 
	m_max_output(1), m_change_rate(1), m_max_overload_modifier(1.0f)
{
}

// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
void CSPowerGeneratorTileDefinition::ApplyClassSpecificDefinition(ComplexShipTile *tile) const
{
	// We know this is a power generator tile
	if (!tile) return;
	CSPowerGeneratorTile *t = (CSPowerGeneratorTile*)tile;

	// Store a reference back to this definition for more efficient data lookup
	t->StorePowerGeneratorTileDefinition(this);

	// Pass relevant fields to the tile
	t->SetMaximumOutput(m_max_output);
	t->SetChangeRate(m_change_rate);
	t->SetOverloadMultiplier(m_max_overload_modifier);
}

// Virtual method to read class-specific XML data for the tile
void CSPowerGeneratorTileDefinition::ReadClassSpecificXMLData(TiXmlElement *node)
{
	// Parameter check
	if (!node) return;
	std::string key, val;
	HashVal hash;

	// Process each sub-node in turn
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		key = child->Value(); StrLowerC(key);
		hash = HashString(key);

		if (hash == HashedStrings::H_MaxPowerOutput)					m_max_output = IO::GetIntValue(child);
		else if (hash == HashedStrings::H_PowerLevelChangeRate)			m_change_rate = IO::GetIntValue(child);
		else if (hash == HashedStrings::H_PowerOverloadMultiplier)		m_max_overload_modifier = IO::GetFloatValue(child);
		/* else if ... */
	}
}



// Default destructor
CSPowerGeneratorTileDefinition::~CSPowerGeneratorTileDefinition(void)
{

}



