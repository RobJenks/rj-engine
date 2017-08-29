#pragma once

#include "GameVarsExtern.h"
#include "DX11_Core.h"

// Structure of a single instance in the instancing model.  No special alignement requirements
struct RM_Instance
{
	// Key type used for sorting items upon submission to the render queue
	typedef UINT64					RM_SortKey;

	// Constant sort key values where we don't want to actually calculate the key (e.g. where the object is a neligible occluder)
	static const RM_SortKey			SORT_KEY_RENDER_FIRST;
	static const RM_SortKey			SORT_KEY_RENDER_LAST;

	// Structure data
	XMFLOAT4X4						World;								// World matrix to transform into the world
	XMFLOAT4						Params;								// Float-4 of parameters that can be passed for each instance
	Game::LIGHT_CONFIG				LightConfig;						// Configuration of lights that should be used to render this instance
	RM_SortKey						SortKey;							// Used to sort instances upon submission to the render queue
	XMFLOAT2						padding;							// (Padding - brings total size to 96, 96 % 16 == 0)

	
	// Constructor where only the world transform and lighting data are required; other params will be unitialised (for efficiency) and should not be used
	CMPINLINE RM_Instance(const XMFLOAT4X4 & world, Game::LIGHT_CONFIG lighting, RM_SortKey sort_key) noexcept 
		: 
		World(world), 
		LightConfig(lighting), 
		SortKey(sort_key)
	{ 
	}

	// Constructor where only the world transform and lighting data are required; other params will be unitialised (for efficiency) and should not be used
	CMPINLINE RM_Instance(const CXMMATRIX world, Game::LIGHT_CONFIG lighting, RM_SortKey sort_key) noexcept
		:
		LightConfig(lighting),
		SortKey(sort_key)
	{
		XMStoreFloat4x4(&World, world);
	}

	// Constructor including additional per-instance parameters
	CMPINLINE RM_Instance(const XMFLOAT4X4 world, Game::LIGHT_CONFIG lighting, const XMFLOAT4 & params, RM_SortKey sort_key) noexcept 
		: 
		World(world), 
		LightConfig(lighting), 
		Params(params), 
		SortKey(sort_key)
	{ 
	}

	// Constructor including additional per-instance parameters
	CMPINLINE RM_Instance(const CXMMATRIX world, Game::LIGHT_CONFIG lighting, const XMFLOAT4 & params, RM_SortKey sort_key) noexcept
		:
		LightConfig(lighting), 
		Params(params), 
		SortKey(sort_key)
	{
		XMStoreFloat4x4(&World, world);
	}

	// Empty constructor
	CMPINLINE RM_Instance(void) noexcept 
	{ 
	}

	// Comparison operator for sorting; instances are sorted in ascending 'SortKey' order
	CMPINLINE bool operator<(const RM_Instance & val) const { return (SortKey < val.SortKey); }

	// Copy constructor & assignment are disallowed
	CMPINLINE										RM_Instance(const RM_Instance & other) = delete;
	CMPINLINE										RM_Instance & operator=(const RM_Instance & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than copied by STL containers
	CMPINLINE RM_Instance(RM_Instance && other) noexcept
		:
		World(other.World), 
		Params(other.Params), 
		LightConfig(other.LightConfig), 
		SortKey(other.SortKey)
		/* Ignore padding; no need to move */
	{
	}


	// Move assignment
	CMPINLINE RM_Instance & RM_Instance::operator=(RM_Instance && other) noexcept
	{
		World = other.World;
		Params = other.Params;
		LightConfig = other.LightConfig;
		SortKey = other.SortKey;
		return *this;
	}

	// Default destructor
	CMPINLINE ~RM_Instance(void) noexcept { }


	// Determine the sort key for an instance based upon relevant data
	static RM_SortKey CalculateSortKey(const FXMVECTOR instance_position);
	static RM_SortKey CalculateSortKey(float distance_sq_to_viewer);

};




