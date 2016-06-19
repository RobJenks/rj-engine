#include "Data\\Shaders\\render_constants.h"
#include "Data\\Shaders\\light_definition.h"
#include "DX11_Core.h"
#include "CoreEngine.h"
#include "GameVarsExtern.h"
#include "CameraClass.h"
#include "ViewFrustrum.h"
#include "ObjectSearch.h"
#include "SpaceSystem.h"
#include "LightSource.h"

#include "LightingManagerObject.h"

// Initialise static comparator for binary search comparisons
LightingManagerObject::_LightSourceEntryPriorityComparator LightingManagerObject::LightSourceEntryPriorityComparator = LightingManagerObject::_LightSourceEntryPriorityComparator();


// Default constructor
LightingManagerObject::LightingManagerObject(void)
	:
	m_dir_light_count(0U), m_source_count(0U), m_active_config(0U)
{
	// Initialise space in the light source vector to hold the maximum possible number of lights
	m_sources.clear();
	for (int i = 0; i < LightingManagerObject::LIGHT_LIMIT; ++i) m_sources.push_back(NULL);

	// Initialise the light config lookup table
	for (int i = 0; i < LightingManagerObject::LIGHT_LIMIT; ++i)
	{
		// Each light uses one bit, e.g. light #0 = 0x0, #1 = 0x1, #2 = 0x2, #3 = 0x4, ...
		m_config_lookup[i] = (1 << i);
	}

	// Initialise space in the directional light data vector to hold the maximum possible number of directional lights
	m_dir_light.clear();
	m_dir_light.reserve(LightingManagerObject::DIR_LIGHT_LIMIT);
}

// Initialise the lighting manager for a new frame
void LightingManagerObject::AnalyseNewFrame(void)
{
	// Clear the current set of registered light sources
	ClearAllLightSources();

	// Perform a search of the local area for all light sources
	std::vector<iObject*> lights;
	int count = Game::ObjectSearch<iObject>::CustomSearch(Game::Engine->GetCamera()->GetPosition(), Game::CurrentPlayer->GetSystem()->SpatialPartitioningTree,
		Game::C_LIGHT_RENDER_DISTANCE, lights, Game::ObjectSearch<iObject>::ObjectIsOfType(iObject::ObjectType::LightSourceObject));

	// Register any lights which could have an impact on the current viewing frustum.  If we exceed
	// the maxmimum supported light count, sources will be prioritised and culled accordingly
	LightSource *light;
	std::vector<iObject*>::const_iterator it_end = lights.end();
	for (std::vector<iObject*>::const_iterator it = lights.begin(); it != it_end; ++it)
	{
		// We know that all search results will be light sources
		light = (LightSource*)(*it); if (!light) continue;

		// Check whether the light could impact the viewing frustum
		if (Game::Engine->GetViewFrustrum()->CheckSphere(light->GetPosition(), light->GetLight().Data.Range))
		{
			// Register this as a potentialy-important light source
			RegisterLightSource(light);
		}
	}

	// Directional light sources are always located at the start of the light data array
	if (m_dir_light_count != 0U)
	{
		memcpy(&(m_light_data[0]), &(m_dir_light[0]), sizeof(LightData) * m_dir_light_count);
	}

	// Now parse any registered light sources and transfer core lighting data to the data array
	for (LightSources::size_type i = 0; i < m_source_count; ++i)
	{
		m_light_data[LightingManagerObject::DIR_LIGHT_LIMIT + i] = m_sources[i].Source->GetLight().Data;
	}
}

