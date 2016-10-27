#include "GameVarsExtern.h"
#include "CoreEngine.h"
#include "LightingManagerObject.h"
#include "Light.h"

#include "LightSource.h"


// Creates a new light source with default properties
LightSource * LightSource::Create(void)
{
	return LightSource::Create(Light(Game::Engine->LightingManager.GetDefaultPointLightData()));
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
	LightSource *ls = new LightSource();
	if (!ls) return NULL;

	// Apply supplied lighting data to the source
	ls->SetLight(light);

	// Set an initial simulation state.  This will likely be overridden in the next frame, but will
	// ensure that the object is registered upon construction
	ls->SetSimulationState(iObject::ObjectSimulationState::StrategicSimulation);

	// Return the new light source
	return ls;
}


// Default constructor
LightSource::LightSource(void)
	:
	m_priority(0), 
	m_relativelightorient(ID_QUATERNION)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::LightSourceObject);

	// Light sources will not collide with anything (although their collision radii are used for illumination tests)
	SetCollisionMode(Game::CollisionMode::NoCollision);
	SetCollisionSphereRadius(1.0f);

	// Light sources do perform a post-simulation update to reposition their internal light component
	SetPostSimulationUpdateFlag(true);
}


// Set the lighting data for this light source
void LightSource::SetLight(const Light & data)
{
	// Store the lighting data
	m_light = data;

	// Set light range directly, which will perform validation and update object properties accordingly
	SetRange(data.Data.Range);
}

// Set the range of this light source
void LightSource::SetRange(float range)
{
	// Store the new range value; validate to ensure valid positive values
	m_light.Data.Range = max(range, Game::C_EPSILON);

	// The object collision radius will be set to equal the light range, for more efficient light 'collision' testing with illuminated objects
	SetCollisionSphereRadius(m_light.Data.Range);
}


// Light sources do implement a post-simulation update method to reposition their internal light component
void LightSource::PerformPostSimulationUpdate(void)
{
	// Update the position of our internal light component
	m_light.Data.Position = m_positionf;

	// Update light direction based on orientation of the light source object
	XMVECTOR dir = XMVector3Rotate(FORWARD_VECTOR, (XMQuaternionMultiply(m_relativelightorient, m_orientation)));
	XMStoreFloat3(&m_light.Data.Direction, dir);
}

// Custom debug string function
std::string	LightSource::DebugString(void) const
{
	return iObject::DebugString(DebugLightDataString());
}

// Custom debug string function for light data specifically
std::string LightSource::DebugLightDataString(void) const
{
	return concat("Type=")(Light::TranslateLightTypeToString((Light::LightType)m_light.Data.Type))
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
	REGISTER_DEBUG_FN(DestroyObject)

	// Redirect method for all Light object properties
	REGISTER_DEBUG_FN_REDIRECT(Light, m_light.ProcessDebugCommand)

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iObject::ProcessDebugCommand(command);

}

