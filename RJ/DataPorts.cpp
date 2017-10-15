#include "DataPorts.h"

// Indicates whether a connection can be made between the specified port types
bool DataPorts::PortTypesAreCompatible(PortType port0, PortType port1)
{
	return ((port0 == PortType::InputPort && port1 == PortType::OutputPort) ||
			(port1 == PortType::OutputPort && port1 == PortType::InputPort));
}