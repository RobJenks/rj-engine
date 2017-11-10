#pragma once

#include "DynamicTerrain.h"


DYNAMIC_TERRAIN_CLASS(DataObjectSwitch)
//{
public:

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectSwitch *					Create(const TerrainDefinition *def);

	// Initialise the data ports required for this object
	void										InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports; switches will not receive any data
	CMPINLINE void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) { }

	// Method invoked when this switch is used by an entity
	virtual bool								OnUsed(iObject *user);

	// Activates the switch
	void										ActivateSwitch(void);

	// Returns the index of the switch output port
	CMPINLINE DataPorts::PortIndex				OutputPort(void) const { return PORT_SEND; }


protected:

	// Default constructor
	DataObjectSwitch(void);

	// Maintain port indices for convenience
	DataPorts::PortIndex						PORT_SEND;
	
};