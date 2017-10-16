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
			const iDataObjectEnvironment::DataPortReference & port = env->GetPortDetails(id[i]);
			result.AssertEqual(port.IsActive, true, ERR("Port was registered but is inactive"));
			result.AssertEqual(port.ID, id[4], ERR("Port was registered but has mismatched ID"));
			result.AssertEqual(port.DataObject, (i < 2 ? (DataEnabledObject*)obj0 : (DataEnabledObject*)obj1), ERR("Port is not registered to correct object"));
			result.AssertEqual(port.ObjectPortIndex, (size_t)(i % 2), ERR("Port was registered but does not have correct object index"));
		}
	}
	
	return result;
}