#include "DataPorts.h"

// Indicates whether a connection can be made between the specified port types
bool DataPorts::PortTypesAreCompatible(PortType port0, PortType port1)
{
	// TODO: this can become (port0 != port1) in future if we only ever implement {input, output} port types
	return ((port0 == PortType::InputPort && port1 == PortType::OutputPort) ||
			(port0 == PortType::OutputPort && port1 == PortType::InputPort));
}