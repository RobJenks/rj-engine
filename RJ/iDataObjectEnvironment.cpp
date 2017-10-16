#include <algorithm>
#include <assert.h>
#include "Logging.h"
#include "DataEnabledObject.h"
#include "iDataObjectEnvironment.h"


// Default constructor
iDataObjectEnvironment::iDataObjectEnvironment(void)
	:
	m_active_port_count(0U)
{
}

// Registers a new data-enabled object with this environment
void iDataObjectEnvironment::RegisterDataEnabledObject(DataEnabledObject *object)
{
	// Parameter check
	if (!object) return;

	// Add each port owned by the object in turn
	for (auto & port : object->GetPorts())
	{
		// Get a new port ID and store a reference to the port in this slot
		DataPorts::PortID port_id = GetNewDataPortID();
		StoreNewDataPortReference(port_id, object, port.GetPortIndex());

		// Notify the port of its new unique ID
		object->AssignUniquePortID(port.GetPortIndex(), port_id);
	}
}

// Removes a data-enabled object from this environment
void iDataObjectEnvironment::UnregisterDataEnabledObject(DataEnabledObject *object)
{
	// Parameter check
	if (!object) return;

	// Remove each port owned by the object in turn
	for (auto & port : object->GetPorts())
	{
		// Attempt to locate this port in the collection
		DataPorts::PortID port_id = GetPortID(object, port.GetPortIndex());
		if (port_id == DataPorts::NO_PORT_ID || port_id != port.GetPortID())
		{
			Game::Log << LOG_ERROR << "Error when attempting to unregister object \"" << object << "\" port with index " << port.GetPortIndex() << "; environment reported port ID of " << port_id << " (0 == not found), port definition has ID = " << port.GetPortID() << "\n";
			continue;
		}

		// Deactive the port in this collection and notify the object that its port is no longer active
		DeactivateDataPortReference(port_id);
		object->RemoveUniquePortID(port.GetPortIndex());
	}
}

// Return the next free data port ID; either the first inactive entry, or a new entry if all are currently active
DataPorts::PortID iDataObjectEnvironment::GetNewDataPortID(void)
{
	// Debug sanity validation; should always be true or something is very wrong
	assert(m_active_port_count <= m_data_ports.size());	

	// If all ports are currently active we simply need to add a new one
	if (m_active_port_count == m_data_ports.size())
	{
		size_t id = m_data_ports.size();

		m_data_ports.push_back(DataPortReference(false, id, NULL, 0U));
		return id;
	}

	// Otherwise, find the next inactive element in the collection and return its index
	size_t n = m_data_ports.size();
	for (size_t i = 0; i < n; ++i)
	{
		if (!m_data_ports[i].IsActive) return i;
	}

	Game::Log << LOG_ERROR << "Data port collection for \"" << this << "\" found to be corrupt; active=" << m_active_port_count << ", total=" << m_data_ports.size() << ", but no inactive ports found\n";
	return DataPorts::NO_PORT_ID;
}

// Store a new data port reference at the given index
void iDataObjectEnvironment::StoreNewDataPortReference(DataPorts::PortID port_id, DataEnabledObject *object, DataPorts::PortIndex object_port_index)
{
	// Parameter check
	if (port_id == DataPorts::NO_PORT_ID || !object)
	{
		Game::Log << LOG_ERROR << "Invalid request to store new data port reference at ID " << port_id << "\n";
		return;
	}

	// Make sure this is not already an active port
	if (m_data_ports[port_id].IsActive)
	{
		Game::Log << LOG_ERROR << "Attempted to store new data port reference at ID " << port_id << " but an active port is already present\n";
		return;
	}

	// Store the new reference and increment the active port could
	m_data_ports.emplace(m_data_ports.begin() + port_id, DataPortReference(true, port_id, object, object_port_index));
	++m_active_port_count;
}

// Deactivate the data port reference with the given ID
void iDataObjectEnvironment::DeactivateDataPortReference(DataPorts::PortID port_id)
{
	// Parameter check
	if (port_id == DataPorts::NO_PORT_ID || port_id > m_data_ports.size())
	{
		Game::Log << LOG_ERROR << "Invalid request to deactivate port ID " << port_id << " (total size = " << m_data_ports.size() << ")\n";
		return;
	}

	// Make sure this is actually an active port
	if (!m_data_ports[port_id].IsActive)
	{
		Game::Log << LOG_ERROR << "Attempted to deactive data port ID " << port_id << " which is already inactive\n";
		return;
	}

	// Set the port to inactive and decrement the active port count
	m_data_ports[port_id].IsActive = false;
	--m_active_port_count;
}

