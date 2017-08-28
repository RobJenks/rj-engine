#include "ModelBuffer.h"
#include "RM_InstanceData.h"


// Initial instance capacity that will be reserved within each struct; keep relatively low to avoid escalating
// memory requirements, but still reducing thrash of vector reallocation at low volumes
const size_t RM_InstanceData::RENDER_QUEUE_INITIAL_INSTANCE_SLOT_ALLOCATION = 32U;


// Add a new instance for rendering
void RM_InstanceData::NewInstance(RM_Instance && instance)
{
	if (CurrentInstanceCount == InstanceCapacity) ExtendCapacity();

	InstanceData[CurrentInstanceCount] = std::move(instance);
	++CurrentInstanceCount;
}



