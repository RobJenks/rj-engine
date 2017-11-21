#pragma once

#include "DataObjectInput.h"

/*
	Derived from basic input component.  Will respond by attempting to set ship engine thrust levels to achieve the target
	heading changes.  Ship will maintain the target heading until new data is received

	PORT_TARGET_YAW_PC_INPUT:			Float input in the range [-1 +1] specifying the target percentage of total yaw capability the ship should try to achieve
	PORT_TARGET_PITCH_PC_INPUT:			Float input in the range [-1 +1] specifying the target percentage of total pitch capability the ship should try to achieve
*/
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectEngineHeadingController, DataObjectInput2)
//{
public:

	// Superclass of this data component
	typedef DataObjectInput2			SUPERCLASS_T;

	// Default constructor
	DataObjectEngineHeadingController(void);

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void								InitialiseDynamicTerrain(void);

	// Set a property of this dynamic terrain object
	void								SetDynamicTerrainProperty(const std::string & key, const std::string & value);

	// Method invoked when this object receives data through one of its public input ports
	// Component will respond by attempting to set the thrust levels of its associated engine hardpoint to the specified value
	void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port);


	// Ports exposed by this controller
	struct
	{
		DataPorts::PortIndex			PORT_TARGET_YAW_PC_INPUT = DataPorts::NO_PORT_INDEX;
		DataPorts::PortIndex			PORT_TARGET_PITCH_PC_INPUT = DataPorts::NO_PORT_INDEX;

		DataPorts::PortIndex			TargetYawPercentageInput(void) const { return PORT_TARGET_YAW_PC_INPUT; }
		DataPorts::PortIndex			TargetPitchPercentageInput(void) const { return PORT_TARGET_PITCH_PC_INPUT; }

	} Ports;


protected:

	// Attempt to set the ship yaw level to the specified value (capped to the range [-1 +1])
	void								SetTargetYawPc(DataPorts::DataType data);

	// Attempt to set the ship pitch level to the specified value (capped to the range [-1 +1])
	void								SetTargetPitchPc(DataPorts::DataType data);



};






