#pragma once

#include "DynamicTerrain.h"


DYNAMIC_TERRAIN_CLASS(DataObjectRelay)
//{
public:

	// Default constructor
	DataObjectRelay(void);

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void								InitialiseDynamicTerrain(void);

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