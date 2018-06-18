#pragma once

#include <string>
#include <unordered_map>
class TextureDX11;
class PipelineDX11;


class NoiseGenerator
{
public:

	// Constructor
	NoiseGenerator(void);

	// Store a new resource.  Returns a flag indicating whether the resource was accepted
	bool AddResource(const std::string & code, const TextureDX11 *resource);

	// Request the resource with the given code, or NULL if none exists
	const TextureDX11 *GetResource(const std::string & code);

	// Bind noise data to the given render pipeline
	void BindNoiseResource(const std::string & code, PipelineDX11 *pipeline);
	void BindNoiseResource(const TextureDX11 * code, PipelineDX11 *pipeline);


	// Destructor
	~NoiseGenerator(void);

private:

	std::unordered_map<std::string, const TextureDX11*>		m_resources;


};