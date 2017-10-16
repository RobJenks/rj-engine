#pragma once

#include "CompilerSettings.h"
#include "DataPorts.h"

class DataPort
{
public:

	// Constructor; creates a new port
	DataPort(DataPorts::PortIndex port_index, DataPorts::PortType port_type);

	// Return data on the port
	CMPINLINE DataPorts::PortID				GetPortID(void) const			{ return m_id; }
	CMPINLINE DataPorts::PortIndex			GetPortIndex(void) const		{ return m_index; }
	CMPINLINE DataPorts::PortType			GetPortType(void) const			{ return m_type; }
	CMPINLINE DataPorts::PortID				GetConnectedPort(void) const	{ return m_connected_port; }

	// Indicates whether the port is connected to another
	CMPINLINE bool							IsConnected(void) const			{ return (m_connected_port != DataPorts::NO_PORT_ID); }

	// Set the uniquely-identifying ID of this port
	void									AssignPortID(DataPorts::PortID id);

	// Remove the unique ID registered to this port
	void									RevokePortID(void);

	// Set the ID of the port that this connected to this one; either the source of our input, or 
	// the target for our output (or 0 if not connected)
	void									SetConnection(DataPorts::PortID port_id);


private:

	DataPorts::PortID						m_id;				// Uniquely-identifying ID within the environment; assigned by the environment when it is added to the env
	DataPorts::PortIndex					m_index;			// Incex of this port within the parent object; assigned by the parent during RegisterNewPort()
	DataPorts::PortType						m_type;				// Type of the port: input/output

	DataPorts::PortID						m_connected_port;	// The port that we are connected to; either the source of our input, or the target for our output (or 0 if not connected)

};
