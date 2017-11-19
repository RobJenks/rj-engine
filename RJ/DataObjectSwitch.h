#pragma once

#include "DynamicTerrain.h"


DYNAMIC_TERRAIN_CLASS(DataObjectSwitch)
//{
public:

	// Default constructor
	DataObjectSwitch(void);

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void										InitialiseDynamicTerrain(void);

	// Initialise the data ports required for this object
	void										InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports; switches will not receive any data
	CMPINLINE void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) { }

	// Activates the switch
	void										ActivateSwitch(void);

	// Returns the index of the switch output port
	CMPINLINE DataPorts::PortIndex				OutputPort(void) const { return PORT_SEND; }


protected:

	// Maintain port indices for convenience
	DataPorts::PortIndex						PORT_SEND;

	// Method invoked when this switch is used by an entity
	virtual bool								OnUsed(iObject *user, DynamicTerrainInteraction && interaction);
	
};