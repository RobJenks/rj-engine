#pragma once

#include "ErrorCodes.h"
#include "DataPorts.h"
#include "DataPort.h"
#include "iDataObjectEnvironment.h"

class DataEnabledObject
{
public:

	// Default constructor
	DataEnabledObject(void);

	// Connect a port to the specified object; either the target for our output, or the source for our input
	// Returns a result code indicating whether the connection could be made
	Result									ConnectPort(DataPorts::PortIndex port_index, const DataEnabledObject *target_object, DataPorts::PortIndex target_object_port);

	// Disconnect a port.  Returns a result code indicating whether the connection could be broken
	Result									DisconnectPort(DataPorts::PortIndex port_index);

	// Indicates whether the specified port index is valid
	bool									HasDataPort(DataPorts::PortIndex port_index);

	// Returns the number of ports that are registered on this object; only ports with index [0 port_count) are valid
	CMPINLINE DataPorts::PortIndex			GetPortCount(void) const					{ return m_dataportcount; }

	// Returns a reference to the ports owned by this object
	CMPINLINE const std::vector<DataPort> & GetPorts(void) const						{ return m_dataports; }

	// Returns a reference to the given port.  Responsibility is with the caller to ensure port index is valid; if not, 
	// behaviour is undefined and probably very bad
	CMPINLINE DataPort &					GetPort(DataPorts::PortIndex port_index)	{ return m_dataports[port_index]; }

	// Indicates whether the object is integrated into a data environment
	CMPINLINE bool							HasDataEnvironment(void) const				{ return (m_data_environment != NULL); }

	// Method invoked when this object receives data through one of its public input ports
	virtual void							DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) = 0;


	// Allow our parent data environment limited friend access to methods for creating/breaking connections
	friend void iDataObjectEnvironment::RegisterDataEnabledObject(DataEnabledObject *object);
	friend void iDataObjectEnvironment::UnregisterDataEnabledObject(DataEnabledObject *object);
	friend Result iDataObjectEnvironment::ConnectPorts(DataPorts::PortID, DataPorts::PortID);
	friend Result iDataObjectEnvironment::DisconnectPorts(DataPorts::PortID, DataPorts::PortID);

protected:

	// Register a new port for this object.  Returns the index of the port within this object
	// All ports should be registered on object construction, before the object is added
	// to an environment or system.  Changes to port layout after this point will not be 
	// communicated to the parent environment
	DataPorts::PortIndex					RegisterNewPort(DataPorts::PortType port_type);

	// Form a connection to the specified port.  This method should only be called by the parent data environment 
	// once all validations are complete on both sides.  In order to request a new connection, use ConnectPort() instead
	void									MakeConnection(DataPorts::PortIndex port_index, DataPorts::PortID target_port_id);

	// Break the connection from the specifid port.  This method should only be called by the parent data 
	// environment once all validations are complete on both sides.  In order to request a disconnection, use DisconnectPort() instead
	void									BreakConnection(DataPorts::PortIndex port_index);

	// Assigns a unique port ID to the given port in this object; called by the data environment when registering objects
	void									AssignUniquePortID(DataPorts::PortIndex port_index, DataPorts::PortID port_id);

	// Assigns the object to the given data environment; called by the environment when registering objects
	void									AssignToDataEnvironment(iDataObjectEnvironment *env);

	// Removes the object reference to its data environment; called by the environment when unregistering objects
	void									RemoveFromDataEnvironment(void);

	// Removes the unique port ID that was previously assigned to a port
	void									RemoveUniquePortID(DataPorts::PortIndex port_index);
	
	// Send data out of a port
	void									SendData(DataPorts::PortIndex port_index, DataPorts::DataType data);

	// Clears all data from the port data collection, without raising any disconnect events or informing other parties.  Should only
	// be used where the data is not valid, e.g. where the object has been cloned from another and we need to remove the copy-constructed data
	void									ClearAllDataPortDataSilently(void);



protected:

	// Set of all data ports owned by this object
	std::vector<DataPort>					m_dataports;
	
	// The number of ports owned by the environment, for efficiency
	std::vector<DataPort>::size_type		m_dataportcount;

	// Reference to the parent environment that owns this object and other networked objects
	iDataObjectEnvironment * 				m_data_environment;


};
