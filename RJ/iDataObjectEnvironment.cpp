#include <algorithm>
#include <assert.h>
#include "DataEnabledObject.h"
#include "iDataObjectEnvironment.h"


// Default constructor
iDataObjectEnvironment::iDataObjectEnvironment(void)
{

}

// Returns the ID of a port matching the given criteria, or NO_PORT_ID if no such port exists
DataPorts::PortID iDataObjectEnvironment::GetPortID(const DataEnabledObject *object, DataPorts::PortIndex port_index) const 
{
	auto it = std::find_if(m_data_ports.begin(), m_data_ports.end(),
		[&object, port_index](const DataPortReference & entry) { return (entry.DataObject == object && entry.ObjectPortIndex == port_index); });

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

