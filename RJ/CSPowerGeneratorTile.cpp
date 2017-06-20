#include "ModifiedValue.h"
#include "AdjustableParameter.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipTileDefinition.h"
#include "CSPowerGeneratorTile.h"

// Default constructor
CSPowerGeneratorTile::CSPowerGeneratorTile(void)
{
	// Power generator tiles do require simulation by default
	ActivateSimulation(Game::C_TILE_POWERGENERATOR_SIMULATION_INTERVAL);

	// Set default parameter values
	m_max_output.SetBaseValue(1);
	m_change_rate.SetBaseValue(1);
	m_current_output = AdjustableParameter<Power::Type>(0, 1, 0, 0, 1);
	m_overload_multiplier = 1.0f;
}

// Set the maximum power level that can be output by this generator, updating other dependent fields as required
void CSPowerGeneratorTile::SetMaximumOutput(Power::Type max_output)
{
	m_max_output.SetBaseValue(max_output);

	m_current_output.Max = m_max_output.Value;
	RecalculateParameters();
}

// Set the power level change rate of this generator, updating other dependent fields as required
void CSPowerGeneratorTile::SetChangeRate(Power::Type change_rate)
{
	m_change_rate.SetBaseValue(change_rate);

	m_current_output.ChangeRate = m_change_rate.Value;
	RecalculateParameters();
}

// Sets the target power level to the specified value
void CSPowerGeneratorTile::SetPowerOutputTarget(Power::Type target)
{
	m_current_output.Target = target;
	RecalculateParameters();
}

// Sets the target power level to the specified value
void CSPowerGeneratorTile::SetPowerOutputTargetPc(float target_pc)
{
	SetPowerOutputTarget((Power::Type)(target_pc * (float)m_max_output.Value));
}

// Simulation method for this tile
void CSPowerGeneratorTile::PerformTileSimulation(unsigned int delta_ms)
{
	// Update the current power level
	m_current_output.Update(delta_ms);

	// The environment needs to update its power simulation based on our update
	m_parent->UpdatePower();

	// We want to continue simulating the tile if we are not yet at target power levels
	SetTileSimulationRequired( !AtTargetPowerLevel() );
}


// Virtual inherited method to make a copy of this tile and return it
ComplexShipTile * CSPowerGeneratorTile::Copy(void) const
{
	// Call the copy constructor, which will in turn call the base class copy constructor, to create a copy of this tile
	return new CSPowerGeneratorTile(*this);
}

// Virtual & static methods respectively to generate and read XML data representing the tile
TiXmlElement * CSPowerGeneratorTile::GenerateXML(void)
{
	// Create a new node to hold this data
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipTile);
	node->SetAttribute("code", m_definition->GetCode().c_str());

	// First, have the base class generate XML for all common tile properties
	TiXmlElement *base = ComplexShipTile::GenerateBaseClassXML(this);
	if (!base) { delete node; return NULL; }
	node->LinkEndChild(base);

	// Now generate XML for all properties specific to this tile class
	/* No tile-specific properties right now */
	// TODO: NOT TRUE

	// Return a reference to the new node
	return node;
}

// Virtual method to read any class-specific data for this tile type
void CSPowerGeneratorTile::ReadClassSpecificXMLData(TiXmlElement *node)
{
	// TODO: DO THIS
}

// Default destructor
CSPowerGeneratorTile::~CSPowerGeneratorTile(void)
{

}

// Processes a debug tile command from the console
void CSPowerGeneratorTile::ProcessDebugTileCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros for convenience
	INIT_DEBUG_TILE_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, Parameter(2) is the already-matched tile ID, and Parameter(3) is the function name
	// We therefore pass Parameter(4) onwards as arguments

	// Accessor methods
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetPowerOutput)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetTargetPowerLevel)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(AtTargetPowerLevel)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetMaximumOutput)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetChangeRate)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetOverloadMultiplier)

	// Mutator methods
	REGISTER_DEBUG_TILE_FN(PerformTileSimulation, (unsigned int)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetPowerOutputTarget, (Power::Type)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetMaximumOutput, (Power::Type)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetChangeRate, (Power::Type)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetOverloadMultiplier, command.ParameterAsFloat(4))
	REGISTER_DEBUG_TILE_FN(RecalculateParameters)

	// Pass back to our base class if we could not handle the command
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		ComplexShipTile::ProcessDebugTileCommand(command);

}



