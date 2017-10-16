#include "DataEnabledLoggingObject.h"
#include "ComplexShip.h"

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

TestResult DataPortTests::BasicTerrainEnvironmentInitialisationTests()
{
	TestResult result = NewResult();

	// Create two data-enabled objects
	DataEnabledLoggingObject *obj0 = DataEnabledLoggingObject::Create(NULL);
	DataEnabledLoggingObject *obj1 = DataEnabledLoggingObject::Create(NULL);
	result.Assert(obj0 != NULL && obj1 != NULL, ERR("Instantiation of test data objects failed"));

	// Create a new data environment and add the objects
	ComplexShip *env = ComplexShip::Create("null_environment");
	result.Assert(env != NULL, ERR("Failed to instantiate data environment"));
	env->AddTerrainObject(obj0);
	env->AddTerrainObject(obj1);
	result.AssertEqual(env->TerrainObjects.size(), (size_t)2U, ERR("Terrain objects were not added to environment correctly"));
	
	// Make sure the object ports were registered correctly
	result.AssertEqual(env->GetDataPorts().size(), (size_t)4U, ERR("Not all data ports registered correctly"));
	result.AssertEqual(env->GetActiveDataPortCount(), env->GetDataPorts().size(), ERR("Not all data ports are correctly flagged as active"));
	DataPorts::PortID id00 = env->GetPortID(obj0, (size_t)0U);
	DataPorts::PortID id01 = env->GetPortID(obj0, (size_t)1U);
	DataPorts::PortID id10 = env->GetPortID(obj1, (size_t)0U);
	DataPorts::PortID id11 = env->GetPortID(obj1, (size_t)1U);

	result.Assert(id00 != DataPorts::NO_PORT_ID, ERR("Data port 0/0 not registered correctly"));
	result.Assert(id01 != DataPorts::NO_PORT_ID, ERR("Data port 0/1 not registered correctly"));
	result.Assert(id10 != DataPorts::NO_PORT_ID, ERR("Data port 1/0 not registered correctly"));
	result.Assert(id11 != DataPorts::NO_PORT_ID, ERR("Data port 1/1 not registered correctly"));

	// Check details of each port ID in turn
	DataPorts::PortID id[4] = { id00, id01, id10, id11 };
	for (int i = 0; i < 4; ++i)
	{
		result.AssertTrue(env->IsValidPortID(id[i]), ERR("Data port ID is not valid in environment"));
		if (env->IsValidPortID(id[i]))
		{
			const DataPortReference & port = env->GetPortDetails(id[i]);
			result.AssertEqual(port.IsActive, true, ERR("Port was registered but is inactive"));
			result.AssertEqual(port.ID, id[i], ERR("Port was registered but has mismatched ID"));
			result.AssertEqual(port.DataObject, (i < 2 ? (DataEnabledObject*)obj0 : (DataEnabledObject*)obj1), ERR("Port is not registered to correct object"));
			result.AssertEqual(port.ObjectPortIndex, (size_t)(i % 2), ERR("Port was registered but does not have correct object index"));
		}
	}
	
	return result;
}