// Returns the ID of a port matching the given criteria, or NO_PORT_ID if no such port exists
DataPorts::PortID iDataObjectEnvironment::GetPortID(const DataEnabledObject *object, DataPorts::PortIndex port_index) const 
{
	auto it = std::find_if(m_data_ports.begin(), m_data_ports.end(),
		[&object, port_index](const DataPortReference & entry) { return (entry.DataObject == object && entry.ObjectPortIndex == port_index && entry.IsActive); });

	return (it == m_data_ports.end() ? DataPorts::NO_PORT_ID : (*it).ID);
}

// Returns details for the given port ID, or the null port reference at NO_PORT_ID if the given ID is not valid
iDataObjectEnvironment::DataPortReference & iDataObjectEnvironment::GetPortDetails(DataPorts::PortID port_id)
{
	return m_data_ports[IsValidPortID(port_id) ? port_id : DataPorts::NO_PORT_ID];
}

// Indicates whether the specified port ID is valid and active
bool iDataObjectEnvironment::IsValidPortID(DataPorts::PortID port_id) const 
{
	return (port_id != DataPorts::NO_PORT_ID && port_id < m_data_ports.size() && m_data_ports[port_id].IsActive);
}

// Checks the given port ID and verifies that it is correct and active
Result iDataObjectEnvironment::VerifyPort(DataPorts::PortID port_id) const
{
	// Make sure the port ID is a valid index into the port collection
	if (!IsValidPortID(port_id)) return ErrorCodes::PortIDIsNotValid;

	// Verify that target object is valid, though this should always be true (or something is wrong with register/unregister logic)
	const DataPortReference & port = m_data_ports[port_id];
	if (port.DataObject == NULL) return ErrorCodes::DataPortObjectIsNotValid;

	// Make sure that the target port within the object is valid; again, this should be valid unless the object registration process is broken
	if (!port.DataObject->HasDataPort(port.ObjectPortIndex)) return ErrorCodes::PortIndexIsNotValid;

	// All validation passed
	return ErrorCodes::NoError;
}

// Connects the two specified ports together, or returns an error code if the connection is not valid or possible
Result iDataObjectEnvironment::ConnectPorts(DataPorts::PortID port_id_0, DataPorts::PortID port_id_1)
{
	// Verify both ports before proceeding	
	Result result;
	if ((result = VerifyPort(port_id_0)) != ErrorCodes::NoError) return result;
	if ((result = VerifyPort(port_id_1)) != ErrorCodes::NoError) return result;

	// A port cannot connect to itself
	if (port_id_0 == port_id_1) return ErrorCodes::CannotConnectDataPortToItself;

	// We can only connect an input to an output port, and vice versa; make sure port types are compatible
	const DataPortReference & p0 = m_data_ports[port_id_0];
	const DataPortReference & p1 = m_data_ports[port_id_1]; 
	const DataPort & port0 = p0.DataObject->GetPort(p0.ObjectPortIndex);
	const DataPort & port1 = p1.DataObject->GetPort(p1.ObjectPortIndex);
	if (!DataPorts::PortTypesAreCompatible(port0.GetPortType(), port1.GetPortType())) return ErrorCodes::PortTypesAreIncompatible;

	// The connection is valid, so notify both ports that they can make the connection
	p0.DataObject->MakeConnection(p0.ObjectPortIndex, port_id_1);
	p1.DataObject->MakeConnection(p1.ObjectPortIndex, port_id_0);

	// Return success
	return ErrorCodes::NoError;
}

// Disconnect the two specified ports, or returns an error code if the connection cannot be broken for some reason
Result iDataObjectEnvironment::DisconnectPorts(DataPorts::PortID port_id_0, DataPorts::PortID port_id_1)
{
	// Verify both ports before proceeding	
	Result result;
	if ((result = VerifyPort(port_id_0)) != ErrorCodes::NoError) return result;
	if ((result = VerifyPort(port_id_1)) != ErrorCodes::NoError) return result;

	// Get a reference to each object and make sure they are valid
	const DataPortReference & p0 = m_data_ports[port_id_0];
	const DataPortReference & p1 = m_data_ports[port_id_1];
	DataEnabledObject *obj0 = p0.DataObject;
	DataEnabledObject *obj1 = p1.DataObject;
	if (!obj0 || !obj1) return ErrorCodes::DataPortObjectIsNotValid;

	// All validations have been completed, so notify each object that they can break the connection
	obj0->BreakConnection(p0.ObjectPortIndex);
	obj1->BreakConnection(p1.ObjectPortIndex);

	// Return success
	return ErrorCodes::NoError;
}

// Transmit data between the two given ports
void iDataObjectEnvironment::TransmitData(DataPorts::PortID source_port, DataPorts::PortID target_port, DataPorts::DataType data)
{
	// We only need to resolve the target port
	if (!IsValidPortID(target_port)) return;

	// Pass this data to the target
	DataPortReference & port = m_data_ports[target_port];
	if (port.DataObject) port.DataObject->DataReceieved(port.ObjectPortIndex, data, source_port);
}


// Default destructor
iDataObjectEnvironment::~iDataObjectEnvironment(void)
{

}

