#include "ArticulatedModel.h"
#include "iObject.h"

#include "DataObjectContinuousSwitch.h"

// Default constructor
DataObjectContinuousSwitch::DataObjectContinuousSwitch(void)
	:
	m_switch_component_tag(NullString), m_switch_component(-1), m_switch_constraint_tag(NullString), m_switch_constraint(-1), 
	m_constraint_min(-PI / 4.0f), m_constraint_max(+PI / 4.0f), 
	m_value_min(-1.0f), m_value_max(+1.0f), m_value_delta_threshold(0.01f), 
	PORT_SEND(DataPorts::NO_PORT_INDEX)
{
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
void DataObjectContinuousSwitch::InitialiseDynamicTerrain(void)
{
	// Clear any data that should be set per-instance
	m_last_value = FLT_MIN;		// So that the first change will always be above the delta threshold

	// Initialise the data ports required for this object
	InitialiseDataPorts();
}

// Initialise the data ports required for this object
void DataObjectContinuousSwitch::InitialiseDataPorts(void)
{
	PORT_SEND = RegisterNewPort(DataPorts::PortType::OutputPort);
}

// Set a property of this dynamic terrain object.  Called once the base class has registered the property.  Should 
// be overridden by any subclasses that wish to use properties
void DataObjectContinuousSwitch::SetDynamicTerrainProperty(const std::string & key, const std::string & value)
{
	if (key.empty()) return;
	std::string prop = key; StrLowerC(prop);
	HashVal hash = HashString(prop);

	if (hash == HashedStrings::H_ValueRangeMin)					SetValueRangeMin((float)atof(value.c_str()));
	else if (hash == HashedStrings::H_ValueRangeMax)			SetValueRangeMax((float)atof(value.c_str()));
	else if (hash == HashedStrings::H_ValueDeltaThreshold)		SetValueDeltaThreshold((float)atof(value.c_str()));
	else if (hash == HashedStrings::H_ArticulatedModel)			SetArticulatedSwitchModel(value);
	else if (hash == HashedStrings::H_ModelSwitchComponent)		SetSwitchComponentTag(value);
	else if (hash == HashedStrings::H_ModelSwitchConstraint)	SetSwitchConstraintTag(value);
	else if (hash == HashedStrings::H_SwitchConstraintMin)		SetConstraintMin((float)atof(value.c_str()));
	else if (hash == HashedStrings::H_SwitchConstraintMax)		SetConstraintMax((float)atof(value.c_str()));


}

// Method invoked when this switch is used by an entity.  Continuous switches only accept extended
// interaction types
bool DataObjectContinuousSwitch::OnUsed(iObject *user, DynamicTerrainInteraction && interaction)
{
	AdjustSwitchRotation(0.1f * Game::TimeFactor);
	return true;
}

// Set the articulated model used for this switch component
void DataObjectContinuousSwitch::SetArticulatedSwitchModel(const std::string & model_code)
{
	ArticulatedModel *model = ArticulatedModel::GetModel(model_code);
	if (!model) return;

	SetArticulatedModel( model->Copy() );
	RefreshSwitchData();
}

// Tag of the axis around which the switch component rotates
void DataObjectContinuousSwitch::SetSwitchConstraintTag(const std::string & tag)
{	
	m_switch_constraint_tag = tag;
	RefreshSwitchData();
}

// Tag of the model component that is the usable switch component
void DataObjectContinuousSwitch::SetSwitchComponentTag(const std::string & tag)
{
	m_switch_component_tag = tag;
	RefreshSwitchData();
}

// Refresh the switch model after relevant data has been loaded
void DataObjectContinuousSwitch::RefreshSwitchData(void)
{
	// Validate range values
	if (m_constraint_min > m_constraint_max) std::swap(m_constraint_min, m_constraint_max);
	if (m_value_min > m_value_max) std::swap(m_value_min, m_value_max);

	// Make updates to the model, assuming it has been loaded
	if (m_articulated_model)
	{
		// Attempt to locate key components
		m_switch_component = m_articulated_model->GetComponentWithTag(m_switch_component_tag);
		m_switch_constraint = m_articulated_model->GetConstraintWithTag(m_switch_constraint_tag);

		// Make sure the switch rotation is valid within the current constraint range
		float current_rotation = m_articulated_model->GetConstraintRotation(m_switch_constraint);
		if (current_rotation < m_constraint_min) SetSwitchRotation(m_constraint_min);
		if (current_rotation > m_constraint_max) SetSwitchRotation(m_constraint_max);
	}
}

// Set the value range of this continuous switch
void DataObjectContinuousSwitch::SetValueRangeMin(float value_range_min)
{
	m_value_min = value_range_min;
	RefreshSwitchData();
}

// Set the value range of this continuous switch
void DataObjectContinuousSwitch::SetValueRangeMax(float value_range_max)
{
	m_value_max = value_range_max; 
	RefreshSwitchData();
}

// Set the constraint movement range for this continuous switch
void DataObjectContinuousSwitch::SetConstraintMin(float constraint_min_rotation)
{
	m_constraint_min = constraint_min_rotation;
	RefreshSwitchData();
}

// Set the constraint movement range for this continuous switch
void DataObjectContinuousSwitch::SetConstraintMax(float constraint_max_rotation)
{
	m_constraint_max = constraint_max_rotation;
	RefreshSwitchData();
}

// Set the value delta threshold.  A value change with magnitude below this threshold will not be 
// registerd and output by the switch
void DataObjectContinuousSwitch::SetValueDeltaThreshold(float threshold)
{
	// Threshold must be postive and non-zero
	m_value_delta_threshold = max(threshold, Game::C_EPSILON);
}

// Set the current rotation of the switch about its constraint.  Will clamp the rotation within the bounds 
// of the switch constraint
void DataObjectContinuousSwitch::SetSwitchRotation(float rotation)
{
	// Clamp the rotation parameter to a value within the acceptable switch rotation bounds
	rotation = clamp(rotation, m_constraint_min, m_constraint_max);

	// Set the switch model rotation
	if (m_articulated_model)
	{
		m_articulated_model->SetConstraintRotation(m_switch_constraint, rotation);
	}

	// Determine the value corresponding to this degree of rotation
	float value = SwitchValueForRotation(rotation);

	// Output this value from the switch component, as long as it differs from the previous value 
	// by more than the delta threshold
	if (std::fabs(value - m_last_value) > m_value_delta_threshold)
	{
		m_last_value = value;
		SendData(PORT_SEND, DataPorts::DataType(value));
	}
}

// Adjust the switch by the specified delta
void DataObjectContinuousSwitch::AdjustSwitchRotation(float rotation_delta)
{
	if (!m_articulated_model || m_switch_constraint < 0) return;

	SetSwitchRotation( m_articulated_model->GetConstraintRotation(m_switch_constraint) + rotation_delta );
}

// Set the current value of the switch, rotating the switch component accordingly.  Will clamp the value
// within the acceptable value range of the switch
void DataObjectContinuousSwitch::SetSwitchValue(float value)
{
	// Ensure the value lies within acceptable value bounds
	value = clamp(value, m_value_min, m_value_max);

	// Determine the rotation angle required to yield this value, and set the switch rotation accordingly
	float angle = SwitchRotationForValue(value);
	SetSwitchRotation(angle);
}

// Determines the switch rotation required in order to yield the given value
float DataObjectContinuousSwitch::SwitchRotationForValue(float value) const
{
	// Make sure there is a valid value range, otherwise we would end up dividing by zero later
	float value_range = (m_value_max - m_value_min);
	if (value_range < Game::C_EPSILON) return m_constraint_min;		// No range, so set to exact allowed value

	// Value % between min and max is (value - min) / (max - min)
	// e.g. 6 in range [-5, 10] -> (6 - -5) / (10 - -5) == 11/15
	// e.g. -3 in range [-5, 10] -> (-3 - -5) / (10 - -5) == 2/15
	float percentage = (value - m_value_min) / (value_range);

	// Translate this percentage to a rotation angle within the acceptable constraint bounds
	float rotation = percentage * (m_constraint_max - m_constraint_min);

	// Clamp to a value within the acceptable switch rotation bounds and return
	return clamp(rotation, m_constraint_min, m_constraint_max);
}

// Determines the value corresponding to a rotation of the switch to the specified angle
float DataObjectContinuousSwitch::SwitchValueForRotation(float rotation) const
{
	// Make sure there is a valid constraint range, otherwise we would end up dividing by zero later
	float constraint_range = (m_constraint_max - m_constraint_min);
	if (constraint_range < Game::C_EPSILON) return m_value_min;		// No range, so set to exact allowed value

	// Value % between min and max is (value - min) / (max - min)
	float percentage = (rotation - m_constraint_min) / (constraint_range);

	// Translate to the corresponding output value
	float value = percentage * (m_value_max - m_value_min);

	// Clamp to a value within acceptable bounds and return
	return clamp(value, m_value_min, m_value_max);
}


