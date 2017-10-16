#pragma once

#include "DataPorts.h"
class DataEnabledObject;


// Structure holding a reference to a specific port within the environment
struct DataPortReference
{
	bool						IsActive;
	DataPorts::PortID			ID;
	DataEnabledObject *			DataObject;
	DataPorts::PortIndex		ObjectPortIndex;

	DataPortReference(void) { }
	DataPortReference(bool active, DataPorts::PortID id, DataEnabledObject *object, DataPorts::PortIndex object_port_index)
		: IsActive(active), ID(id), DataObject(object), ObjectPortIndex(object_port_index) { }

	// Single static instance representing the 'null port'
	static DataPortReference	NULL_PORT;
};