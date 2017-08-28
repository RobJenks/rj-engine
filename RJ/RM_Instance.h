#pragma once

#include "GameVarsExtern.h"
#include "DX11_Core.h"

// Structure of a single instance in the instancing model.  No special alignement requirements
struct RM_Instance
{
	XMFLOAT4X4						World;								// World matrix to transform into the world
	XMFLOAT4						Params;								// Float-4 of parameters that can be passed for each instance
	Game::LIGHT_CONFIG				LightConfig;						// Configuration of lights that should be used to render this instance
	XMFLOAT3						padding;							// (Padding - brings total size to 96, 96 % 16 == 0)

	
	// Constructor where only the world transform and lighting data are required; other params will be unitialised (for efficiency) and should not be used
	CMPINLINE RM_Instance(const XMFLOAT4X4 & world, Game::LIGHT_CONFIG lighting) noexcept 
		: 
		World(world), 
		LightConfig(lighting) 
	{ 
	}

	// Constructor where only the world transform and lighting data are required; other params will be unitialised (for efficiency) and should not be used
	CMPINLINE RM_Instance(const CXMMATRIX world, Game::LIGHT_CONFIG lighting) noexcept
		:
		LightConfig(lighting)
	{
		XMStoreFloat4x4(&World, world);
	}

	// Constructor including additional per-instance parameters
	CMPINLINE RM_Instance(const XMFLOAT4X4 world, Game::LIGHT_CONFIG lighting, const XMFLOAT4 & params) noexcept 
		: 
		World(world), 
		LightConfig(lighting), 
		Params(params) 
	{ 
	}

	// Constructor including additional per-instance parameters
	CMPINLINE RM_Instance(const CXMMATRIX world, Game::LIGHT_CONFIG lighting, const XMFLOAT4 & params) noexcept
		:
		LightConfig(lighting), 
		Params(params)
	{
		XMStoreFloat4x4(&World, world);
	}

	// Empty constructor
	CMPINLINE RM_Instance(void) noexcept 
	{ 
	}

	// Copy constructor & assignment are disallowed
	CMPINLINE										RM_Instance(const RM_Instance & other) = delete;
	CMPINLINE										RM_Instance & operator=(const RM_Instance & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than copied by STL containers
	CMPINLINE RM_Instance(RM_Instance && other) noexcept
		:
		World(other.World), 
		Params(other.Params), 
		LightConfig(other.LightConfig) 
		/* Ignore padding; no need to move */
	{
	}


	// Move assignment
	CMPINLINE RM_Instance & RM_Instance::operator=(RM_Instance && other) noexcept
	{
		World = other.World;
		Params = other.Params;
		LightConfig = other.LightConfig;
		return *this;
	}

	// Default destructor
	CMPINLINE ~RM_Instance(void) noexcept { }

};
