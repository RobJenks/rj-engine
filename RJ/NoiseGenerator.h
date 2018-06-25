#pragma once

#include <string>
#include <random>
#include <vector>
#include <unordered_map>
#include "ErrorCodes.h"
#include "ManagedPtr.h"
#include "Data/Shaders/noise_buffers.hlsl.h"
class TextureDX11;
class PipelineDX11;
class ConstantBufferDX11;


class NoiseGenerator
{
public:

	// Custom resource ID
	typedef std::vector<const TextureDX11 *>::size_type		NoiseResourceID;
	static const NoiseResourceID							INVALID_NOISE_RESOURCE = (0U - 1U);

	// Constructor
	NoiseGenerator(void);

	// Initialise all owned resources
	Result													Initialise(void);

	// Store a new resource.  Returns a flag indicating whether the resource was accepted
	bool													AddResource(const std::string & code, TextureDX11 *resource);

	// Request the resource with the given code, or NULL if none exists
	TextureDX11 *											GetResource(const std::string & code);
	TextureDX11 *											GetResource(NoiseResourceID id);

	// Return the ID of the resource matching this code, or INVALID_NOISE_RESOURCE if no match exists
	NoiseResourceID											GetResourceID(const std::string & code);

	// Bind the given noise resouce and related metadata
	void													BindNoiseResources(const std::string & code);
	void													BindNoiseResources(NoiseResourceID id);
	void													BindNoiseResources(TextureDX11 * resource);

	// Called per-frame to update the noise data buffers and any resources
	void													BeginFrame(void);

	// Returns a pointer to the active noise resources
	TextureDX11 *											GetActiveNoiseResource(void) const;
	ConstantBufferDX11 *									GetActiveNoiseBuffer(void) const;

	// Destructor
	~NoiseGenerator(void);

private:

	// Compile the noise resource data buffer before it is pushed to the GPU
	void													CompileBuffer(void);

private:

	std::vector<TextureDX11 *>								m_resources;
	std::unordered_map<std::string, NoiseResourceID>		m_resource_ids;
	NoiseResourceID											m_resource_count;

	TextureDX11 *											m_active_resource;			// Noise texture resource that has currently been bound
	std::uniform_int_distribution<unsigned int>				m_random_dist;				// Uniform random distribution for generating seed values

	ManagedPtr<NoiseDataBuffer>								m_cb_noise_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *									m_cb_noise;					// Compiled CB

};