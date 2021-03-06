#pragma once

#include "ErrorCodes.h"
#include "DataPorts.h"
#include "DataPortReference.h"
class DataEnabledObject;

class iDataObjectEnvironment
{
public:

	// Default constructor
	iDataObjectEnvironment(void);
	
	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void									InitialiseCopiedObject(iDataObjectEnvironment *source);

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

	// Returns the number of ACTIVE ports in the environment
	CMPINLINE DataPorts::PortIndex			GetActiveDataPortCount(void) const { return m_active_port_count; }

	// Connects the two specified ports together, or returns an error code if the connection is not valid or possible
	Result									ConnectPorts(DataPorts::PortID port_id_0, DataPorts::PortID port_id_1);

	// Disconnect the two specified ports, or returns an error code if the connection cannot be broken for some reason
	Result									DisconnectPorts(DataPorts::PortID port_id_0, DataPorts::PortID port_id_1);

	// Transmit data between the two given ports
	void									TransmitData(DataPorts::PortID source_port, DataPorts::PortID target_port, DataPorts::DataType data);

	// Return a constant reference to the set of registered data ports (including those which are inactive)
	CMPINLINE const std::vector<DataPortReference> & GetDataPorts(void) const { return m_data_ports; }

	// Default destructor
	~iDataObjectEnvironment(void);


private:

	// Collection of data ports registered in this environment
	std::vector<DataPortReference>			m_data_ports;

	// Count of the number of ACTIVE ports in this environment (which will be <= the
	// total number of ports in the collection, due to ports being set inactive when removed)
	DataPorts::PortIndex					m_active_port_count;


	// Return the next free data port ID; either the first inactive entry, or a new entry if all are currently active
	DataPorts::PortID						GetNewDataPortID(void);

	// Store a new data port reference at the given index
	void									StoreNewDataPortReference(DataPorts::PortID port_id, DataEnabledObject *object, DataPorts::PortIndex object_port_index);

	// Deactivate the data port reference with the given ID
	void									DeactivateDataPortReference(DataPorts::PortID port_id);

	// Clear all data object data in the environment, without notifying the relevant objects or raising any associated
	// events.  Clearing the data silently in this way should only be performed in very specific circumstances, for 
	// example if the object has been cloned from another and contains copy-constructed data that is not valid in the 
	// object, and could trigger erroneous disconnections in the source environment if events were allowed to fire
	void									ClearAllDataObjectEnvironmentDataSilently(void);

};