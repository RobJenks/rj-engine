#include "DataPorts.h"

// Indicates whether a connection can be made between the specified port types
bool DataPorts::PortTypesAreCompatible(PortType port0, PortType port1)
{
	// TODO: this can become (port0 != port1) in future if we only ever implement {input, output} port types
	return ((port0 == PortType::InputPort && port1 == PortType::OutputPort) ||
			(port0 == PortType::OutputPort && port1 == PortType::InputPort));
}

// Custom string serialisation for data values
std::string DataPorts::DataType::str(void) const
{
	std::ostringstream ss;
	ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw((sizeof(IntValue) / 4) * 8) << static_cast<int>(IntValue) << std::nouppercase << std::dec;
	return ss.str();
}

