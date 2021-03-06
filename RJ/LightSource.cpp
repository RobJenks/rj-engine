#include "GameVarsExtern.h"
#include "CoreEngine.h"
#include "LightingManagerObject.h"
#include "Light.h"

#include "LightSource.h"


// Creates a new light source with default properties
LightSource * LightSource::Create(void)
{
	return LightSource::Create(Light(Game::Engine->LightingManager->GetDefaultPointLightData()));
}

// Creates a new light source based on the supplied light data
LightSource * LightSource::Create(const LightData & data)
{
	return LightSource::Create(Light(data));
}

// Creates a new light source based on the supplied light object
LightSource * LightSource::Create(const Light & light)
{
	// Create a new light source object
	LightSource *ls = new LightSource(light);
	if (!ls) return NULL;

	// Set an initial simulation state.  This will likely be overridden in the next frame, but will
	// ensure that the object is registered upon construction
	ls->SetSimulationState(iObject::ObjectSimulationState::StrategicSimulation);

	// Return the new light source
	return ls;
}


// Default constructor
LightSource::LightSource(const Light & data)
	:
	m_priority(0), 
	m_relativelightorient(ID_QUATERNION)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::LightSourceObject);

	// Store the lighting data
	m_light = data;

	// Set light range directly, which will perform validation and update object properties accordingly
	SetRange(data.Data.Range);

	// Light sources will not collide with anything (although their collision radii are used for illumination tests)
	SetCollisionMode(Game::CollisionMode::NoCollision);
	SetCollisionSphereRadius(1.0f);

	// Determine an object code for the light based on its properties
	DetermineObjectCode();

	// Light sources do not require any post-simulation update
	SetPostSimulationUpdateFlag(false);
}

// Determine an object code for the light based on its properties
void LightSource::DetermineObjectCode(void)
{
	SetCode(concat("LightSource-")(Light::TranslateLightTypeToString((LightType)m_light.GetType())).str());
}

// Set the range of this light source
void LightSource::SetRange(float range)
{
	// Store the new range value; validate to ensure valid positive values
	m_light.Data.Range = max(range, Game::C_EPSILON);

	// The object collision radius will be set to equal the light range, for more efficient light 'collision' testing with illuminated objects
	SetCollisionSphereRadius(m_light.Data.Range);
}

// Calculate point light volume transform for the given light
XMMATRIX LightSource::CalculatePointLightTransform(const LightData & light)
{
	return XMMatrixMultiply(
		XMMatrixScaling(light.Range, light.Range, light.Range),
		XMMatrixTranslation(light.PositionWS.x, light.PositionWS.y, light.PositionWS.z)
	);
}

// Calculate spotlight light volume transform for the given light
XMMATRIX LightSource::CalculateSpotlightTransform(const LightData & light)
{
	// Spotlight cone radius: tan(x) = O/A -> O = Atan(x)
	float cone_radius = light.Range * std::tanf(light.SpotlightAngle);
	return XMMatrixMultiply(XMMatrixMultiply(
		XMMatrixScaling(cone_radius, cone_radius, light.Range),
		XMMatrixRotationQuaternion(QuaternionBetweenVectors(FORWARD_VECTOR, XMLoadFloat4(&light.DirectionWS)))),	// TODO: Can likely make this more efficient
		XMMatrixTranslation(light.PositionWS.x, light.PositionWS.y, light.PositionWS.z)
	);
}

// Calculates all derived data for the light required for rendering, for example view-space equivalent data.  We only need to calculate
// these fields if the light is being rendered this frame
void LightSource::RecalculateRenderingData(void)
{
	/** Position: common to all light types **/

	// World-space light position
	XMStoreFloat4(&m_light.Data.PositionWS, m_position);

	// View-space light position
	const XMMATRIX & view = Game::Engine->GetRenderViewMatrix();
	XMStoreFloat4(&m_light.Data.PositionVS, XMVector3TransformCoord(m_position, view));

	/** Direction: We can skip these calculations for point lights **/
	if (m_light.GetType() != LightType::Point)
	{
		// We need to derive the heading for lighting calcs.  Also store within the object given that we are doing the work
		PhysicsState.Heading = XMVector3Rotate(FORWARD_VECTOR, m_orientation);

		// World space light direction
		XMVECTOR dirWS = XMVector3Rotate(PhysicsState.Heading, m_relativelightorient);
		XMStoreFloat4(&m_light.Data.DirectionWS, dirWS);

		// View-space light direction
		XMStoreFloat4(&m_light.Data.DirectionVS, XMVector3NormalizeEst(XMVector3TransformCoord(dirWS, view)));
	}

}

// Custom debug string function
std::string	LightSource::DebugString(void) const
{
	return iObject::DebugString(DebugLightDataString());
}

// Custom debug string function for light data specifically
std::string LightSource::DebugLightDataString(void) const
{
	return concat("Type=")(Light::TranslateLightTypeToString((LightType)m_light.Data.Type))
		(", Active=")((m_light.IsActive() ? "True" : "False")).str();
	//	(", R=")(m_light.Data.Colour.x)(", G=")(m_light.Data.Colour.y)(", B=")(m_light.Data.Colour.z)
	//	(", A=")(m_light.Data.AmbientIntensity)(", D=")(m_light.Data.DiffuseIntensity)(", S=")(m_light.Data.SpecularPower).str();
}


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void LightSource::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetPriority)
	REGISTER_DEBUG_ACCESSOR_FN(GetRange)

	// Mutator methods
	REGISTER_DEBUG_FN(SetPriority, command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(SetRange, command.ParameterAsFloat(2))

	// Redirect method for all Light object properties
	REGISTER_DEBUG_FN_REDIRECT(Light, m_light.ProcessDebugCommand)

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iObject::ProcessDebugCommand(command);

}