TestResult DataPortTests::BasicConnectonTests()
{
	Result res;
	TestResult result = NewResult();

	// Create two data-enabled objects
	DataEnabledLoggingObject *obj0 = DataEnabledLoggingObject::Create(NULL);
	DataEnabledLoggingObject *obj1 = DataEnabledLoggingObject::Create(NULL);
	DataEnabledLoggingObject *obj2 = DataEnabledLoggingObject::Create(NULL);
	result.Assert(obj0 != NULL && obj1 != NULL && obj2 != NULL, ERR("Instantiation of test data objects failed"));

	// Create a new data environment and add the objects
	ComplexShip *env = ComplexShip::Create("null_environment");
	result.Assert(env != NULL, ERR("Failed to instantiate data environment"));
	env->AddTerrainObject(obj0);
	env->AddTerrainObject(obj1);
	result.AssertEqual(env->TerrainObjects.size(), (size_t)2U, ERR("Terrain objects were not added to environment correctly"));

	// Attempt to make an invalid connection from output -> output and input -> input ports; there should be no connections made
	res = obj0->ConnectPort(obj0->OutputPort(), obj1, obj1->OutputPort());
	result.Assert(res != ErrorCodes::NoError, ERR("Data port connection attempt from (output -> output) was not rejected as expected"));
	res = obj1->ConnectPort(obj1->InputPort(), obj0, obj0->InputPort());
	result.Assert(res != ErrorCodes::NoError, ERR("Data port connection attempt from (input -> input) was not rejected as expected"));
	result.AssertEqual(obj0->GetPort(obj0->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Input port 0 erroneously connected in invalid connection"));
	result.AssertEqual(obj1->GetPort(obj1->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Input port 1 erroneously connected in invalid connection"));
	result.AssertEqual(obj0->GetPort(obj0->OutputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Output port 0 erroneously connected in invalid connection"));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Output port 1 erroneously connected in invalid connection"));

	// Connect obj0 -> obj1
	res = obj0->ConnectPort(obj0->OutputPort(), obj1, obj1->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR(concat("Data port connection attempt failed with error code ")(res).str().c_str()));
	result.AssertEqual(obj0->GetPort(obj0->OutputPort()).GetConnectedPort(), obj1->GetPort(obj1->InputPort()).GetPortID(), ERR("Output port connection ID is not correct"));
	result.AssertEqual(obj1->GetPort(obj1->InputPort()).GetConnectedPort(), obj0->GetPort(obj0->OutputPort()).GetPortID(), ERR("Input port connection ID is not correct"));
	
	// Make sure the other ports remain unconnected
	result.AssertEqual(obj0->GetPort(obj0->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Input port of sending object is erroneously connected"));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Output port of receiving object is erroneously connected"));

	// Attempt to make another connection from obj0 output; should be disallowed since obj0 output is already connected
	res = obj0->ConnectPort(obj0->OutputPort(), obj2, obj2->OutputPort());
	result.Assert(res != ErrorCodes::NoError, ERR("Data port connection attempt when already connected was not rejected as expected"));
	result.AssertEqual(obj0->GetPort(obj0->OutputPort()).GetConnectedPort(), obj1->GetPort(obj1->InputPort()).GetPortID(), ERR("Output port was reassigned and is no longer connected to correct target"));
	result.AssertEqual(obj2->GetPort(obj2->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Reassignment of active connection was erroneously performed"));

	// Make the reciprocal connection from obj1 -> obj0
	res = obj1->ConnectPort(obj1->OutputPort(), obj0, obj0->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR(concat("Data port connection attempt failed with error code ")(res).str().c_str()));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), obj0->GetPort(obj0->InputPort()).GetPortID(), ERR("Output port connection ID is not correct"));
	result.AssertEqual(obj0->GetPort(obj0->InputPort()).GetConnectedPort(), obj1->GetPort(obj1->OutputPort()).GetPortID(), ERR("Input port connection ID is not correct"));
	
	// Break the connection from obj1 -> obj0 and verify all connections are as expected
	res = obj1->DisconnectPort(obj1->OutputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR(concat("Data port disconnection attempt from output side failed with error code ")(res).str().c_str()));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Output port was not correctly disconnected from target"));
	result.AssertEqual(obj0->GetPort(obj0->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Input port was not correctly disconnected from source"));
	result.AssertEqual(obj0->GetPort(obj0->OutputPort()).GetConnectedPort(), obj1->GetPort(obj1->InputPort()).GetPortID(), ERR("Unrelated object output port was incorrectly disconnected when breaking other connection"));
	result.AssertEqual(obj1->GetPort(obj1->InputPort()).GetConnectedPort(), obj0->GetPort(obj0->OutputPort()).GetPortID(), ERR("Unrelated object input port was incorrectly disconnected when breaking other connection"));

	// Break the connection from obj0 -> obj1, this time via the target port, and verify that there are no longer any connections active at all
	res = obj1->DisconnectPort(obj1->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR(concat("Data port disconnection attempt from input side failed with error code ")(res).str().c_str()));
	result.AssertEqual(obj0->GetPort(obj0->OutputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Output port was not correctly disconnected when breaking connection")); 
	result.AssertEqual(obj1->GetPort(obj1->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Input port was not correctly disconnected when breaking connection"));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Unrelated output port is no longer disconnected following break of other connection"));
	result.AssertEqual(obj0->GetPort(obj0->InputPort()).GetConnectedPort(), DataPorts::NO_PORT_ID, ERR("Unrelated input port is no longer disconnected following break of other connection"));

	return result;
}




