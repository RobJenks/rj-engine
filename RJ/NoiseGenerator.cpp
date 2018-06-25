#include "NoiseGenerator.h"
#include "Utility.h"
#include "Logging.h"
#include "TextureDX11.h"
#include "ConstantBufferDX11.h"
#include "Random.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"

// Constructor
NoiseGenerator::NoiseGenerator(void)
	:
	m_active_resource(NULL), 
	m_resource_count(0U), 
	m_cb_noise(NULL), 
	m_random_dist(Random::UniformIntegralDistribution<unsigned int>())
{
}

// Initialise all owned resources
Result NoiseGenerator::Initialise(void)
{
	Game::Log << LOG_INFO << "Initialising rendering noise generator\n";

	Game::Log << LOG_INFO << "Initialising noise generation buffer resources\n";
	m_cb_noise = Game::Engine->GetAssets().CreateConstantBuffer<NoiseDataBuffer>(NoiseDataBufferName, m_cb_noise_data.RawPtr);
	if (!m_cb_noise)
	{
		Game::Log << LOG_ERROR << "Failed to initialise noise generation buffer resource \"" << NoiseDataBufferName << "\"\n";
		return ErrorCodes::FailedToInitialiseNoiseGenerator;
	}

	return ErrorCodes::NoError;
}

// Store a new resource.  Returns a flag indicating whether the resource was accepted
bool NoiseGenerator::AddResource(const std::string & code, TextureDX11 *resource)
{
	// Parameter validation
	if (code == NullString) { Game::Log << LOG_ERROR << "Cannot add new noise resource; no unique code specified\n"; return false; }
	if (resource == NULL) { Game::Log << LOG_ERROR << "Cannot add new noise resource; null texture resource supplied\n"; return false; }
	if (m_resource_ids.find(code) != m_resource_ids.end())
	{
		Game::Log << LOG_ERROR << "Cannot add new noise resource; resource already exists with code \"" << code << "\"\n";
		return false;
	}

	// Store this new resource
	NoiseResourceID id = m_resources.size();
	m_resources.push_back(resource);
	m_resource_ids[code] = id;
	m_resource_count = m_resources.size();
	Game::Log << LOG_INFO << "Registered new noise generation resource \"" << code << "\" (" << id << ")\n";

	// Activate this resource if no noise data is currently bound, so that we always have non-null data active
	if (m_active_resource == NULL)
	{
		Game::Log << LOG_INFO << "Binding noise generation data \"" << code << "\" as first active resource\n";
		BindNoiseResources(code);
	}

	// Return success
	return true;
}

// Return the ID of the resource matching this code, or INVALID_NOISE_RESOURCE if no match exists
NoiseGenerator::NoiseResourceID NoiseGenerator::GetResourceID(const std::string & code)
{
	const std::unordered_map<std::string, NoiseResourceID>::const_iterator it = m_resource_ids.find(code);
	if (it != m_resource_ids.end())
	{
		return it->second;
	}

	return NoiseGenerator::INVALID_NOISE_RESOURCE;
}

// Request the resource with the given code, or NULL if none exists
TextureDX11 * NoiseGenerator::GetResource(const std::string & code)
{
	return GetResource(GetResourceID(code));
}

TextureDX11 * NoiseGenerator::GetResource(NoiseGenerator::NoiseResourceID id)
{
	return (id < m_resource_count ? m_resources[id] : NULL);
}

// Bind noise data to the given render pipeline
void NoiseGenerator::BindNoiseResources(const std::string & code)
{
	// Attempt to locate this resource
	const std::unordered_map<std::string, NoiseResourceID>::const_iterator it = m_resource_ids.find(code);
	if (it == m_resource_ids.end()) return;

	// Call the binding function with this texture resource
	BindNoiseResources(m_resources[it->second]);
}

void NoiseGenerator::BindNoiseResources(NoiseGenerator::NoiseResourceID id)
{
	BindNoiseResources(GetResource(id));
}

// Bind noise data to the given render pipeline
void NoiseGenerator::BindNoiseResources(TextureDX11 * resource)
{
	// No action to take if this is already the active resource
	if (resource == m_active_resource) return;

	// Store the active texture resource
	m_active_resource = resource;

	// Update the constant buffer with any resource-dependent data
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

	// Compile the buffer to include this data immediately (alongside per-frame data
	// already present in the buffer)
	CompileBuffer();
}

// Called per-frame to update the noise data buffers and any resources
void NoiseGenerator::BeginFrame(void)
{
	// Update any per-frame data; random noise seed is regenerated for each frame
	m_cb_noise_data.RawPtr->RandomNoiseSeed = XMUINT4(
		Random::GetUniformIntegral(m_random_dist), Random::GetUniformIntegral(m_random_dist),
		Random::GetUniformIntegral(m_random_dist), Random::GetUniformIntegral(m_random_dist));

	// Compile the buffer to include this data immediately (alongside resource-dependent data
	// already in the buffer)
	CompileBuffer();
}

// Compile the noise resource data buffer before it is pushed to the GPU
void NoiseGenerator::CompileBuffer(void)
{
	m_cb_noise->Set(m_cb_noise_data.RawPtr);
}

// Returns a pointer to the active noise resources
TextureDX11 * NoiseGenerator::GetActiveNoiseResource(void) const
{
	return m_active_resource;
}

// Returns a pointer to the active noise resources
ConstantBufferDX11 * NoiseGenerator::GetActiveNoiseBuffer(void) const
{
	return m_cb_noise;
}

// Destructor
NoiseGenerator::~NoiseGenerator(void)
{
}



