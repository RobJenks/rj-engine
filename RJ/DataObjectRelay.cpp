#include "DataObjectRelay.h"

// Default constructor
DataObjectRelay::DataObjectRelay(void)
	:
	PORT_SEND(DataPorts::NO_PORT_INDEX), PORT_RECEIVE(DataPorts::NO_PORT_INDEX)
{
}


// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectRelay::InitialiseDynamicTerrain(void)
{
	// Initialise the data ports required for this object
	InitialiseDataPorts();
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