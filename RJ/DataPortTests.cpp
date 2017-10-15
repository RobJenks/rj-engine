#include "DataEnabledLoggingObject.h"

#include "DataPortTests.h"


TestResult DataPortTests::BasicInitialisationTests()
{
	TestResult result = NewResult();

	// Create two data-enabled objects
	DataEnabledLoggingObject *obj0 = DataEnabledLoggingObject::Create(NULL);
	DataEnabledLoggingObject *obj1 = DataEnabledLoggingObject::Create(NULL);

	// Verify that required ports exist
	result.AssertEqual(obj0->GetPortCount(), (DataPorts::PortIndex)2U, ERR("Data object was not correctly initialised with two ports"));
	result.AssertEqual(obj1->GetPortCount(), (DataPorts::PortIndex)2U, ERR("Data object was not correctly initialised with two ports"));

	// Verify the contents of each port
	DataEnabledLoggingObject *obj[2] = { obj0, obj1 };
	for (size_t i = 0U; i < 2U; ++i)
	{
		for (DataPorts::PortIndex port_index = 0U; port_index < 2U; ++port_index)
		{
			// Validate basic properties of each port
			const DataPort & port = obj[i]->GetPort(port_index);
			result.AssertEqual(port.GetPortIndex(), port_index, ERR("Data object was not initialised with correct port index"));
			result.AssertEqual(port.GetPortID(), DataPorts::NO_PORT_ID, ERR("Data object port ID was not correctly initialised to NO_PORT_ID"));
			result.AssertEqual(port.GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Data object target port ID was not correctly initialised to NO_PORT_ID"));

			// Each logging object should have two ports; one input [0], one output [1]
			result.AssertEqual(port.GetPortType(), (port_index == 0U ? DataPorts::PortType::InputPort : DataPorts::PortType::OutputPort), ERR("Data object port does not have correct type"));
		}
	}

	// Return the final test result
	return result;
}