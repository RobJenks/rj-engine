#pragma once

#include "GameVarsExtern.h"
#include "DX11_Core.h"
#include "InstanceFlags.h"
#include "../Definitions/VertexDefinitions.hlsl.h"

// Structure of a single instance in the instancing model.  No special alignement requirements
struct RM_Instance
{
	// Constant sort key values where we don't want to actually calculate the key (e.g. where the object is a neligible occluder)
	static const RM_SortKey			SORT_KEY_RENDER_FIRST;
	static const RM_SortKey			SORT_KEY_RENDER_LAST;

	// Structure data (loaded from common vertex definitions)
	PerInstanceData

	/* Constructors to define new instances in different rendering scenarios */
	CMPINLINE RM_Instance(void) noexcept 
		:
		Flags(InstanceFlags::DEFAULT_INSTANCE_FLAGS)
	{}
	
	// Unsorted instance; will be sorted near the start of the queue and rendered approximately first (possible adding overdraw)
	CMPINLINE RM_Instance(XMFLOAT4X4 && world) noexcept
		:
		Flags(InstanceFlags::DEFAULT_INSTANCE_FLAGS), 
		Transform(world), 
		LastTransform(world), 
		SortKey(SORT_KEY_RENDER_FIRST)
	{}

	CMPINLINE RM_Instance(XMFLOAT4X4 && world, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		Transform(world),
		LastTransform(world),
		SortKey(SORT_KEY_RENDER_FIRST)
	{}

	CMPINLINE RM_Instance(XMFLOAT4X4 && world, XMFLOAT4X4 && lastworld, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		Transform(world),
		LastTransform(lastworld), 
		SortKey(SORT_KEY_RENDER_FIRST)
	{}

	CMPINLINE RM_Instance(XMFLOAT4X4 && world, RM_SortKey sort_key, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		Transform(world), 
		LastTransform(world), 
		SortKey(sort_key)
	{}

	CMPINLINE RM_Instance(XMFLOAT4X4 && world, XMFLOAT4X4 && lastworld, RM_SortKey sort_key, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		Transform(world),
		LastTransform(lastworld),
		SortKey(sort_key)
	{}

	CMPINLINE RM_Instance(XMFLOAT4X4 && world, RM_SortKey sort_key, const XMFLOAT4 & highlight, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		Transform(world),
		LastTransform(world), 
		SortKey(sort_key), 
		Highlight(highlight)
	{}

	CMPINLINE RM_Instance(XMFLOAT4X4 && world, XMFLOAT4X4 && lastworld, RM_SortKey sort_key, const XMFLOAT4 & highlight, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		Transform(world),
		LastTransform(lastworld),
		SortKey(sort_key),
		Highlight(highlight)
	{}


	CMPINLINE RM_Instance(const FXMMATRIX world) noexcept
		:
		Flags(InstanceFlags::DEFAULT_INSTANCE_FLAGS)
	{
		XMStoreFloat4x4(&Transform, world);
		LastTransform = Transform;
	}

	CMPINLINE RM_Instance(const FXMMATRIX world, InstanceFlags::Type flags) noexcept
		:
		Flags(flags)
	{
		XMStoreFloat4x4(&Transform, world);
		LastTransform = Transform;
	}

	CMPINLINE RM_Instance(const FXMMATRIX world, const CXMMATRIX lastworld, InstanceFlags::Type flags) noexcept
		:
		Flags(flags)
	{
		XMStoreFloat4x4(&Transform, world);
		XMStoreFloat4x4(&LastTransform, lastworld);
	}

	CMPINLINE RM_Instance(const FXMMATRIX world, RM_SortKey sort_key, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		SortKey(sort_key)
	{
		XMStoreFloat4x4(&Transform, world);
		LastTransform = Transform;
	}

	CMPINLINE RM_Instance(const FXMMATRIX world, const CXMMATRIX lastworld, RM_SortKey sort_key, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		SortKey(sort_key)
	{
		XMStoreFloat4x4(&Transform, world);
		XMStoreFloat4x4(&LastTransform, lastworld);
	}

	CMPINLINE RM_Instance(const FXMMATRIX world, RM_SortKey sort_key, const XMFLOAT4 & highlight, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		SortKey(sort_key),
		Highlight(highlight)
	{
		XMStoreFloat4x4(&Transform, world);
		LastTransform = Transform;
	}

	CMPINLINE RM_Instance(const FXMMATRIX world, const CXMMATRIX lastworld, RM_SortKey sort_key, const XMFLOAT4 & highlight, InstanceFlags::Type flags) noexcept
		:
		Flags(flags),
		SortKey(sort_key),
		Highlight(highlight)
	{
		XMStoreFloat4x4(&Transform, world);
		XMStoreFloat4x4(&LastTransform, lastworld);
	}

	// Comparison operator for sorting; instances are sorted in ascending 'SortKey' order
	CMPINLINE bool operator<(const RM_Instance & val) const { return (SortKey < val.SortKey); }

	// Copy constructor
	CMPINLINE RM_Instance(const RM_Instance & other)
		:
		Transform(other.Transform),
		LastTransform(other.LastTransform), 
		Flags(other.Flags),
		SortKey(other.SortKey),
		Highlight(other.Highlight)
	{
	}

	// Copy assignment is disallowed
	CMPINLINE RM_Instance & operator=(const RM_Instance & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than copied by STL containers
	CMPINLINE RM_Instance(RM_Instance && other) noexcept
		:
		Transform(std::move(other.Transform)), 
		LastTransform(std::move(other.LastTransform)), 
		Flags(other.Flags), 
		SortKey(other.SortKey), 
		Highlight(std::move(other.Highlight))
		/* Ignore padding; no need to move */
	{
	}


	// Move assignment
	CMPINLINE RM_Instance & RM_Instance::operator=(RM_Instance && other) noexcept
	{
		Transform = std::move(other.Transform);
		LastTransform = std::move(other.LastTransform);
		Flags = other.Flags;
		SortKey = other.SortKey;
		Highlight = std::move(other.Highlight);
		return *this;
	}

	// Default destructor
	CMPINLINE ~RM_Instance(void) noexcept { }


	// Determine the sort key for an instance based upon relevant data
	static RM_SortKey CalculateSortKey(const FXMVECTOR instance_position);
	static RM_SortKey CalculateSortKey(float distance_sq_to_viewer);

};