// Register a new light source for this scene
bool LightingManagerObject::RegisterLightSource(const LightSource *light)
{
	// Parameter check
	if (!light) return false;

	// Lights are prioritised in part based on their position relative to the camera
	const XMVECTOR & cam_pos = Game::Engine->GetCamera()->GetPosition();

	// Take different action depending on how many lights are currently registered
	if (m_source_count < LIGHT_LIMIT)
	{
		// We are under the light limit, so simply add to the collection and increment the counter
		m_sources[m_source_count] = LightingManagerObject::LightSourceEntry(light, XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(light->GetPosition(), cam_pos))));
		++m_source_count;
		return true;
	}
	else
	{
		// Otherwise, we need to perform prioritisation of light sources to fit within the limit
		if (m_source_count == LIGHT_LIMIT)
		{
			// If we are exactly at the limit, we need to sort all elements.  Every subsequent addition (including this
			// one) this frame must then be prioritised against the existing sorted objects
			std::sort(m_sources.begin(), m_sources.end(), LightingManagerObject::LightSourceEntryPriorityComparator);
		}

		// Find the first light source that is lower priority than us; if == end() we are lower priority than all of them
		LightSourceEntry entry = LightSourceEntry(light, XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(light->GetPosition(), cam_pos))));
		LightSources::iterator it = std::lower_bound(m_sources.begin(), m_sources.end(), entry, LightingManagerObject::LightSourceEntryPriorityComparator);
		if (it != m_sources.end())
		{
			return false;
		}

		// We are higher priority than the item at 'it'.  Insert this light before it, and discard the lowest
		// element (since we now have LIMIT+1 elements)
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
}


// Return the light configuration relevant to the specified object
Game::LIGHT_CONFIG LightingManagerObject::GetLightingConfigurationForObject(const iObject *object)
{
	// Results will be returned in a light config bitstring
	Game::LIGHT_CONFIG config = 0U;

	// Iterate through each potentially-relevant light in turn
	const LightSource *light; 
	for (LightSources::size_type i = 0; i < m_source_count; ++i)
	{
		// Get a reference to the light
		light = m_sources[i].Source;

		// Set this light source to active in the output light config if it is in range
		float r1r2 = (object->GetCollisionSphereRadius() + light->GetCollisionSphereRadius());
		float distsq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(object->GetPosition(), light->GetPosition())));
		if (distsq < (r1r2 * r1r2))
		{
			// The object is in range of this light
			SetBit(config, m_config_lookup[i]);
		}
	}

	// Return the relevant lighting configuration
	return config;
}

// Update directional light data for the scene
bool LightingManagerObject::AddDirectionalLight(const LightData & data)
{
	// Ensure we remain below the limit
	if (m_dir_light_count >= LightingManagerObject::DIR_LIGHT_LIMIT) return false;

	// Push into the collection and return success
	m_dir_light.push_back(data);
	m_dir_light_count = m_dir_light.size();
	return true;
}

// Returns a flag indicating whether we can add any new directional lights to the scene (or whether we are at the limit)
bool LightingManagerObject::CanAddNewDirectionalLight(void) const
{
	return ((m_dir_light_count + 1) < LightingManagerObject::DIR_LIGHT_LIMIT);
}

// Update directional light data for the scene
void LightingManagerObject::UpdateDirectionalLight(DirLightData::size_type index, const LightData & data)
{
	if (index >= 0 && index < m_dir_light_count) m_dir_light[index] = data;
}

// Update directional light data for the scene
void LightingManagerObject::RemoveDirectionalLight(DirLightData::size_type index)
{
	if (index >= 0 && index < m_dir_light_count) 
	{
		m_dir_light.erase(m_dir_light.begin() + index);
		m_dir_light_count = m_dir_light.size();
	}
}

// Remove all directional lights from the scene
void LightingManagerObject::ClearDirectionalLightData(void)
{
	m_dir_light.clear();
	m_dir_light_count = m_dir_light.size();
}

// Returns data for a basic, default unsituated directional light
void LightingManagerObject::GetDefaultDirectionalLightData(LightData & outLight)
{
	// Populate with default values
	outLight.Type = LightingManagerObject::LightType::Directional;
	outLight.Colour = XMFLOAT3(1.0f, 1.0f, 0.82f);
	outLight.AmbientIntensity = 0.1f;
	outLight.DiffuseIntensity = 0.1f;
	outLight.SpecularPower = 0.05f;
	outLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
}

// Returns data for a basic, default point light
void LightingManagerObject::GetDefaultPointLightData(LightData & outLight)
{
	// Populate with default values
	outLight.Type = LightingManagerObject::LightType::PointLight;
	outLight.Colour = XMFLOAT3(1.0f, 1.0f, 1.0f);
	outLight.AmbientIntensity = 30.0f;
	outLight.DiffuseIntensity = 26.0f;
	outLight.SpecularPower = 0.5f;
	outLight.Range = 500.0f;
	outLight.Attenuation.Constant = 1.0f;
	outLight.Attenuation.Linear = 0.012f;
	outLight.Attenuation.Exp = 0.0052f;
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