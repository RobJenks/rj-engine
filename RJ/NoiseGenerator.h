#pragma once

#include <string>
#include <random>
#include <unordered_map>
#include "ManagedPtr.h"
#include "Data/Shaders/noise_buffers.hlsl.h"
class TextureDX11;
class PipelineDX11;
class ConstantBufferDX11;


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
	void BindNoiseResources(const std::string & code);
	void BindNoiseResources(const TextureDX11 * resource);

	// Called per-frame to update the noise data buffers and any resources


	// Destructor
	~NoiseGenerator(void);

private:

	std::unordered_map<std::string, const TextureDX11*>		m_resources;

	const TextureDX11 *										m_active_resource;			// Noise texture resource that has currently been bound
	std::uniform_int_distribution<unsigned int>				m_random_dist;				// Uniform random distribution for generating seed values

	ManagedPtr<NoiseDataBuffer>								m_cb_noise_data;			// Raw CB data & responsible for deallocation
	ConstantBufferDX11 *									m_cb_noise;					// Compiled CB

};