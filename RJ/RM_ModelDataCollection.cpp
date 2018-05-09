#include "ModelBuffer.h"
#include "RenderQueueShaders.h"
#include "RM_ModelDataCollection.h"

// Initial capacity that will be reserved within each model slot in the render queue
const size_t RM_ModelDataCollection::RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION = 128U;


// Assigns data from a shader definition
void RM_ModelDataCollection::SetShaderDetails(const RM_InstancedShaderDetails & shader)
{
	PrimitiveTopology = shader.PrimitiveTopology;
	Flags = shader.Flags;
}