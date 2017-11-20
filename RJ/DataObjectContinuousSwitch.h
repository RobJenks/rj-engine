#pragma once

#include "DynamicTerrain.h"
class ArticulatedModel;


DYNAMIC_TERRAIN_CLASS(DataObjectContinuousSwitch)
//{
public:

	// Default constructor
	DataObjectContinuousSwitch(void);

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void										InitialiseDynamicTerrain(void);

	// Initialise the data ports required for this object
	void										InitialiseDataPorts(void);

	// Set the current rotation of the switch about its constraint.  Will clamp the rotation within the bounds 
	// of the switch constraint
	void										SetSwitchRotation(float rotation);

	// Adjust the switch by the specified delta
	void										AdjustSwitchRotation(float rotation_delta);

	// Set the current value of the switch, rotating the switch component accordingly.  Will clamp the value
	// within the acceptable value range of the switch
	void										SetSwitchValue(float value);

	// Method invoked when this object receives data through one of its public input ports; switches will not receive any data
	CMPINLINE void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) { }

	// Returns the index of the switch output port
	CMPINLINE DataPorts::PortIndex				OutputPort(void) const { return PORT_SEND; }

	
protected:

	// Switch model and associated data
	std::string									m_switch_component_tag;
	int											m_switch_component;
	std::string									m_switch_constraint_tag;
	int											m_switch_constraint;
	float										m_constraint_min, m_constraint_max;
	float										m_value_min, m_value_max;
	float										m_last_value;
	float										m_value_delta_threshold;

	// Flag which indicates whether the switch is currently being interacted with
	FrameFlag									m_interaction_in_progress;

	// Maintain port indices for convenience
	DataPorts::PortIndex						PORT_SEND;

	// Set a property of this dynamic terrain object.  Called once the base class has registered the property.  Should 
	// be overridden by any subclasses that wish to use properties
	virtual void								SetDynamicTerrainProperty(const std::string & key, const std::string & value);

	// Method invoked when this switch is used by an entity
	virtual bool								OnUsed(iObject *user, DynamicTerrainInteraction && interaction);

	// Set the articulated model used for this switch component
	void										SetArticulatedSwitchModel(const std::string & model_code);

	// Store required tags relating to the switch model
	void										SetSwitchConstraintTag(const std::string & tag);
	void										SetSwitchComponentTag(const std::string & tag);

	// Refresh the switch model after relevant data has been loaded
	void										RefreshSwitchData(void);

	// Set the value range of this continuous switch
	void										SetValueRangeMin(float value_range_min);
	void										SetValueRangeMax(float value_range_max);

	// Set the value delta threshold.  A value change with magnitude below this threshold will not be 
	// registerd and output by the switch
	void										SetValueDeltaThreshold(float threshold);

	// Set the constraint movement range for this continuous switch
	void										SetConstraintMin(float constraint_min_rotation);
	void										SetConstraintMax(float constraint_max_rotation);

	// Determines the switch rotation required in order to yield the given value
	float										SwitchRotationForValue(float value) const;

	// Determines the value corresponding to a rotation of the switch to the specified angle
	float										SwitchValueForRotation(float rotation) const;
};

