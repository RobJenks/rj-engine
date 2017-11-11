#include "DataObjectRelay.h"
#include "DataObjectOutput.h"
#include "DataObjectRegister.h"
#include "ComplexShip.h"

#include "DataPortTests.h"


TestResult DataPortTests::BasicInitialisationTests()
{
	TestResult result = NewResult();

	// Create two data-enabled objects
	DataObjectRelay *obj0 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	DataObjectRelay *obj1 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	result.Assert(obj0 != NULL && obj1 != NULL, ERR("Instantiation of test data objects failed"));

	// Verify that required ports exist
	result.AssertEqual(obj0->GetPortCount(), (DataPorts::PortIndex)2U, ERR("Data object was not correctly initialised with two ports"));
	result.AssertEqual(obj1->GetPortCount(), (DataPorts::PortIndex)2U, ERR("Data object was not correctly initialised with two ports"));

	// Verify the contents of each port
	DataObjectRelay *obj[2] = { obj0, obj1 };
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
	DataObjectRelay *obj0 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	DataObjectRelay *obj1 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
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
	DataObjectRelay *obj0 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	DataObjectRelay *obj1 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	DataObjectRelay *obj2 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	result.Assert(obj0 != NULL && obj1 != NULL && obj2 != NULL, ERR("Instantiation of test data objects failed"));

	// Create a new data environment and add the objects
	ComplexShip *env = ComplexShip::Create("null_environment");
	result.Assert(env != NULL, ERR("Failed to instantiate data environment"));
	env->AddTerrainObject(obj0);
	env->AddTerrainObject(obj1);
	env->AddTerrainObject(obj2);
	result.AssertEqual(env->TerrainObjects.size(), (size_t)3U, ERR("Terrain objects were not added to environment correctly"));

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


TestResult DataPortTests::DataPortEnvironmentInteractionTests()
{
	Result res;
	TestResult result = NewResult();

	// Create two data-enabled objects
	DataObjectRelay *obj0 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	DataObjectRelay *obj1 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	DataObjectRelay *obj2 = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
	result.Assert(obj0 != NULL && obj1 != NULL && obj2 != NULL, ERR("Instantiation of test data objects failed"));

	// Create a new data environment and add the objects
	ComplexShip *env = ComplexShip::Create("null_environment");
	result.Assert(env != NULL, ERR("Failed to instantiate data environment"));
	env->AddTerrainObject(obj0);
	env->AddTerrainObject(obj1);
	env->AddTerrainObject(obj2);
	result.AssertEqual(env->TerrainObjects.size(), (size_t)3U, ERR("Terrain objects were not added to environment correctly"));

	// Create connections from (obj0 -> obj1) and (obj1 -> obj2); one from output and one from input side as an added test
	res = obj0->ConnectPort(obj0->OutputPort(), obj1, obj1->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR(concat("Data port connection attempt (0 -> 1) failed with error code ")(res).str().c_str()));
	res = obj2->ConnectPort(obj2->InputPort(), obj1, obj1->OutputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR(concat("Data port connection attempt (2 <- 1) failed with error code ")(res).str().c_str()));

	// Make sure both connections were made
	result.AssertEqual(obj0->GetPort(obj0->OutputPort()).GetConnectedPort(), obj1->GetPort(obj1->InputPort()).GetPortID(), ERR("Connection (0 -> 1) not present on output side"));
	result.AssertEqual(obj1->GetPort(obj1->InputPort()).GetConnectedPort(), obj0->GetPort(obj0->OutputPort()).GetPortID(), ERR("Connection (0 -> 1) not present on input side"));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), obj2->GetPort(obj2->InputPort()).GetPortID(), ERR("Connection (1 -> 2) not present on output side"));
	result.AssertEqual(obj2->GetPort(obj2->InputPort()).GetConnectedPort(), obj1->GetPort(obj1->OutputPort()).GetPortID(), ERR("Connection (1 -> 2) not present on input side"));

	// Now remove object 0 from the environment and make sure that the connection (0 -> 1) is correctly broken
	DataPorts::PortID obj1_input = obj1->GetPort(obj1->InputPort()).GetPortID();
	DataPorts::PortID obj1_output = obj1->GetPort(obj1->OutputPort()).GetPortID();
	env->RemoveTerrainObject(obj0);
	result.AssertEqual(env->TerrainObjects.size(), (size_t)2U, ERR("Terrain object 0 not removed from environment correctly"));
	result.AssertEqual(env->GetActiveDataPortCount(), (size_t)4U, ERR("Environment active port count is incorrect following object 0 removal"));

	result.Assert(obj1->HasDataEnvironment() == true, ERR("Object 1 was incorrectly removed from environment during removal of connected object "));
	result.Assert(obj1->GetPort(obj1->InputPort()).IsConnected() == false, ERR("Object 1 input port not correctly disconnected upon removal of connected object from environment"));
	result.AssertEqual(obj1->GetPort(obj1->InputPort()).GetPortID(), obj1_input, ERR("Object 1 input port ID has erroneously changed upon removal of connected object from environment"));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetPortID(), obj1_output, ERR("Object 1 output port ID has erroneously changed upon removal of connected object from environment"));

	// Make sure the connection (1 -> 2) was unaffected by the removal of object 0 and the subsequent breaking of connection (0 -> 1)
	result.Assert(obj1->GetPort(obj1->OutputPort()).IsConnected() == true, ERR("Object 1 output port was incorrectly disconnected during break of (0 -> 1) connection"));
	result.Assert(obj2->GetPort(obj2->InputPort()).IsConnected() == true, ERR("Object 2 input port was incorrectly disconnected during break of (0 -> 1) connection"));
	result.Assert(obj2->GetPort(obj2->OutputPort()).IsConnected() == false, ERR("Object 2 output port has unexpected connection following break of (0 -> 1) connection"));
	result.AssertEqual(obj1->GetPort(obj1->OutputPort()).GetConnectedPort(), obj2->GetPort(obj2->InputPort()).GetPortID(), ERR("Connection (1 -> 2) incorrectly broken on output side following break of connection (0 -> 1)"));
	result.AssertEqual(obj2->GetPort(obj2->InputPort()).GetConnectedPort(), obj1->GetPort(obj1->OutputPort()).GetPortID(), ERR("Connection (1 -> 2) incorrectly broken on input side following break of connection (0 -> 1)"));

	return result;
}


