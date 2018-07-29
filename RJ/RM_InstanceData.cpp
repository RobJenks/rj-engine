#include "ModelBuffer.h"
#include "RM_InstanceData.h"


// Initial instance capacity that will be reserved within each struct; keep relatively low to avoid escalating
// memory requirements, but still reducing thrash of vector reallocation at low volumes
const size_t RM_InstanceData::RENDER_QUEUE_INITIAL_INSTANCE_SLOT_ALLOCATION = 32U;


// Add a new instance for rendering
void RM_InstanceData::NewInstance(RM_Instance && instance, RM_InstanceMetadata && metadata)
{
	if (CurrentInstanceCount == InstanceCapacity) ExtendCapacity();

	if (CheckBit_Single(instance.Flags, RM_Instance::INSTANCE_FLAG_SHADOW_CASTER))
	{
		++ShadowCasterCount;		// TODO: can probably become a flag, not a count
	}

	InstanceData[CurrentInstanceCount] = std::move(instance);	
	InstanceMetadata[CurrentInstanceCount] = std::move(metadata);

	++CurrentInstanceCount;
}

// Sort instances based on their sort key, in preparation for rendering
// TODO: Can test whether maintaining a separate sorted index vector {sortkey, instance_index} 
// and lower_bound inserting into it with each instance is faster than one post-sort.  For now
// stick with the post-sort since the number of instances should always be relatively low
void RM_InstanceData::SortInstances(void)
{
	// Sort only in the range [begin(), begin()+count) rather than to end(), since we
	// are only using a portion of the vector and leaving the rest allocated between frames
	//std::sort(InstanceData.begin(), InstanceData.begin() + CurrentInstanceCount);

	// TODO: Sort method is not currently used.  Will require different mechanism if this is re-enabled in order
	// to keep instance data and metadata in sync
}

// Reset the instance collection ready for the next frame
void RM_InstanceData::Reset(void)
{
	CurrentInstanceCount = 0U;
	ShadowCasterCount = 0U;
}