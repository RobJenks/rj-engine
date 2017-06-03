#include "XML\tinyxml.h"
#include "CSLifeSupportTile.h"
#include "CSLifeSupportTileDefinition.h"

// Default constructor
CSLifeSupportTileDefinition::CSLifeSupportTileDefinition(void)
{
	// Set the tile class type
	m_classtype = D::TileClass::LifeSupport;

	// We do have class-specific data for this tile type
	RegisterClassSpecificDataForTileDefinition();

	// Initialise default values
	m_gravity_range = 20;
	m_gravity_falloffdelay = 0.25f;
	m_gravity_exponent = 3;
	
	// Gravity strength table starts uninitialised
	m_gravity_strength = NULL;

}

// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
void CSLifeSupportTileDefinition::ApplyClassSpecificDefinition(ComplexShipTile *tile) const
{
	// We know this is a life support system tile
	if (!tile) return;
	CSLifeSupportTile *t = (CSLifeSupportTile*)tile;

	// Store a direct reference to this life support tile definition, for more efficient lookups at runtime
	t->StoreLifeSupportTileDefinition(this);

	// Set default starting values for gravity and oxygen, based on the tile definition
	t->Gravity = InitialGravity;
	t->OxygenLevel = InitialOxygenLevel;
	t->OxygenRange = InitialOxygenRange;

	// Also pass the maximum gravity range to the target tile
	t->SetGravityRange(m_gravity_range);
}


// Recalculates all derived values in the definition based on the current properties
void CSLifeSupportTileDefinition::RecalculateDefinitionProperties(void)
{
	// Recalculate the gravity data based on current properties
	RecalculateGravityData();

	// Recalculate oxygen supply delivered by this tile
	RecalculateOxygenData();
}

// Recalculates all gravity data based on the current tile properties
void CSLifeSupportTileDefinition::RecalculateGravityData(void)
{
	// First, limit our properties to acceptable values, in case they have been set out of bounds
	m_gravity_range = clamp(m_gravity_range, 0, 100);
	m_gravity_falloffdelay = clamp(m_gravity_falloffdelay, 0.0f, 1.0f);
	m_gravity_exponent = clamp(m_gravity_exponent, 0, 6);

	// Now allocate a table of gravity strength values from 0 to the maximum gravity range, which will then be applied to elements in range
	// Make sure to deallocate any existing data first
	if (m_gravity_strength) SafeDeleteArray(m_gravity_strength);
	m_gravity_strength = new float[m_gravity_range + 1];				// We want to go from 0 to Range inclusive, since el[Range] is still within the gravity effect

	// Now populate the gravity strength table based upon the parameters that have been set
	float strength;
	float falloff_delay_extent = (m_gravity_range * m_gravity_falloffdelay);
	for (int i = 0; i <= m_gravity_range; ++i)
	{
		// Determine the percentage of total range that this is, adjusted by the falloff delay (to make 0% extend further into the range)
		strength = max(0.0f, (float)i - falloff_delay_extent) / (float)m_gravity_range;

		// Raise to the power specified in gravity exponent
		strength = pow(strength, m_gravity_exponent);

		// Invert the percentage so that 100% begins at i=0, clamp to ensure all values are within the range (0.0 - 1.0), and store the strength value
		m_gravity_strength[i] = clamp(1.0f - strength, 0.0f, 1.0f);
	}
}


// Recalculates all oxygen supply data based on the current tile properties
void CSLifeSupportTileDefinition::RecalculateOxygenData(void)
{
	// TODO: Do this
}

// Virtual method to read class-specific XML data for the tile
void CSLifeSupportTileDefinition::ReadClassSpecificXMLData(TiXmlElement *node)
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

		if (hash == HashedStrings::H_InitialGravity)						InitialGravity.ReadDataFromXML(child);
		else if (hash == HashedStrings::H_InitialOxygenLevel)				InitialOxygenLevel.ReadDataFromXML(child);
		else if (hash == HashedStrings::H_InitialOxygenRange)				InitialOxygenRange.ReadDataFromXML(child);
		else if (hash == HashedStrings::H_GravityRange)
		{
			val = child->GetText(); if (val == NullString) continue;
			SetGravityRange(atoi(val.c_str()));
		}
		else if (hash == HashedStrings::H_GravityFalloffDelay)
		{
			val = child->GetText(); if (val == NullString) continue;
			SetGravityFalloffDelay((float)atof(val.c_str()));
		}
		else if (hash == HashedStrings::H_GravityExponent)
		{
			val = child->GetText(); if (val == NullString) continue;
			SetGravityExponent(atoi(val.c_str()));
		}
	}

	// Perform a full recalculation of derived properties once all data has been read
	RecalculateDefinitionProperties();
}


// Default destructor
CSLifeSupportTileDefinition::~CSLifeSupportTileDefinition(void)
{
	// Deallocate the gravity strength data, if appropriate
	if (m_gravity_strength) 
	{
		SafeDeleteArray(m_gravity_strength);
	}
}



