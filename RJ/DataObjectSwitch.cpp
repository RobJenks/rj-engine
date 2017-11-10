#include "DynamicTerrainDefinition.h"
#include "DataObjectSwitch.h"


// Default constructor
DataObjectSwitch::DataObjectSwitch(void)
	:
	PORT_SEND(DataPorts::NO_PORT_INDEX)
{
}

// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
DataObjectSwitch * DataObjectSwitch::Create(const TerrainDefinition *def)
{
	// Create and initialise the underlying terrain object
	DataObjectSwitch *object = new DataObjectSwitch();
	object->InitialiseNewTerrain(def);

	// Initialise the data ports required for this object
	object->InitialiseDataPorts();

	// Return the new object
	return object;
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

	// Invoke the default transition based on our current state
	ExecuteDefaultStateTransition();

	// Determine whether a transition took place
	std::string state = GetState();
	if (state != previous_state)
	{
		const DynamicTerrainState *state_def = m_dynamic_terrain_def->GetStateDefinition(state);
		if (state_def)
		{
			// The switch outputs a signed int value corresponding to the state ID when a transition takes place
			SendData(PORT_SEND, DataPorts::DataType(state_def->GetStateID()));
		}
	}
	
}