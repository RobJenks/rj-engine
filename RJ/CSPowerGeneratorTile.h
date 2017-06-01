#pragma once 

#include "ComplexShipTile.h"
#include "Power.h"
class CSPowerGeneratorTileDefinition;


class CSPowerGeneratorTile : public ComplexShipTile
{
public:

	// Default constructor
	CSPowerGeneratorTile(void);

	// We store a direct reference to the power generator definition for more efficient runtime access
	const CSPowerGeneratorTileDefinition *	GetPowerGeneratorTileDefinition(void) const										{ return m_powergeneratordef; }
	void									StorePowerGeneratorTileDefinition(const CSPowerGeneratorTileDefinition *def)	{ m_powergeneratordef = def; }

	CMPINLINE Power::Type					GetMaxNormalPowerOutput(void) const												{ return m_max_output; }
	CMPINLINE void							SetMaxNormalPowerOutput(Power::Type output)										{ m_max_output = max(0, output); }
	CMPINLINE float							GetMaximumOverloadMultiplier(void) const										{ return m_max_overload_modifier; }
	CMPINLINE Power::Type					GetMaximumOverloadPowerOutput(void) const										{ return m_max_overload_output; }

	// Sets the maximum overload modifier that can be applied to maximum power output
	void									SetMaximumOverloadMultiplier(float overload_multiplier);

	// Default destructor
	~CSPowerGeneratorTile(void);

protected:

	const CSPowerGeneratorTileDefinition *				m_powergeneratordef;

	Power::Type											m_max_output;
	
	float												m_max_overload_modifier;
	Power::Type											m_max_overload_output;

};










