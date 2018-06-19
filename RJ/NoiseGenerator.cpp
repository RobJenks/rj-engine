#include "NoiseGenerator.h"
#include "Utility.h"
#include "Logging.h"
#include "TextureDX11.h"
#include "ConstantBufferDX11.h"
#include "Random.h"

// Constructor
NoiseGenerator::NoiseGenerator(void)
	:
	m_active_resource(NULL), 
	m_random_dist(Random::UniformIntegralDistribution<unsigned int>())
{
}

// Store a new resource.  Returns a flag indicating whether the resource was accepted
bool NoiseGenerator::AddResource(const std::string & code, const TextureDX11 *resource)
{
	// Parameter validation
	if (code == NullString) { Game::Log << LOG_ERROR << "Cannot add new noise resource; no unique code specified\n"; return false; }
	if (resource == NULL) { Game::Log << LOG_ERROR << "Cannot add new noise resource; null texture resource supplied\n"; return false; }
	if (m_resources.find(code) != m_resources.end())
	{
		Game::Log << LOG_ERROR << "Cannot add new noise resource; resource already exists with code \"" << code << "\"\n";
		return false;
	}

	// Store this new resource
	m_resources[code] = resource;
	Game::Log << LOG_INFO << "Registered new noise generation resource \"" << code << "\"\n";

	// Activate this resource if no noise data is currently bound, so that we always have non-null data active
	if (m_active_resource == NULL)
	{
		Game::Log << LOG_INFO << "Binding noise generation data \"" << code << "\" as first active resource\n";
		BindNoiseResources(code);
	}

	// Return success
	return true;
}

// Request the resource with the given code, or NULL if none exists
const TextureDX11 * NoiseGenerator::GetResource(const std::string & code)
{
	const std::unordered_map<std::string, const TextureDX11*>::const_iterator it = m_resources.find(code);
	if (it != m_resources.end())
	{
		return it->second;
	}

	return NULL;
}

// Bind noise data to the given render pipeline
void NoiseGenerator::BindNoiseResources(const std::string & code)
{
	// Attempt to locate this resource
	const std::unordered_map<std::string, const TextureDX11*>::const_iterator it = m_resources.find(code);
	if (it == m_resources.end()) return;

	// Call the binding function with this texture resource
	BindNoiseResources(it->second);
}

// Bind noise data to the given render pipeline
void NoiseGenerator::BindNoiseResources(const TextureDX11 * resource)
{
	// Store the active texture resource
	m_active_resource = resource;

	// Update the constant buffer
	if (resource)
	{
		m_cb_noise_data.RawPtr->TextureWidth = resource->GetWidth();
		m_cb_noise_data.RawPtr->TextureHeight = resource->GetHeight();
		m_cb_noise_data.RawPtr->TextureDepth = resource->GetDepth();
	}
	else
	{
		m_cb_noise_data.RawPtr->TextureWidth = 0;
		m_cb_noise_data.RawPtr->TextureHeight = 0;
		m_cb_noise_data.RawPtr->TextureDepth = 0;
	}

	m_cb_noise_data.RawPtr->RandomNoiseSeed = XMUINT4(
		Random::GetUniformIntegral(m_random_dist), Random::GetUniformIntegral(m_random_dist), 
		Random::GetUniformIntegral(m_random_dist), Random::GetUniformIntegral(m_random_dist));

	// Compile the buffer
	m_cb_noise->Set(m_cb_noise_data.RawPtr);
}


// Destructor
NoiseGenerator::~NoiseGenerator(void)
{
}
