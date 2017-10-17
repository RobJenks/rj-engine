#pragma once

#include "DataEnabledStaticTerrain.h"


class DataObjectDebugLogger : public DataEnabledStaticTerrain
{
public:

	// Default constructor
	DataObjectDebugLogger(void);

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectDebugLogger *		Create(const StaticTerrainDefinition *def);

	// Initialise the data ports required for this object
	void								InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port);

	// Return port indices for the object
	CMPINLINE DataPorts::PortIndex		InputPort(void) const { return PORT_RECEIVE; }
	CMPINLINE DataPorts::PortIndex		OutputPort(void) const { return PORT_SEND; }


private:

	// Maintain port indices for convenience
	DataPorts::PortIndex				PORT_SEND;
	DataPorts::PortIndex				PORT_RECEIVE;

};