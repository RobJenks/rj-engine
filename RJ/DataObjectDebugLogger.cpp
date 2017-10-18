#include "Logging.h"
#include "DataObjectDebugLogger.h"

// Default constructor
DataObjectDebugLogger::DataObjectDebugLogger(void)
	:
	PORT_SEND(DataPorts::NO_PORT_ID), PORT_RECEIVE(DataPorts::NO_PORT_ID)
{
}


// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
DataObjectDebugLogger * DataObjectDebugLogger::Create(const TerrainDefinition *def)
{
	// Create and initialise the underlying terrain object
	DataObjectDebugLogger *object = new DataObjectDebugLogger();
	object->InitialiseNewTerrain(def);

	// Initialise the data ports required for this object
	object->InitialiseDataPorts();

	// Return the new object
	return object;
}

// Initialise the data ports required for this object
void DataObjectDebugLogger::InitialiseDataPorts(void)
{
	PORT_RECEIVE = RegisterNewPort(DataPorts::PortType::InputPort);
	PORT_SEND = RegisterNewPort(DataPorts::PortType::OutputPort);
}


// Method invoked when this object receives data through one of its public input ports
void DataObjectDebugLogger::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	Game::Log << LOG_DEBUG << "Data logging object \"" << this << "\" receieved data \"" << data << "\" at port " << port_index << " (received from p-id " << source_port << ")\n";
	
	if (port_index != PORT_RECEIVE)
	{
		Game::Log << LOG_DEBUG << "Warning: object \"" << this << "\" received data at port " << port_index << " which is not the designated receiver port " << PORT_RECEIVE << "\n";
	}
}