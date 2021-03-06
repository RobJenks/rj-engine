#include "ArticulatedModel.h"
#include "iObject.h"
#include "CoreEngine.h"
#include "CameraClass.h"

#include "DataObjectContinuousSwitch.h"

// Initialise static data
const float DataObjectContinuousSwitch::PLAYER_INPUT_RANGE_SCREEN_COORDS = 0.3f;
const float DataObjectContinuousSwitch::PLAYER_INPUT_MAX_SPEED = (PI / 4.0f); // rad/sec

// Default constructor
DataObjectContinuousSwitch::DataObjectContinuousSwitch(void)
	:
	m_switch_component_tag(NullString), m_switch_component(-1), m_switch_constraint_tag(NullString), m_switch_constraint(-1), 
	m_constraint_axis(UP_VECTOR_F), m_constraint_min(-PI / 4.0f), m_constraint_max(+PI / 4.0f), m_max_rotation_speed(PI / 4.0f), 
	m_value_min(-1.0f), m_value_max(+1.0f), m_value_delta_threshold(0.01f), 
	m_player_interaction_mouse_start(NULL_FLOAT2), 
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
	else if (hash == HashedStrings::H_MaxRotationSpeed)			SetSwitchMaxRotationSpeed((float)atof(value.c_str()));

}

// Method invoked when this switch is used by an entity.  Continuous switches only accept extended
// interaction types
bool DataObjectContinuousSwitch::OnUsed(iObject *user, DynamicTerrainInteraction && interaction)
{
	// Get the target rotation value for the switch
	float target_rotation = interaction.GetValue();
	if (interaction.IsPlayerInteraction())
	{
		// If this is a player interaction, we need to calculate their desired rotation value based 
		// upon the current mouse input
		target_rotation = CalculatePlayerInteractionTargetRotation(user);
	}
	
	// Adjust the switch rotation towards this target value
	AdjustSwitchRotationTowardsTarget(target_rotation);

	// Successful interaction
	return true;
}

// Calculate the target rotation for player interactions, where it must be derived based on player input state
float DataObjectContinuousSwitch::CalculatePlayerInteractionTargetRotation(iObject *player_object)
{
	// If this is the first cycle of the interaction, store the current player input state so we can
	// calculate deltas from it in subsequent cycles
	if (!IsInteractionInProgress())
	{
		m_player_interaction_mouse_start = Game::Mouse.GetNormalisedMousePos();
	}

	// Project the player mouse input movement onto the switch constraint vector and determine the degree of
	// movement in the relevant axis, accounting for the player view heading
	float input_delta_pc = DeterminePlayerInputDeltaPercentage(m_player_interaction_mouse_start, Game::Mouse.GetNormalisedMousePos());
	
	// Translate this input into a target rotation value, based on the maximum player input speed
	float rotation_delta = (input_delta_pc * PLAYER_INPUT_MAX_SPEED);

	// Set a rotation target based upon this delta
	return (GetSwitchRotation() + rotation_delta);
}

// Determine the input delta based on player input.  Returns a value indicating the degree of mouse movement 
// that aligns with the switch constraint, based on the current view orientation and mouse input state
float DataObjectContinuousSwitch::DeterminePlayerInputDelta(const XMFLOAT2 & mouse_start_norm_pos, const XMFLOAT2 & mouse_norm_pos) const
{
	// Take the dot product of current player view heading and the constraint axis.  This will allow us to determine 
	// the relative orientation of the switch constraint and how the input movement should be mapped
	XMVECTOR axis = XMLoadFloat3(&m_constraint_axis);
	XMVECTOR dot_view_axis = XMVector3Dot(Game::Engine->GetCamera()->GetCameraHeading(), axis);
	float dot = XMVectorGetX(dot_view_axis);
	float dot_abs = fabsf(dot);

	// We will use a weighted combination of both x- and y-input deltas.  If the dot product is negative
	// we reverse the direction of this input movement vector since all actions will be mirrored with respect to the target
	XMFLOAT2 input_delta;
	if (dot >= 0.0f)	input_delta = Float2Subtract(Game::Mouse.GetNormalisedMousePos(), m_player_interaction_mouse_start);
	else				input_delta = Float2Subtract(m_player_interaction_mouse_start, Game::Mouse.GetNormalisedMousePos());

	// Now weight the delta based upon the dot product, i.e. the degree to which we align with the constraint axis
	float result = ((dot_abs * input_delta.x) + ((1.0f - dot_abs) * input_delta.y));
	return result;
}