TestResult DataPortTests::BasicDataTransmissionTests()
{
	Result res;
	TestResult result = NewResult();

	// Create a new data environment
	ComplexShip *env = ComplexShip::Create("null_environment");
	result.Assert(env != NULL, ERR("Failed to instantiate data environment"));

	// Create the data-enabled objects
	static const unsigned int CHAIN_LENGTH = 8U;
	static_assert(CHAIN_LENGTH > 6U);										// Required for some tests in the sequence below

	DataObjectRegister4 *registers = (DataObjectRegister4*)DynamicTerrain::Create("DataObjectRegister4");
	DataObjectOutput3 *single_output = (DataObjectOutput3*)DynamicTerrain::Create("DataObjectOutput3");
	DataObjectOutput1 *chain_start = (DataObjectOutput1*)DynamicTerrain::Create("DataObjectOutput1");
	result.Assert(registers != NULL && single_output != NULL && chain_start != NULL, ERR("Instantiation of test data objects failed"));

	DataObjectRelay *relay_chain[8];
	env->AddTerrainObject(registers);
	env->AddTerrainObject(single_output);
	env->AddTerrainObject(chain_start);

	// Connnect the relay array in a linear chain
	for (int i = 0; i < CHAIN_LENGTH; ++i)
	{
		relay_chain[i] = (DataObjectRelay*)DynamicTerrain::Create("DataObjectRelay");
		env->AddTerrainObject(relay_chain[i]);
		if (i != 0)
		{
			res = relay_chain[i]->ConnectPort(relay_chain[i]->InputPort(), relay_chain[i - 1U], relay_chain[i - 1U]->OutputPort());
			result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to connect elements in relay chain"));
		}
	}

	// Make sure all objects and ports were added to the environment as expected
	result.AssertEqual(env->TerrainObjects.size(), (size_t)(1U + 1U + 1U + CHAIN_LENGTH), ERR("Terrain objects were not added to environment correctly"));
	result.AssertEqual(env->GetActiveDataPortCount(), (size_t)(registers->GetPortCount() + 3U + 1U + (CHAIN_LENGTH * 2U)), ERR("Object ports were not registered with environment correctly"));

	// Add an output to the start of the relay chain, then connect both the single output and the end of the relay 
	// chain to inputs in the register object.  Final structure looks like:
	//		Output ------------------------> Registers[SINGLE_INPUT]
	//		Output -> {Relay, ..., Relay} -> Registers[CHAIN_INPUT]
	
	const unsigned int SINGLE_OUTPUT = 1U;						// Value (not port) index for the output of the (output -> registers) connection
	const unsigned int SINGLE_INPUT = 0U;						// Value (not port) index for the register input accepting (output -> registers) input
	const unsigned int CHAIN_OUTPUT = 0U;						// Value (not port) index for the output of the (start_output -> {chain}) connection
	const unsigned int CHAIN_INPUT = 2U;						// Value (not port) index for the register input accepting (relay -> registers) input
	res = chain_start->ConnectPort(chain_start->OutputPort(CHAIN_OUTPUT), relay_chain[0], relay_chain[0]->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to connect single output to start of relay chain"));
	res = registers->ConnectPort(registers->InputPort(SINGLE_INPUT), single_output, single_output->OutputPort(SINGLE_OUTPUT));
	result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to connect single output to register input"));
	res = registers->ConnectPort(registers->InputPort(CHAIN_INPUT), relay_chain[CHAIN_LENGTH - 1U], relay_chain[CHAIN_LENGTH - 1U]->OutputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to connect relay chain to register input"));

	// Send a value through the single relay and make sure it is transmmitted
	result.AssertEqual(registers->GetValue(SINGLE_INPUT), DataPorts::DataType::Zero(), ERR("Single relay register input not correctly initialised on creation"));
	single_output->SendOutput(SINGLE_OUTPUT - 1U, 6.0f);		// Disconnected output
	single_output->SendOutput(SINGLE_OUTPUT, 12.0f);			// Connected output - this value should be transmitted
	single_output->SendOutput(SINGLE_OUTPUT + 1U, 24.0f);		// Disconnected output
	single_output->SendOutput(SINGLE_OUTPUT + 2U, 48.0f);		// Non-existent output
	result.AssertEqual(registers->GetValue(SINGLE_INPUT).FloatValue, 12.0f, ERR("Correct value not tranmitted to and stored in data register"));

	// Send a value through the relay chain and make sure is transmitted all the way to the destination register
	result.AssertEqual(registers->GetValue(CHAIN_INPUT), DataPorts::DataType::Zero(), ERR("Chain relay register input not correctly initialised on creation"));
	chain_start->SendOutput(CHAIN_OUTPUT, 1024.0f);
	result.AssertEqual(registers->GetValue(CHAIN_INPUT).FloatValue, 1024.0f, ERR("Value not transmitted correctly through relay chain to destination register"));

	// Break two links in the relay chain and make sure that data is no longer transmitted
	unsigned int break_start = (CHAIN_LENGTH / 2);
	res = relay_chain[break_start + 1U]->DisconnectPort(relay_chain[break_start + 1U]->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to break relay chain at input point"));
	res = relay_chain[break_start + 1U]->DisconnectPort(relay_chain[break_start + 1U]->OutputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to break relay chain at output point"));
	chain_start->SendOutput(CHAIN_OUTPUT, 2048.0f);
	result.AssertEqual(registers->GetValue(CHAIN_INPUT).FloatValue, 1024.0f, ERR("Data incorrectly transmitted across disconnected relay chain"));

	// Reconnect the broken ends of the relay, skipping relay[break_start + 1U], and make sure it can once again transmit data
	res = relay_chain[break_start + 0U]->ConnectPort(relay_chain[break_start + 0U]->OutputPort(), relay_chain[break_start + 2U], relay_chain[break_start + 2U]->InputPort());
	result.AssertEqual(res, ErrorCodes::NoError, ERR("Failed to reconnect relay chain at (break+0 -> break+2)"));
	chain_start->SendOutput(CHAIN_OUTPUT, 4096.0f);
	result.AssertEqual(registers->GetValue(CHAIN_INPUT).FloatValue, 4096.0f, ERR("Value not transmitted correctly through re-connected relay chain to destination register"));

	// Verify final state of unused data registers was not affected by any of the above activity
	Game::Log << LOG_INFO << "Data transmission test results\n";
	for (unsigned int i = 0U; i < 4U; ++i)
	{
		Game::Log << LOG_INFO << "   Final value of register " << i << ": " << registers->GetValue(i).str() << " (" << registers->GetValue(i).FloatValue << ")\n";
		if (i != SINGLE_INPUT && i != CHAIN_INPUT)
		{
			result.AssertEqual(registers->GetValue(i), DataPorts::DataType::Zero(), ERR("Unused data register was impacted by data transfer to other registers"));
		}
	}

	return result;
}

