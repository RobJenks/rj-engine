#include "Logging.h"
#include "DataObjectDebugLogger.h"

// Default constructor
DataObjectDebugLogger::DataObjectDebugLogger(void)
	:
	PORT_RECEIVE(DataPorts::NO_PORT_INDEX)
{
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectDebugLogger::InitialiseDynamicTerrain(void)
{
	// Initialise the data ports required for this object
	InitialiseDataPorts();
}

// Initialise the data ports required for this object
void DataObjectDebugLogger::InitialiseDataPorts(void)
{
	PORT_RECEIVE = RegisterNewPort(DataPorts::PortType::InputPort);
}


// Method invoked when this object receives data through one of its public input ports
void DataObjectDebugLogger::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	Game::Log << LOG_DEBUG << "Data logging object \"" << this << "\" receieved data \"" << data.str() << "\" at port index " << port_index << " (received from p-id " << source_port << ")\n";
	
	if (port_index != PORT_RECEIVE)
	{
		Game::Log << LOG_DEBUG << "Warning: object \"" << this << "\" received data at port " << port_index << " which is not the designated receiver port " << PORT_RECEIVE << "\n";
	}
}