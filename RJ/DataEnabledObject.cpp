#include <assert.h>
#include "Logging.h"
#include "DataEnabledObject.h"
#include "iDataObjectEnvironment.h"


// Default constructor
DataEnabledObject::DataEnabledObject(void)
	:
	m_dataportcount(0U), m_data_environment(NULL)
{
}

// Register a new port for this object.  Returns the index of the port within this object
// All ports should be registered on object construction, before the object is added
// to an environment or system.  Changes to port layout after this point will not be 
// communicated to the parent environment
DataPorts::PortIndex DataEnabledObject::RegisterNewPort(DataPorts::PortType port_type)
{
	DataPorts::PortIndex port_index = m_dataports.size();
	m_dataports.push_back(DataPort(port_index, port_type));
	m_dataportcount = m_dataports.size();

	return port_index;
}

// Connect a port to the specified object; either the target for our output, or the source for our input
// Returns a result code indicating whether the connection could be made.  Call sequence is: 
// { ConnectPort() -> Environment::ConnectPorts() -> { obj0::MakeDataConnection(), obj1::MakeDataConnection() }
Result DataEnabledObject::ConnectPort(DataPorts::PortIndex port_index, const DataEnabledObject *target_object, DataPorts::PortIndex target_object_port)
{
	// Make sure this is a valid port before starting, and that we exist in a data-enabled environment
	if (!HasDataPort(port_index)) return ErrorCodes::PortIndexIsNotValid;
	if (!m_data_environment) return ErrorCodes::ObjectHasNoDataEnvironment;

	// Ask the data environment for the ID of the target port, and make sure it is valid
	DataPorts::PortID target_id = m_data_environment->GetPortID(target_object, target_object_port);
	if (target_id == DataPorts::NO_PORT_ID)
	{
		return ErrorCodes::TargetPortIDIsNotValid;
	}

	// Send a request to the data environment to connect us to the target port
	Result result = m_data_environment->ConnectPorts(GetPort(port_index).GetPortID(), target_id);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return ErrorCodes::NoError;
}

// Form a connection to the specified port.  This method should only be called by the parent data environment 
// once all validations are complete on both sides.  In order to request a new connection, use ConnectPort() instead
void DataEnabledObject::MakeConnection(DataPorts::PortIndex port_index, DataPorts::PortID target_port_id)
{
	// Validate parameters, though they should be guaranteed to be valid if this method is only called from the 
	// parent data environment following validations
	assert(HasDataPort(port_index));
	assert(target_port_id != DataPorts::NO_PORT_ID);

	// If the port is already connection, disconnect if before proceeding
	if (m_dataports[port_index].IsConnected())
	{
		DisconnectPort(port_index);
	}

	// Update the relevant port with its new connection target
	m_dataports[port_index].SetConnection(target_port_id);
}

// Disconnect a port.  Returns a result code indicating whether the connection could be broken
Result DataEnabledObject::DisconnectPort(DataPorts::PortIndex port_index)
{
	// Parameter validation
	if (!HasDataPort(port_index)) return ErrorCodes::PortIndexIsNotValid;
	if (!m_data_environment) return ErrorCodes::ObjectHasNoDataEnvironment;

	// Make sure the port is actually connected
	const DataPort & port = GetPort(port_index);
	if (!port.IsConnected()) return ErrorCodes::CannotDisconnectAnUnconnectedPort;

	// Send a request to the data environment to break this connection
	Result result = m_data_environment->DisconnectPorts(port.GetPortID(), port.GetConnectedPort());
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success
	return result;
}

// Break the connection from the specifid port.  This method should only be called by the parent data 
// environment once all validations are complete on both sides.  In order to request a disconnection, use DisconnectPort() instead
void DataEnabledObject::BreakConnection(DataPorts::PortIndex port_index)
{
	// Validate parameters, though they should be guaranteed to be valid if this method is only called from the 
	// parent data environment following validations
	assert(HasDataPort(port_index));

	// Remove the connection from the specified port
	m_dataports[port_index].SetConnection(DataPorts::NO_PORT_ID);
}

// Send data out of a port
void DataEnabledObject::SendData(DataPorts::PortIndex port_index, DataPorts::DataType data)
{
	assert(HasDataPort(port_index));
	assert(m_dataports[port_index].GetPortType() == DataPorts::PortType::OutputPort);
	assert(m_dataports[port_index].GetPortID() != DataPorts::NO_PORT_ID);

	if (m_data_environment)
	{
		const DataPort & port = m_dataports[port_index];
		m_data_environment->TransmitData(port.GetPortID(), port.GetConnectedPort(), data);
	}
}

// Indicates whether the specified port index is valid
bool DataEnabledObject::HasDataPort(DataPorts::PortIndex port_index)
{
	return (port_index < m_dataportcount);
}

// Assigns a unique port ID to the given port in this object; called by the data environment when registering objects
void DataEnabledObject::AssignUniquePortID(DataPorts::PortIndex port_index, DataPorts::PortID port_id)
{
	// Parameter check
	if (!HasDataPort(port_index) || port_id == DataPorts::NO_PORT_ID)
	{
		Game::Log << LOG_ERROR << "Invalid attempt to assign unique port ID of " << port_id << " to object \"" << this << "\" port with index " << port_index << " (total object ports = " << m_dataports.size() << ")\n";
		return;
	}

	// Make sure the port does not already have an ID assigned
	if (m_dataports[port_index].GetPortID() != DataPorts::NO_PORT_ID)
	{
		Game::Log << LOG_ERROR << "Attempted to assign port ID " << port_id << " to object \"" << this << "\" port with index " << port_index << ", but port is already registered with ID " << m_dataports[port_index].GetPortID() << "\n";
		return;
	}

	// All validations passed, to assign the ID and return
	m_dataports[port_index].AssignPortID(port_id);
}

// Removes the unique port ID that was previously assigned to a port
void DataEnabledObject::RemoveUniquePortID(DataPorts::PortIndex port_index)
{
	// Parameter check
	if (!HasDataPort(port_index))
	{
		Game::Log << LOG_ERROR << "Invalid attempt to remove unique port ID from object \"" << this << "\" port with index " << port_index << " (total object ports = " << m_dataports.size() << ")\n";
		return;
	}

	// Make sure the port does actually have an ID assigned
	if (m_dataports[port_index].GetPortID() == DataPorts::NO_PORT_ID)
	{
		Game::Log << LOG_ERROR << "Attempted to remove port ID from object \"" << this << "\" port with index " << port_index << ", but port is not yet registered\n";
		return;
	}

	// All validations passed, so remove the ID and return
	m_dataports[port_index].RevokePortID();
}

