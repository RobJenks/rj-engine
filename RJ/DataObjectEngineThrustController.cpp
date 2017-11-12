#include "DataObjectEngineThrustController.h"

// Properties applicable to this object
const std::string DataObjectEngineThrustController::PROPERTY_ENGINE_HARDPOINT = "Engine";


// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectEngineThrustController::InitialiseDynamicTerrain(void)
{
	// Initialise our data object superclass first
	DataObjectEngineThrustController::SUPERCLASS_T::InitialiseDynamicTerrain();

	// Assign input ports to each engine controller function
	assert(GetPortCount() == 2);
	PORT_ABSOLUTE_THRUST_INPUT = InputPort(0U);
	PORT_PERCENTAGE_THRUST_INPUT = InputPort(1U);
}

// Set a property of this dynamic terrain object
void DataObjectEngineThrustController::SetDynamicTerrainProperty(const std::string & key, const std::string & value)
{
	if (key == DataObjectEngineThrustController::PROPERTY_ENGINE_HARDPOINT)
	{
		AssignControllerToEngine(value);
	}
}

// Assign this controller to an engine hardpoint, based upon its string code
void DataObjectEngineThrustController::AssignControllerToEngine(const std::string & hardpoint)
{
	// We can only be assigned to an engine if we have a parent environment
	iSpaceObjectEnvironment *environment = this->GetParentEnvironment();
	if (!environment) return;

	*** NEED A WAY TO LINK THIS TO A SPECIFIC HP IN THE ENVIRONMENT (WHEN EITHER PART OF A TILE, OR NOT) ***
	*** IF NOT FROM TILE, CAN SIMPLY USE STRING CODE (?) ***
	*** IF FROM A TILE, NEED TO FIGURE OUT WHAT THE HP CODE WILL BECOME AND ASSIGN IT, OR DETERMINE ANOTHER WAY ***
	
}

// Method invoked when this object receives data through one of its public input ports
// Component will respond by attempting to set the thrust levels of its associated engine hardpoint to the specified value
void DataObjectEngineThrustController::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	//*** DO THIS.  NEED TO ADD A WAY OF ASSOCIATING THESE COMPONENTS TO AN ENGINE HARDPOINT.  THEN CAN SEND A SIGNAL TO THEM IN THIS METHOD ***
}