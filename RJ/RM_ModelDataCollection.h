#pragma once

#include <vector>
#include "RM_InstanceData.h"
class ModelBuffer;


// Collection holding a vector of models to be rendered by the current shader
struct RM_ModelDataCollection
{
	// Initial capacity that will be reserved within each model slot in the render queue
	static const size_t RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION;

	// Structure data
	std::vector<RM_InstanceData>				ModelData;
	std::vector<RM_InstanceData>::size_type		CurrentSlotCount;
	std::vector<RM_InstanceData>::size_type		SlotCapacity;


	// Default constructor
	CMPINLINE RM_ModelDataCollection(void) noexcept 
		: 
		CurrentSlotCount(0U), SlotCapacity(0U)
	{
		InitialiseCapacity(RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION);
	}

	// Initialises capacity of the collection (both allocation and initialised elements) to the specified size
	CMPINLINE void								InitialiseCapacity(std::vector<RM_InstanceData>::size_type initial_capacity)
	{
		assert(initial_capacity > 0U);
		assert(initial_capacity < 1000000U);

		// Pre-allocate space and initialse all starting elements
		ModelData.reserve(initial_capacity);
		for (std::vector<RM_InstanceData>::size_type i = 0U; i < initial_capacity; ++i)
		{
			ModelData.emplace_back(std::move(RM_InstanceData()));
		}

		SlotCapacity = initial_capacity;
	}

	// Extends size of the collection by one, to allow addition of a new element when we are at the allocated limit
	CMPINLINE void ExtendCapacity(void)
	{
		ModelData.emplace_back(std::move(RM_InstanceData()));
		++SlotCapacity;
	}

	// Returns the next available render slot, allocating space if required
	CMPINLINE size_t							NewRenderSlot(void)
	{
		if (CurrentSlotCount == SlotCapacity) ExtendCapacity();
		return (CurrentSlotCount++);
	}


	// Assign this instance to the given model buffer
	void										RegisterModelBuffer(size_t shader, size_t slot, ModelBuffer *model);

	// Clears the specified model data following a successful render, unregistering any 
	// relevant model buffer data and returning to a state ready for next frame
	void										UnregisterModelBuffer(size_t shader, size_t slot);

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
		SlotCapacity(other.SlotCapacity)
	{
	}

	// Move assignment
	CMPINLINE RM_ModelDataCollection &			RM_ModelDataCollection::operator=(RM_ModelDataCollection && other) noexcept
	{
		ModelData = std::move(other.ModelData);
		CurrentSlotCount = other.CurrentSlotCount;
		SlotCapacity = other.SlotCapacity;
		return *this;
	}


	// Default destructor
	CMPINLINE ~RM_ModelDataCollection(void) noexcept { }
};


