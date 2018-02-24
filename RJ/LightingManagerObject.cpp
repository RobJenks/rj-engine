#include "DX11_Core.h"
#include "CoreEngine.h"
#include "GameVarsExtern.h"
#include "CameraClass.h"
#include "ObjectSearch.h"
#include "GameUniverse.h"
#include "SpaceSystem.h"
#include "LightSource.h"
#include "CPUGraphicsResourceAccess.h"
#include "Data/Shaders/LightDataBuffers.hlsl"

#include "LightingManagerObject.h"

// Initialise static comparator for binary search comparisons
LightingManagerObject::_LightSourceEntryPriorityComparator LightingManagerObject::LightSourceEntryPriorityComparator = LightingManagerObject::_LightSourceEntryPriorityComparator();


// Default constructor
LightingManagerObject::LightingManagerObject(void)
	:
	m_source_count(0U), m_lighting_is_overridden(false), m_source_vector_is_sorted(false)
{
	// Initialise space in the light source vector to hold the maximum possible number of lights
	m_sources.clear();
	for (int i = 0; i < LightingManagerObject::LIGHT_LIMIT; ++i) m_sources.push_back(NULL);

	// Initialise space in the lighting override vector to hold the maximum possible number of lights
	m_override_lights.clear();
	m_override_lights.reserve(LightingManagerObject::LIGHT_LIMIT);

	// Create the structured buffer that will hold compiiled frame lighting data
	m_sb_lights = Game::Engine->GetRenderDevice()->Assets.CreateStructuredBuffer<LightData>(LightBufferName, LightingManagerObject::LIGHT_LIMIT, CPUGraphicsResourceAccess::Write);
	if (!m_sb_lights)
	{
		Game::Log << LOG_ERROR << "Failed to instantiate lighting manager buffers for frame lighting data\n";
	}

	// Initialise the pre-configured lighting setups that can be used as a standard override
	InitialiseStandardLightingOverrides();
}

// Initialise the lighting manager for a new frame
void LightingManagerObject::AnalyseNewFrame(void)
{
	// Clear the current set of registered light sources
	ClearAllLightSources();

	// Test whether there is a lighting override in place
	std::vector<iObject*> & lights = _m_frame_light_sources;
	if (m_lighting_is_overridden)
	{
		// Special case.  Lighting is overriden so instead just reference the override vector directly
		lights = m_override_lights;

		// Reset the override state now that it has been applied in the current frame
		DisableLightingOverride();
	}
	else
	{
		// Now perform a search of the local area for all NON-DIRECTIONAL light sources
		const SpaceSystem & system = Game::Universe->GetCurrentSystem();
		Game::Search<iObject>().CustomSearch(Game::Engine->GetCamera()->GetPosition(), system.SpatialPartitioningTree,
			Game::C_LIGHT_RENDER_DISTANCE, _m_frame_light_sources, LightingManagerObject::LightNotOfSpecificType(LightType::Directional));

		// Pre-register all directional system list sources since we know they will always be relevant, and they 
		// will not be returned by the object search method above
		std::vector<ObjectReference<LightSource>>::const_iterator it_end = system.SystemLightSources().end();
		for (std::vector<ObjectReference<LightSource>>::const_iterator it = system.SystemLightSources().begin(); it != it_end; ++it)
		{
			RegisterLightSource((*it)());
		}
	}

	// Register any lights which could have an impact on the current viewing frustum.  If we exceed
	// the maxmimum supported light count, sources will be prioritised and culled accordingly
	LightSource *light;
	std::vector<iObject*>::const_iterator it_end = lights.end();
	for (std::vector<iObject*>::const_iterator it = lights.begin(); it != it_end; ++it)
	{
		// We know that all search results will be light sources
		light = (LightSource*)(*it); if (!light) continue;

		// Check whether the light could impact the viewing frustum.   We can skip the range check for
		// directional lights since they are defined to be unsituated
		if (light->GetLight().GetType() == LightType::Directional ||  
			Game::Engine->GetViewFrustrum()->CheckSphere(light->GetPosition(), light->GetLight().Data.Range))
		{
			// Register this as a potentialy-important light source
			RegisterLightSource(light);
		}
	}

	// Copy relevant lighting data out of each light source for rendering
	for (LightSources::size_type i = 0; i < m_source_count; ++i)
	{
		m_light_data[i] = m_sources[i].Source->GetLight().Data;
	}

	// Update contents of the structured buffer in texture memory, ready for rendering
	m_sb_lights->Set(&(m_light_data[0]), static_cast<UINT>(sizeof(LightData) * m_source_count));
}

