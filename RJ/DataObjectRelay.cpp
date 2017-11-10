#include "DataObjectRelay.h"

// Default constructor
DataObjectRelay::DataObjectRelay(void)
	:
	PORT_SEND(DataPorts::NO_PORT_INDEX), PORT_RECEIVE(DataPorts::NO_PORT_INDEX)
{
}


// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
DataObjectRelay * DataObjectRelay::Create(const TerrainDefinition *def)
{
	// Create and initialise the underlying terrain object
	DataObjectRelay *object = new DataObjectRelay();
	object->InitialiseNewTerrain(def);

	// Initialise the data ports required for this object
	object->InitialiseDataPorts();

	// Return the new object
	return object;
}

// Initialise the data ports required for this object
void DataObjectRelay::InitialiseDataPorts(void)
{
	PORT_RECEIVE = RegisterNewPort(DataPorts::PortType::InputPort);
	PORT_SEND = RegisterNewPort(DataPorts::PortType::OutputPort);
}


// Method invoked when this object receives data through one of its public input ports
// A relay object will simply forward the signal it receives to its output port
void DataObjectRelay::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	if (port_index == PORT_RECEIVE)
	{
		SendData(PORT_SEND, data);
	}
}