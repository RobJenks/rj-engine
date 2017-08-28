#include "ModelBuffer.h"
#include "RM_ModelDataCollection.h"

// Initial capacity that will be reserved within each model slot in the render queue
const size_t RM_ModelDataCollection::RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION = 128U;

// Assign an instance to the given model buffer
void RM_ModelDataCollection::RegisterModelBuffer(size_t shader, size_t slot, ModelBuffer *model)
{
	assert(model != NULL);

	model->AssignRenderSlot(shader, slot);
	ModelData[slot].ModelBufferInstance = model;
	ModelData[slot].CurrentInstanceCount = 0U;	
}

// Clears the specified model data following a successful render, unregistering any 
// relevant model buffer data and returning to a state ready for next frame
void RM_ModelDataCollection::UnregisterModelBuffer(size_t shader, size_t slot)
{
	assert(ModelData[slot].ModelBufferInstance != NULL);

	ModelData[slot].ModelBufferInstance->ClearRenderSlot(shader);
	ModelData[slot].ModelBufferInstance = NULL;
	ModelData[slot].CurrentInstanceCount = 0U;
}
