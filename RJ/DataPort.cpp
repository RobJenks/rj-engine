#include "DataPort.h"

// Constructor; creates a new port
DataPort::DataPort(DataPorts::PortIndex port_index, DataPorts::PortType port_type)
	:
	m_id(DataPorts::NO_PORT_ID), m_index(port_index), m_type(port_type), 
	m_connected_port(DataPorts::NO_PORT_ID)
{
}

// Set the uniquely-identifying ID of this port
void DataPort::AssignPortID(DataPorts::PortID id)
{
	m_id = id;
}

// Remove the unique ID registered to this port
void DataPort::RevokePortID(void)
{
	m_id = DataPorts::NO_PORT_ID;
}

// Set the ID of the port that this connected to this one; either the source of our input, or 
// the target for our output (or 0 if not connected)
void DataPort::SetConnection(DataPorts::PortID port_id)
{
	m_connected_port = port_id;
}

