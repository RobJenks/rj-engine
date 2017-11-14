#include "DynamicTerrainDefinition.h"
#include "DataObjectSwitch.h"




#include "Logging.h" // ***************




// Default constructor
DataObjectSwitch::DataObjectSwitch(void)
	:
	PORT_SEND(DataPorts::NO_PORT_INDEX)
{
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectSwitch::InitialiseDynamicTerrain(void)
{
	// Initialise the data ports required for this object
	InitialiseDataPorts();
}

// Initialise the data ports required for this object
void DataObjectSwitch::InitialiseDataPorts(void)
{
	PORT_SEND = RegisterNewPort(DataPorts::PortType::OutputPort);
}

// Method invoked when this switch is used by an entity
bool DataObjectSwitch::OnUsed(iObject *user)
{
	// Activate the switch and return success
	ActivateSwitch();
	return true;
}

// Activates the switch
void DataObjectSwitch::ActivateSwitch(void)
{
	// Record the previous state so we can check whether a transition took place
	std::string previous_state = GetState();
	Game::Log << LOG_DEBUG << "Previous state: " << previous_state << "\n";
	// Invoke the default transition based on our current state
	ExecuteDefaultStateTransition();

	// Determine whether a transition took place
	std::string state = GetState();
	Game::Log << LOG_DEBUG << "New state: " << state << "\n";
	if (state != previous_state)
	{
		const DynamicTerrainState *state_def = m_dynamic_terrain_def->GetStateDefinition(state);
		if (state_def)
		{
			// The switch outputs a signed int value corresponding to the state ID when a transition takes place
			Game::Log << LOG_DEBUG << "Sending state ID of " << state_def->GetStateID() << " for state \"" << state_def->GetStateCode() << "\"\n";
			SendData(PORT_SEND, DataPorts::DataType(state_def->GetStateID()));
		}
	}
	
}