// Register a new light source for this scene
bool LightingManagerObject::RegisterLightSource(LightSource *light)
{
	// Parameter check
	if (!light) return false;
	const Light & l = light->GetLight();

	// We only want to register lights that are currently active
	if (l.IsActive() == false) return false;

	// This light is likely to be included in the rendered scene; recalculate any per-frame rendering data at this
	// point so it is available (assuming the light is not deprioritised later, but it is neater to do this once here)
	light->RecalculateRenderingData();

	// Lights are prioritised in part based on their position relative to the camera
	const XMVECTOR & cam_pos = Game::Engine->GetCamera()->GetPosition();

	// Determine a distance/priority for the light; directional lights always have top priority so set a distsq of -999
	// This ensures that directional light sources will ALWAYS be at the start of the light vector
	float distsq = -999.0f;
	if ((LightType)l.GetType() != LightType::Directional)
	{
		// Directional lights keep a distsq value of -999.  For all other light types, determine the actual squared
		// distance here.  Will always be >= 0 and so always lower priority than the directional lights
		distsq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(light->GetPosition(), cam_pos)));
	}

	// Take different action depending on how many lights are currently registered
	if (m_source_count < LIGHT_LIMIT)
	{
		// We are under the light limit, so simply add to the collection and increment the counter
		m_sources[m_source_count] = LightingManagerObject::LightSourceEntry(light, distsq);
		++m_source_count;
		return true;
	}
	else
	{
		// Otherwise, we need to perform prioritisation of light sources to fit within the limit
		// We sort the vector ONCE at the point we first reach the limit, and then perfomed a sorted-insert
		// for all future light sources
		if (m_source_vector_is_sorted == false)
		{
			std::sort(m_sources.begin(), m_sources.end(), LightingManagerObject::LightSourceEntryPriorityComparator);
			m_source_vector_is_sorted = true;
		}

		// Find the first light source that is lower priority than us; if == end() we are lower priority than all of them
		LightSourceEntry entry = LightSourceEntry(light, distsq);
		LightSources::iterator it = std::lower_bound(m_sources.begin(), m_sources.end(), entry, LightingManagerObject::LightSourceEntryPriorityComparator);
		if (it != m_sources.end())
		{
			return false;
		}

		// We are higher priority than the item at 'it'.  Insert this light before it, and discard the lowest
		// element (since we now have LIMIT+1 elements).  DO NOT increment the source count since we are 
		// replacing an existing light source
		m_sources.insert(it, entry);
		m_sources.pop_back();
		return true;
	}
}

// Clear all registered light sources
void LightingManagerObject::ClearAllLightSources(void)
{
	// Remove all currently-registered light sources by simply setting the count back to zero
	m_source_count = 0U;	

	// The vector is also no longer sorted, and will not be sorted again until needed
	m_source_vector_is_sorted = false;

	// Remove any temporary light source search results from the previous frame
	_m_frame_light_sources.clear();
}


// Return the light configuration relevant to the specified object
/*Game::LIGHT_CONFIG LightingManagerObject::GetLightingConfigurationForObject(const iObject *object)
{
	// Results will be returned in a light config bitstring
	Game::LIGHT_CONFIG config = 0U;

	// Iterate through each potentially-relevant light in turn
	for (LightSources::size_type i = 0; i < m_source_count; ++i)
	{
		// Get a reference to the light and make sure it is active
		const LightSource & lightsrc = *(m_sources[i].Source);
		const Light & light = lightsrc.GetLight();
		if (light.IsActive() == false) continue;

		// Take different action based on the type of light
		if (light.Data.Type == (int)LightType::Directional)
		{
			// If this is a directional light it should always be included
			SetBit(config, m_config_lookup[i]);
		}
		else
		{
			// Otherwise, only set this light source to active in the output light config if it is in range
			float r1r2 = (object->GetCollisionSphereRadius() + lightsrc.GetCollisionSphereRadius());
			float distsq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(object->GetPosition(), lightsrc.GetPosition())));
			if (distsq < (r1r2 * r1r2))
			{
				// The object is in range of this light
				SetBit(config, m_config_lookup[i]);
			}
		}
	}

	// Return the relevant lighting configuration
	return config;
}*/

// Called at the end of a frame to perform any final lighting-related activities
void LightingManagerObject::EndFrame(void)
{
}

// Returns data for a basic, default unsituated directional light
void LightingManagerObject::GetDefaultDirectionalLightData(LightData & outLight)
{
	// Populate with default values
	outLight.Type = LightType::Directional;
	outLight.Colour = XMFLOAT4(1.0f, 1.0f, 0.82f, 1.0f);
	outLight.DirectionWS = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	outLight.Enabled = true;
	outLight.Intensity = 0.75f;
}

// Returns data for a basic, default point light
void LightingManagerObject::GetDefaultPointLightData(LightData & outLight)
{
	// Populate with default values
	outLight.Type = LightType::Point;
	outLight.Colour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	outLight.Range = 500.0f;
	outLight.Enabled = true;
}

