#include "Collections.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipTile.h"
#include "Hardpoint.h"
#include "Hardpoints.h"
#include "HpEngine.h"
#include "DataObjectEngineThrustController.h"

// Properties applicable to this object
const std::string DataObjectEngineThrustController::PROPERTY_ENGINE_HARDPOINT = "Engine";


// Default constructor
DataObjectEngineThrustController::DataObjectEngineThrustController(void)
	:
	m_engine_hardpoint(NullString), m_is_linked_to_engine(false)
{
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectEngineThrustController::InitialiseDynamicTerrain(void)
{
	// Initialise our data object superclass first
	DataObjectEngineThrustController::SUPERCLASS_T::InitialiseDynamicTerrain();

	// Assign input ports to each engine controller function
	assert(GetPortCount() == 2);
	Ports.PORT_ABSOLUTE_THRUST_INPUT = InputPort(0U);
	Ports.PORT_PERCENTAGE_THRUST_INPUT = InputPort(1U);
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

	// Take different action depending on the context in which we have been added to the environment
	std::string engine_hardpoint = NullString;
	if (this->HasParentTile())
	{
		// Controller was added as part of a tile, so attempt to resolve the hardpoint name within the tile-defined
		// hardpoints first
		ComplexShipTile *parent_tile = environment->FindTileByID(this->GetParentTileID(), D::TileClass::EngineRoom);
		if (parent_tile)
		{
			// We only use the tile-relative code if we can find a matching hardpoint in the tile collection
			std::string tile_relative_name = parent_tile->DetermineTileHardpointCode(hardpoint);
			if (Collections::Contains(parent_tile->GetHardpointReferences(), [&tile_relative_name](const std::string & element) { return (element == tile_relative_name); }))
			{
				// We also make sure that this hardpoint can be located in the environment itself, to be sure
				Hardpoint *hp = environment->GetHardpoints().Get(tile_relative_name);
				if (hp)
				{
					// We will link to this tile-relative hardpoint
					engine_hardpoint = tile_relative_name;
				}
			}
		}
	}

	// Test whether we were able to locate a hardpoint within the scope of our parent tile
	if (engine_hardpoint.empty())
	{
		// We could not, so attempt to match a hardpoint in the environment based on the 'unqualified' name
		Hardpoint *hp = environment->GetHardpoints().Get(hardpoint);
		if (hp)
		{
			engine_hardpoint = hp->Code;
		}
	}

	// Store the final derived hardpoint code as our linked component; whether it could be resolved or is a null string
	SetLinkedHardpointCode(engine_hardpoint);
}

// Stores the string code of the hardpoint that this controller is associated with, or removes any links if the supplied code is a null string
void DataObjectEngineThrustController::SetLinkedHardpointCode(const std::string & engine_hardpoint_code)
{
	m_engine_hardpoint = engine_hardpoint_code;
	m_is_linked_to_engine = (!engine_hardpoint_code.empty());
}

// Method invoked when this object receives data through one of its public input ports
// Component will respond by attempting to set the thrust levels of its associated engine hardpoint to the specified value
void DataObjectEngineThrustController::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	if (port_index == Ports.PORT_ABSOLUTE_THRUST_INPUT)
	{
		SetEngineAbsoluteThrust(data);
	}
	else if (port_index == Ports.PORT_PERCENTAGE_THRUST_INPUT)
	{
		SetEnginePercentageThrust(data);
	}
}

// Attempt to resolve the associated engine hardpoint code to a hardpoint within our environment, or returns NULL if not possible
HpEngine * DataObjectEngineThrustController::GetEngineHardpoint(void)
{
	if (!m_is_linked_to_engine || !m_parent) return NULL;

	Hardpoint *hp = m_parent->GetHardpoints().Get(m_engine_hardpoint);
	if (hp->GetType() != Equip::Class::Engine) return NULL;

	return (HpEngine*)hp;
}

// Attempt to set the absolute thrust level of our associated engine hardpoint to the received data value
void DataObjectEngineThrustController::SetEngineAbsoluteThrust(DataPorts::DataType data)
{
	// Attempt to make a connection to our associated engine hardpoint
	HpEngine *engine = GetEngineHardpoint();
	if (!engine) return;

	// The engine will ensure this data is correctly bounded, so simply forward the data packet to the engine
	engine->SetTargetThrust(data.FloatValue());
}

// Attempt to set the absolute thrust level of our associated engine hardpoint to the received data value
void DataObjectEngineThrustController::SetEnginePercentageThrust(DataPorts::DataType data)
{
	// Attempt to make a connection to our associated engine hardpoint
	HpEngine *engine = GetEngineHardpoint();
	if (!engine) return;

	// The engine will ensure this data is correctly bounded, so simply forward the data packet to the engine
	engine->SetTargetThrustPercentage(data.FloatValue());
}




