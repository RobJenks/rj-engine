#pragma once

#include "ErrorCodes.h"
#include "DataPorts.h"
class DataEnabledObject;

class iDataObjectEnvironment
{
public:

	// Structure holding a reference to a specific port within the environment
	struct DataPortReference
	{
		bool						IsActive;
		DataPorts::PortID			ID;
		DataEnabledObject *			DataObject;
		DataPorts::PortIndex		ObjectPortIndex;
	};

	// Default constructor
	iDataObjectEnvironment(void);

	// Registers a new data-enabled object with this environment
	void									RegisterDataEnabledObject(DataEnabledObject *object);

	// Removes a data-enabled object from this environment
	void									UnregisterDataEnabledObject(DataEnabledObject *object);

	// Returns the ID of a port matching the given criteria, or NO_PORT_ID if no such port exists
	DataPorts::PortID						GetPortID(const DataEnabledObject *object, DataPorts::PortIndex port_index) const;

	// Returns details for the given port ID, or the null port reference at NO_PORT_ID if the given ID is not valid
	DataPortReference &						GetPortDetails(DataPorts::PortID port_id);

	// Indicates whether the specified port ID is valid and active
	bool									IsValidPortID(DataPorts::PortID port_id) const;
	
	// Checks the given port ID and verifies that it is correct and active
	Result									VerifyPort(DataPorts::PortID port_id) const;

	// Connects the two specified ports together, or returns an error code if the connection is not valid or possible
	Result									ConnectPorts(DataPorts::PortID port_id_0, DataPorts::PortID port_id_1);

	// Disconnect the two specified ports, or returns an error code if the connection cannot be broken for some reason
	Result									DisconnectPorts(DataPorts::PortID port_id_0, DataPorts::PortID port_id_1);

	// Transmit data between the two given ports
	void									TransmitData(DataPorts::PortID source_port, DataPorts::PortID target_port, DataPorts::DataType data);

	// Default destructor
	~iDataObjectEnvironment(void);

private:

	// Collection of data ports registered in this environment
	std::vector<DataPortReference>	m_data_ports;


};