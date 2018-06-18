#include "NoiseGenerator.h"
#include "Utility.h"
#include "Logging.h"

// Constructor
NoiseGenerator::NoiseGenerator(void)
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

	// Store this new resource and return success
	m_resources[code] = resource;
	Game::Log << LOG_INFO << "Registered new noise generation resource \"" << code << "\"\n";
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
void NoiseGenerator::BindNoiseResource(const std::string & code, PipelineDX11 *pipeline)
{
	// Attempt to locate this resource
	const std::unordered_map<std::string, const TextureDX11*>::const_iterator it = m_resources.find(code);
	if (it == m_resources.end()) return;

	// Call the binding function with this texture resource
	BindNoiseResource(it->second, pipeline);
}

// Bind noise data to the given render pipeline
void NoiseGenerator::BindNoiseResource(const TextureDX11 * code, PipelineDX11 *pipeline)
{
	*** DO THIS; NEED TO BIND TEXTURE RESOURCE AND ALSO POPULATE CBUFFER WITH { WIDTH, HEIGHT, DEPTH, RANDOMSEED } ***
}


// Destructor
NoiseGenerator::~NoiseGenerator(void)
{
}
