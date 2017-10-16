#pragma once

#include <vector>

class DataPorts
{
public:

	// Uniquely-identifying index to a port within a specific environment
	typedef size_t							PortID;

	// Indicates that the object has not yet been assigned a unique port ID
	static const PortID						NO_PORT_ID = ((PortID)0U - (PortID)1U);

	// Index of a port within the scope of its parent object
	typedef size_t							PortIndex;

	// Available port types
	enum									PortType
	{
											InputPort = 0,
											OutputPort
	};

	// The type of data transmitted by all data ports
	typedef float							DataType;

	// Indicates whether a connection can be made between the specified port types
	static bool								PortTypesAreCompatible(PortType port0, PortType port1);


private:


};
