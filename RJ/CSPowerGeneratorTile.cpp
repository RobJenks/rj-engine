#include "CSPowerGeneratorTile.h"


// Default constructor
CSPowerGeneratorTile::CSPowerGeneratorTile(void)
	:
	m_max_output(1), m_max_overload_modifier(1.0f), m_max_overload_output(1)
{
}


// Sets the maximum overload modifier that can be applied to maximum power output
void CSPowerGeneratorTile::SetMaximumOverloadMultiplier(float overload_multiplier)
{
	m_max_overload_modifier = overload_multiplier;
	m_max_overload_output = (Power::Type)((float)m_max_output * m_max_overload_modifier);
}

// Default destructor
CSPowerGeneratorTile::~CSPowerGeneratorTile(void)
{

}

*** IMPLEMENT REMAINDER OF POWER GENERATOR TILE AND ADD TO TEST SHIP AT ELEMENT 194 ***

