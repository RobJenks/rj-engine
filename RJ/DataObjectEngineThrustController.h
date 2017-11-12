#pragma once

#include "DynamicTerrain.h"
#include "DataObjectInput.h"


/*
	Derived from basic input component.  Will respond by attempting to set the thrust levels of its associated engine 
	hardpoint to the specified value.  Accepts the following inputs:
		PORT_ABSOLUTE_THRUST_INPUT:			Float input specifying the absolute thrust value the engine should attempt to reach
		PORT_PERCENTAGE_THRUST_INPUT:		Float input specifying the percentage of total potential thrust the engine should try to attain
*/
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectEngineThrustController, DataObjectInput2)
//{
public:

	// Superclass of this data component
	typedef DataObjectInput2			SUPERCLASS_T;

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void								InitialiseDynamicTerrain(void);

	// Set a property of this dynamic terrain object
	void								SetDynamicTerrainProperty(const std::string & key, const std::string & value);

	// Method invoked when this object receives data through one of its public input ports
	// Component will respond by attempting to set the thrust levels of its associated engine hardpoint to the specified value
	void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port);


private:

	// Store port indices for convenience
	DataPorts::PortIndex				PORT_ABSOLUTE_THRUST_INPUT = DataPorts::NO_PORT_INDEX;
	DataPorts::PortIndex				PORT_PERCENTAGE_THRUST_INPUT = DataPorts::NO_PORT_INDEX;

	// Properties applicable to this object
	static const std::string			PROPERTY_ENGINE_HARDPOINT;

	// Assign this controller to an engine hardpoint, based upon its string code
	void								AssignControllerToEngine(const std::string & hardpoint);

};