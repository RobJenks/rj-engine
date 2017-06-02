#pragma once 

#include "ComplexShipTile.h"
#include "Power.h"
#include "ModifiedValue.h"
#include "AdjustableParameter.h"
class CSPowerGeneratorTileDefinition;


class CSPowerGeneratorTile : public ComplexShipTile
{
public:

	// Default constructor
	CSPowerGeneratorTile(void);

	// Public inherited virtual method to return the type of ship tile this object represents
	CMPINLINE D::TileClass					GetClass(void) const	{ return D::TileClass::PowerGenerator; }

	// Static method to return the class; allows this to be determined based on the class and not an instance
	CMPINLINE static D::TileClass			GetClassStatic(void)	{ return D::TileClass::PowerGenerator; }

	// Simulation method for this tile
	void									PerformTileSimulation(unsigned int delta_ms);

	// We store a direct reference to the power generator definition for more efficient runtime access
	const CSPowerGeneratorTileDefinition *	GetPowerGeneratorTileDefinition(void) const										{ return m_powergeneratordef; }
	void									StorePowerGeneratorTileDefinition(const CSPowerGeneratorTileDefinition *def)	{ m_powergeneratordef = def; }

	// Return the current power level
	CMPINLINE Power::Type								GetPowerOutput(void) const											{ return m_current_output.Value; }

	// Sets the target power level to the specified value
	void												SetPowerOutputTarget(Power::Type target);
	void												SetPowerOutputTargetPc(float target_pc);

	CMPINLINE bool										AtTargetPowerLevel(void) const										{ return m_current_output.IsAtTarget(); }
	
	CMPINLINE Power::Type								GetMaximumOutput(void) const										{ return m_max_output.Value; }
	CMPINLINE ModifiedValue<Power::Type> &				MaximumOutputValue(void)											{ return m_max_output; }
	void												SetMaximumOutput(Power::Type max_output);

	CMPINLINE Power::Type								GetChangeRate(void) const											{ return m_change_rate.Value; }
	CMPINLINE ModifiedValue<Power::Type> &				ChangeRateValue(void)												{ return m_change_rate; }
	void												SetChangeRate(Power::Type change_rate);

	CMPINLINE float										GetOverloadMultiplier(void) const									{ return m_overload_multiplier; }
	CMPINLINE void										SetOverloadMultiplier(float overload_multiplier)					{ m_overload_multiplier = overload_multiplier; }

	// Recalculates all values following a change to our parameters
	CMPINLINE void										RecalculateParameters(void)
	{
		m_current_output.EnsureValid();
		SetTileSimulationRequired( !AtTargetPowerLevel() );
	}


	// Apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship.  Inherited virtual.
	CMPINLINE void										ApplyTileSpecific(void) { }

	// Virtual inherited method to make a copy of this tile and return it
	ComplexShipTile *									Copy(void) const;
	
	// Virtual & static methods respectively to generate and read XML data representing the tile
	TiXmlElement *										GenerateXML(void);						// Virtual inherited since is called on an instance of a tile

	// Virtual method to read any class-specific data for this tile type
	void												ReadClassSpecificXMLData(TiXmlElement *node);

	// Default destructor
	~CSPowerGeneratorTile(void);

protected:

	const CSPowerGeneratorTileDefinition *				m_powergeneratordef;				// Pointer to our tile definition

	AdjustableParameter<Power::Type>					m_current_output;					// Current power level

	ModifiedValue<Power::Type>							m_max_output;						// Maximum normal power output by this generator
	ModifiedValue<Power::Type>							m_change_rate;						// Rate at which power level changes towards target, per sec

	float												m_overload_multiplier;				// Maximum possible multiplier to power output when we overload the generator

};










