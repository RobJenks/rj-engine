#pragma once

#include <vector>
#include "RM_Instance.h"
#include "RM_InstanceMetadata.h"
class ModelBuffer;

// Instance collection & supporting data for a particular model in the render queue
struct RM_InstanceData 
{
	// Initial instance capacity that will be reserved within each struct 
	static const size_t					RENDER_QUEUE_INITIAL_INSTANCE_SLOT_ALLOCATION;

	// Structure data
	std::vector<RM_Instance>			InstanceData;
	std::vector<RM_InstanceMetadata>	InstanceMetadata;
	std::vector<RM_Instance>::size_type	CurrentInstanceCount;
	std::vector<RM_Instance>::size_type	InstanceCapacity;

	// The number of instances that are shadow-casting in this collection (will be 0 to N-1 in the collection)
	std::vector<RM_Instance>::size_type	ShadowCasterCount;


	// TODO: Change operation of RQ optimiser and timeouts
	int									TimeoutCounter;

	// Constructor
	CMPINLINE							RM_InstanceData(void) noexcept		
		: 
		CurrentInstanceCount(0U), 
		InstanceCapacity(0U), 
		TimeoutCounter(0), 
		ShadowCasterCount(0U)
	{
		InitialiseCapacity(RENDER_QUEUE_INITIAL_INSTANCE_SLOT_ALLOCATION);
	}

	// Extends capacity of the collection (both allocation and initialised elements) to the specified size
	CMPINLINE void						InitialiseCapacity(std::vector<RM_Instance>::size_type initial_capacity)
	{
		assert(initial_capacity > 0U);
		assert(initial_capacity < 1000000U);

		// Increase both the capacity and size of the vector
		InstanceData.reserve(initial_capacity);
		InstanceMetadata.reserve(initial_capacity);
		for (std::vector<RM_Instance>::size_type i = 0U; i < initial_capacity; ++i)
		{
			InstanceData.emplace_back(std::move(RM_Instance()));
			InstanceMetadata.emplace_back(std::move(RM_InstanceMetadata()));
		}

		InstanceCapacity = initial_capacity;
	}

	// Extends size of the collection by one, to allow addition of a new element when we are at the allocated limit
	CMPINLINE void						ExtendCapacity(void)
	{
		InstanceData.emplace_back(std::move(RM_Instance()));
		InstanceMetadata.emplace_back(std::move(RM_InstanceMetadata()));
		++InstanceCapacity;
	}

	// Add a new instance for rendering
	void								NewInstance(RM_Instance && instance, RM_InstanceMetadata && metadata);

	// Reset the instance collection ready for the next frame
	void								Reset(void);

	// Sort instances based on their sort key, in preparation for rendering
	// TODO: Can test whether maintaining a separate sorted index vector {sortkey, instance_index} 
	// and lower_bound inserting into it with each instance is faster than one post-sort.  For now
	// stick with the post-sort since the number of instances should always be relatively low
	void								SortInstances(void);

	// Copy constructor & assignment are disallowed
	CMPINLINE							RM_InstanceData(const RM_InstanceData & other) = delete;
	CMPINLINE							RM_InstanceData & operator=(const RM_InstanceData & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than copied by STL containers
	CMPINLINE							RM_InstanceData(RM_InstanceData && other) noexcept
		:
		InstanceData(std::move(other.InstanceData)), 
		InstanceMetadata(std::move(other.InstanceMetadata)), 
		CurrentInstanceCount(other.CurrentInstanceCount), 
		InstanceCapacity(other.InstanceCapacity), 
		TimeoutCounter(other.TimeoutCounter), 
		ShadowCasterCount(other.ShadowCasterCount)
	{
	}

	// Move assignment
	CMPINLINE RM_InstanceData &			RM_InstanceData::operator=(RM_InstanceData && other) noexcept
	{
		InstanceData = std::move(other.InstanceData);
		InstanceMetadata = std::move(other.InstanceMetadata);
		CurrentInstanceCount = other.CurrentInstanceCount;
		InstanceCapacity = other.InstanceCapacity;
		TimeoutCounter = other.TimeoutCounter;
		ShadowCasterCount = other.ShadowCasterCount;
		return *this;
	}

	// Default destructor
	CMPINLINE ~RM_InstanceData(void) noexcept { }
};

