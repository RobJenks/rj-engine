#pragma once

#include <vector>
#include "RM_InstanceData.h"
#include "RM_MaterialInstanceMapping.h"
class ModelBufferInstance;
class MaterialDX11;

struct RM_ModelData
{
	// Model buffer to be rendered
	ModelBuffer *										ModelBufferInstance;

	// We maintain a set of instances per material, for a given model buffer
	typedef std::vector<RM_MaterialInstanceMapping>		MaterialInstanceData;
	MaterialInstanceData								Data;

	// Count of currently-active material slots
	MaterialInstanceData::size_type						CurrentMaterialCount;


	// Constructor; make sure the inital (default material) slot is always available
	CMPINLINE RM_ModelData(void)
		:
		ModelBufferInstance(NULL), 
		CurrentMaterialCount(1U)		// Set to 1 since the default material slot always remains open
	{
		Data.push_back(RM_MaterialInstanceMapping(NULL));
	}



	// Return the instance collection for the default model material
	CMPINLINE RM_InstanceData & GetInstances(void)
	{
		return Data[0U].InstanceCollection;
	}

	// Return the instance collection for the given material
	RM_InstanceData & GetInstances(MaterialDX11 *material);

	// Return the number of materials currently registered for this model
	CMPINLINE MaterialInstanceData::size_type GetMaterialCount(void) const { return CurrentMaterialCount; }

	// Reset the per-material data back to a default state, ready for the next frame
	void Reset(void);

	// Set the model buffer that will be assigned to this set of instances
	void AssignModel(ModelBuffer *model);

	// Set the material that will be used for all default instances (i.e. those with no material explicitly specified)
	void SetDefaultMaterial(const MaterialDX11 *material);
	
	// Clear the model buffer assigment for these instances
	void ClearModel(void);


	// TODO: Render queue optimiser can process these objects by clearing Data vector down to only element [0] if 
	// required.  It should always retain [0] since this is the slot reserved for the default & unspecified material
	// This should be implemented since otherwise slots will remain open indefinitely and slowly waste space


	// Copy construction and assignment are disallowed
	CMPINLINE RM_ModelData(const RM_ModelData & other) = delete;
	CMPINLINE RM_ModelData & operator=(const RM_ModelData & other) = delete;

	// Move constructor
	CMPINLINE RM_ModelData(RM_ModelData && other)
		:
		ModelBufferInstance(other.ModelBufferInstance), 
		Data(std::move(other.Data)), 
		CurrentMaterialCount(other.CurrentMaterialCount)
	{
	}

	// Move assignment
	CMPINLINE RM_ModelData & operator=(RM_ModelData && other)
	{
		ModelBufferInstance = other.ModelBufferInstance;
		Data = std::move(other.Data);
		CurrentMaterialCount = other.CurrentMaterialCount;
	}

	// Destructor
	CMPINLINE ~RM_ModelData(void)
	{
	}



};