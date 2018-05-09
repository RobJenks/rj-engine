#pragma once

#include <vector>
#include "Utility.h"
#include "ShaderFlags.h"
#include "RM_ModelData.h"
class ModelBuffer;
struct RM_InstancedShaderDetails;


// Collection holding a vector of models to be rendered by the current shader
struct RM_ModelDataCollection
{
	// Initial capacity that will be reserved within each model slot in the render queue
	static const size_t RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION;

	// Structure data
	std::vector<RM_ModelData>					ModelData;
	std::vector<RM_ModelData>::size_type		CurrentSlotCount;
	std::vector<RM_ModelData>::size_type		SlotCapacity;
	D3D_PRIMITIVE_TOPOLOGY						PrimitiveTopology;
	ShaderFlags									Flags;

	// Default constructor
	CMPINLINE RM_ModelDataCollection(void) noexcept 
		: 
		CurrentSlotCount(0U), SlotCapacity(0U), PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST), Flags(0U)
	{
		InitialiseCapacity(RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION);
	}

	// Initialises capacity of the collection (both allocation and initialised elements) to the specified size
	CMPINLINE void								InitialiseCapacity(std::vector<RM_ModelData>::size_type initial_capacity)
	{
		assert(initial_capacity > 0U);
		assert(initial_capacity < 1000000U);

		// Pre-allocate space and initialise all starting elements
		ModelData.reserve(initial_capacity);
		for (std::vector<RM_ModelData>::size_type i = 0U; i < initial_capacity; ++i)
		{
			ModelData.emplace_back(std::move(RM_ModelData()));
		}

		SlotCapacity = initial_capacity;
	}

	// Extends size of the collection by one, to allow addition of a new element when we are at the allocated limit
	CMPINLINE void ExtendCapacity(void)
	{
		ModelData.emplace_back(std::move(RM_ModelData()));
		++SlotCapacity;
	}

	// Returns the next available render slot, allocating space if required
	CMPINLINE size_t							NewRenderSlot(void)
	{
		if (CurrentSlotCount == SlotCapacity) ExtendCapacity();
		return (CurrentSlotCount++);
	}

	// Indicates whether the data collection contains any data
	CMPINLINE bool								Empty(void) const { return (CurrentSlotCount == 0U); }

	// Copy constructor & assignment are disallowed
	CMPINLINE									RM_ModelDataCollection(const RM_ModelDataCollection & other) = delete;
	CMPINLINE									RM_ModelDataCollection & operator=(const RM_ModelDataCollection & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than copied by STL containers
	CMPINLINE									RM_ModelDataCollection(RM_ModelDataCollection && other) noexcept
		: 
		ModelData(std::move(other.ModelData)), 
		CurrentSlotCount(other.CurrentSlotCount), 
		SlotCapacity(other.SlotCapacity), 
		Flags(other.Flags)
	{
	}

	// Move assignment
	CMPINLINE RM_ModelDataCollection &			RM_ModelDataCollection::operator=(RM_ModelDataCollection && other) noexcept
	{
		ModelData = std::move(other.ModelData);
		CurrentSlotCount = other.CurrentSlotCount;
		SlotCapacity = other.SlotCapacity;
		Flags = other.Flags;
		return *this;
	}

	// Assigns data from a shader definition
	void SetShaderDetails(const RM_InstancedShaderDetails & shader);


	// Default destructor
	CMPINLINE ~RM_ModelDataCollection(void) noexcept { }
};


