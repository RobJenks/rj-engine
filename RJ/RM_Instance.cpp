#include "GameVarsExtern.h"
#include "CoreEngine.h"
#include "CameraClass.h"

#include "RM_Instance.h"

// Constant sort key values where we don't want to actually calculate the key (e.g. where the object is a neligible occluder)
const RM_Instance::RM_SortKey RM_Instance::SORT_KEY_RENDER_FIRST = 0U;
const RM_Instance::RM_SortKey RM_Instance::SORT_KEY_RENDER_LAST = (0U - 1U);


// Determine the sort key for an instance based upon relevant data
RM_Instance::RM_SortKey RM_Instance::CalculateSortKey(const FXMVECTOR instance_position)
{
	return static_cast<RM_SortKey>(
		XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(Game::Engine->GetCamera()->GetPosition(), instance_position)))
	);
}

// Determine the sort key for an instance based upon relevant data
RM_Instance::RM_SortKey RM_Instance::CalculateSortKey(float distance_sq_to_viewer)
{
	// For now, sort key is simply a truncated integral dist^2 from object to viewer, from near to far
	return static_cast<RM_SortKey>(distance_sq_to_viewer);
}