// Returns data for a basic, default spot light
void LightingManagerObject::GetDefaultSpotLightData(LightData & outLight)
{
	// Populate with default values
	outLight.Type = LightType::Spotlight;
	outLight.Colour = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	outLight.Range = 500.0f;
	outLight.Enabled = true;
	outLight.SpotlightAngle = (PIBY180 * 30.0f);
}

// Initialise the pre-configured lighting setups that can be used as a standard override
void LightingManagerObject::InitialiseStandardLightingOverrides(void)
{
	LightData std_dim, std_bright;
	GetDefaultDirectionalLightData(std_dim);
	GetDefaultDirectionalLightData(std_bright);
	std_dim.Intensity = 0.15f;
	std_bright.Intensity = 0.75f;

	// Standard camera-facing override.  Directional lighting only which primarily shines out of the 
	// camera, but also includes a more limited amount of ambient/surrounding light from other angles 
	XMVECTOR orient[] = { 
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),						// Light 1: Facing directly forwards from the camera; e.g. (0,0,-10) to local (0,0,0)
		XMVectorSet(0.330013f, 0.660026f, -0.0f, 0.674875f),		// Light 2: Facing from (10,-5,1) up+left to local (0,0,0)
		XMVectorSet(0.151432f, -0.757161f, 0.0f, 0.635433f) };		// Light 3: Facing from	(10,-2,2) up+right to local (0,0,0)

	for (int i = 0; i < 3; ++i)
	{
		LightSource *l = LightSource::Create((i == 0 ? std_bright : std_dim));
		l->SetRelativeLightOrientation(orient[i]);
		l->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
		m_std_override_cameralights.push_back((iObject*)l);
	}
}

// Orients a lighting set with the camera
void LightingManagerObject::OrientLightingOverrideSetWithCamera(std::vector<iObject*> & lighting_override)
{
	const XMVECTOR & camera_orient = Game::Engine->GetCamera()->GetOrientation();

	std::vector<iObject*>::iterator it_end = lighting_override.end();
	for (std::vector<iObject*>::iterator it = lighting_override.begin(); it != it_end; ++it)
	{
		if ((*it)) (*it)->SetOrientation(camera_orient);
	}
}

// Apply a standard lighting override this frame
void LightingManagerObject::ApplyStandardCameraFacingLightOverride(void)
{
	OrientLightingOverrideSetWithCamera(m_std_override_cameralights);
	AddOverrideLights(m_std_override_cameralights);
}

// Returns data for a basic, default unsituated directional light.  Creates a returns a new instance by value
LightData LightingManagerObject::GetDefaultDirectionalLightData(void)
{
	LightData data;
	GetDefaultDirectionalLightData(data);
	return data;
}

// Returns data for a basic, default point light.  Creates a returns a new instance by value
LightData LightingManagerObject::GetDefaultPointLightData(void)
{
	LightData data;
	GetDefaultPointLightData(data);
	return data;
}

// Returns data for a basic, default spot light.  Creates a returns a new instance by value
LightData LightingManagerObject::GetDefaultSpotLightData(void)
{
	LightData data;
	GetDefaultSpotLightData(data);
	return data;
}

bool LightingManagerObject::_LightSourceEntryPriorityComparator::operator() (const LightSourceEntry & lhs, const LightSourceEntry & rhs) const
{
	// Light sources are sorted by priority, followed by inverse distance to camera if required
	return ((lhs.Source->GetPriority() > rhs.Source->GetPriority()) && (lhs.DistSq < rhs.DistSq));
}

// Default destructor
LightingManagerObject::~LightingManagerObject(void)
{

}

// Generates a debug output string for the current lighting state
std::string LightingManagerObject::DebugOutputLightingState(void) const
{
	const LightSource *ls;
	std::ostringstream os;
	os << "Lights" << (m_lighting_is_overridden ? "(OVERIDDEN)" : "") << "[" << m_source_count << "] = {";
	
	for (LightSources::size_type i = 0; i < m_source_count; ++i)
	{
		ls = m_sources[i].Source;
		os << (i == 0 ? " " : ", ");

		if (ls == NULL)
		{
			os << "[ERROR: NULL]";
		}
		else
		{
			os << "[ID=" << ls->GetID() << ", " << ls->DebugLightDataString() << ", DSQ=" << m_sources[i].DistSq << "]";
		}
	}

	os << " }";
	return os.str();
}

// Virtual inherited method to accept a command from the console
bool LightingManagerObject::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "GetCurrentLightSources")
	{
		command.SetSuccessOutput(DebugOutputLightingState());
		return true;
	}

	// We did not recognise the command
	return false;
}