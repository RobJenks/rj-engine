#include "ModelBuffer.h"
#include "RenderQueue.h"


// Constructor that will pre-initialise the given number of elements
RenderQueue::RenderQueue(const size_type count)
	:
	std::vector<RM_ModelDataCollection>(count)
{
}

// Assign an instance to the given model buffer
void RenderQueue::RegisterModelBuffer(size_t shader, size_t slot, ModelBuffer *model)
{
	assert(model != NULL);

	model->AssignRenderSlot(shader, slot);
	Get(shader).ModelData[slot].AssignModel(model);
}

// Clears the specified model data following a successful render, unregistering any 
// relevant model buffer data and returning to a state ready for next frame
void RenderQueue::UnregisterModelBuffer(size_t shader, size_t slot)
{
	assert(Get(shader).ModelData[slot].ModelBufferInstance != NULL);

	Get(shader).ModelData[slot].ModelBufferInstance->ClearRenderSlot(shader);
	Get(shader).ModelData[slot].ClearModel();
}
