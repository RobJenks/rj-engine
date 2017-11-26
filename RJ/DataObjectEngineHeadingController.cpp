#include "iSpaceObjectEnvironment.h"
#include "DataObjectEngineHeadingController.h"

// Default constructor
DataObjectEngineHeadingController::DataObjectEngineHeadingController(void)
{
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectEngineHeadingController::InitialiseDynamicTerrain(void)
{
	// Initialise our data object superclass first
	DataObjectEngineHeadingController::SUPERCLASS_T::InitialiseDynamicTerrain();

	// Assign input ports to each heading controller function
	assert(GetPortCount() == 2);
	Ports.PORT_TARGET_YAW_PC_INPUT = InputPort(0U);
	Ports.PORT_TARGET_PITCH_PC_INPUT = InputPort(1U);
}

// Set a property of this dynamic terrain object
void DataObjectEngineHeadingController::SetDynamicTerrainProperty(const std::string & key, const std::string & value)
{

}

// Method invoked when this object receives data through one of its public input ports
// Component will respond by attempting to set the thrust levels of its associated engine hardpoint to the specified value
void DataObjectEngineHeadingController::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	if (port_index == Ports.PORT_TARGET_YAW_PC_INPUT)
	{
		SetTargetYawPc(data);
	}
	else if (port_index == Ports.PORT_TARGET_PITCH_PC_INPUT)
	{
		SetTargetPitchPc(data);
	}
}

// Attempt to set the ship yaw level to the specified value (capped to the range [-1 +1])
void DataObjectEngineHeadingController::SetTargetYawPc(DataPorts::DataType data)
{
	// Get a reference to the parent ship
	iSpaceObjectEnvironment *parent = GetParentEnvironment();
	if (!parent) return;

	// Constrain the input to a valid range
	float value = clamp(data.FloatValue(), -1.0f, 1.0f);

	// Pass input to the parent ship.  TODO: Should pass to engines in future
	parent->YawShip(value, false);
	Game::Log << LOG_DEBUG << "Setting target yaw to " << value << "\n";
}

// Attempt to set the ship pitch level to the specified value (capped to the range [-1 +1])
void DataObjectEngineHeadingController::SetTargetPitchPc(DataPorts::DataType data)
{
	// Get a reference to the parent ship
	iSpaceObjectEnvironment *parent = GetParentEnvironment();
	if (!parent) return;

	// Constrain the input to a valid range
	float value = clamp(data.FloatValue(), -1.0f, 1.0f);

	// Pass input to the parent ship.  TODO: Should pass to engines in future
	parent->PitchShip(value, false);
	Game::Log << LOG_DEBUG << "Setting target pitch to " << value << "\n";
}






