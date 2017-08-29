#include "ModelBuffer.h"
#include "RM_ModelDataCollection.h"

// Initial capacity that will be reserved within each model slot in the render queue
const size_t RM_ModelDataCollection::RENDER_QUEUE_INITIAL_MODEL_SLOT_ALLOCATION = 128U;
