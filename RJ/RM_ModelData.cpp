#include "RM_ModelData.h"
#include "ModelBuffer.h"


// Return the instance collection for the given material
RM_InstanceData & RM_ModelData::GetInstances(MaterialDX11 *material)
{
	/* A null material should always resolve to the default material specified in the model buffer */
	if (!material) return Data[0U].InstanceCollection;

	/* Check whether we already have an instance collection for this material */
	// This is linear search, however the majority of cases should be the default model material (i.e. the early-
	// exit) and there should only ever be a very small number of alternative materials.  Vector is likely going
	// to be fastest and most cache-coherent in all currently forseeable use cases
	for (MaterialInstanceData::size_type i = 0U; i < CurrentMaterialCount; ++i)
	{
		if (Data[i].Material == material) return Data[i].InstanceCollection;
	}


	/* This material has not been registered yet; create a new entry and return a reference to it */
	if (CurrentMaterialCount < (Data.size() - 1U))
	{
		// We already have an allocated slot that we can reuse
		auto & data = Data[CurrentMaterialCount];
		data.Material = material;
		data.InstanceCollection.Reset();
		
		// Increment the active material count
		++CurrentMaterialCount;

		// Return a reference to the instance collection]
		return data.InstanceCollection;
	}
	else
	{
		// We need to create a new slot
		Data.push_back(RM_MaterialInstanceMapping(material));
		return Data[Data.size() - 1U].InstanceCollection;
	}
}

// Reset the per-material data back to a default state, ready for the next frame
void RM_ModelData::Reset(void)
{
	// Reset to 1 since the default material slot always remains open at [0]
	CurrentMaterialCount = 1U;

	// Reset the instance count for this remaining entry to zero
	Data[0U].InstanceCollection.Reset();
}

// Set the model buffer that will be assigned to this set of instances
void RM_ModelData::AssignModel(ModelBuffer *model)
{
	ModelBufferInstance = model;
	if (model)
	{
		SetDefaultMaterial(model->Material);
	}
}

// Set the material that will be used for all default instances (i.e. those with no material explicitly specified)
void RM_ModelData::SetDefaultMaterial(const MaterialDX11 *material)
{
	Data[0].Material = material;
}

// Clear the model buffer assigment for these instances
void RM_ModelData::ClearModel(void)
{
	ModelBufferInstance = NULL;
	Data[0].Material = NULL;
}