// Determine the input delta percentage based on player input.  Will return a value in the range [-1.0 +1.0] based
// on the player mouse input and current player view orientation
float DataObjectContinuousSwitch::DeterminePlayerInputDeltaPercentage(const XMFLOAT2 & mouse_start_norm_pos, const XMFLOAT2 & mouse_norm_pos) const
{
	// Determine the absolute delta figure
	float delta = DeterminePlayerInputDelta(mouse_start_norm_pos, mouse_norm_pos);

	// Convert to a percentage delta based on the defined screen-space movement range
	float delta_pc = (delta / PLAYER_INPUT_RANGE_SCREEN_COORDS);

	// Clamp within the bound [-1 +1] and return
	return clamp(delta_pc, -1.0f, 1.0f);
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

		// Store the axis which this switch constraint will rotate about
		XMVECTOR axis = m_articulated_model->GetConstraintAxis(m_switch_constraint);
		XMStoreFloat3(&m_constraint_axis, axis);

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

// Set the maximum possible rotation speed (rad/sec) for the switch component
void DataObjectContinuousSwitch::SetSwitchMaxRotationSpeed(float max_rotation_speed)
{
	m_max_rotation_speed = max_rotation_speed;
}

// Validate the provided switch rotation value and return a value which is valid
float DataObjectContinuousSwitch::ValidateSwitchRotation(float rotation) const
{
	// Keep all values within [0 2PI); do not allow rotations of greater than one revolution
	if (rotation < NEG_TWOPI)
	{
		do { rotation += TWOPI; } while (rotation < NEG_TWOPI);
	}
	else if (rotation >= TWOPI)
	{
		do { rotation -= TWOPI; } while (rotation >= TWOPI);
	}

	// Clamp within the acceptable switch rotation range and return
	return clamp(rotation, m_constraint_min, m_constraint_max);
}

// Validate the provided switch value and return a value which is valid
float DataObjectContinuousSwitch::ValidateSwitchValue(float value) const
{
	// Clamp within the acceptable value range and return
	return clamp(value, m_value_min, m_value_max);
}

float DataObjectContinuousSwitch::GetSwitchRotation(void) const 
{ 
	return (m_articulated_model ? m_articulated_model->GetConstraintRotation(m_switch_constraint) : 0.0f); 
}

// Set the current rotation of the switch about its constraint.  Will clamp the rotation within the bounds 
// of the switch constraint
void DataObjectContinuousSwitch::SetSwitchRotation(float rotation)
{
	// Clamp the rotation parameter to a value within the acceptable switch rotation bounds
	rotation = ValidateSwitchRotation(rotation);

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
	if (rotation_delta == 0.0f) return;

	SetSwitchRotation( GetSwitchRotation() + rotation_delta );
}

// Adjust the switch rotation towards the specified target point, accounting for e.g. max rotation
// speed of the switch component and the current time step
void DataObjectContinuousSwitch::AdjustSwitchRotationTowardsTarget(float target_rotation)
{
	// Constrain the target rotation to lie withing acceptable component bounds
	target_rotation = ValidateSwitchRotation(target_rotation);

	// Calculate the delta between our current and target rotation
	float delta = (target_rotation - GetSwitchRotation());

	// Constrain the delta to our maximum rotation speed and apply it
	float max_delta_abs = (GetSwitchMaxRotationSpeed() * Game::TimeFactor);
	delta = clamp(delta, -max_delta_abs, max_delta_abs);
	AdjustSwitchRotation(delta);
}

// Set the current value of the switch, rotating the switch component accordingly.  Will clamp the value
// within the acceptable value range of the switch
void DataObjectContinuousSwitch::SetSwitchValue(float value)
{
	// Ensure the value lies within acceptable value bounds
	value = ValidateSwitchValue(value);

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
	return ValidateSwitchRotation(rotation);
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
	float value = m_value_min + (percentage * (m_value_max - m_value_min));

	// Clamp to a value within acceptable bounds and return
	return ValidateSwitchValue(value);
}


