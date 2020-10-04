#include <DirectXMath.h>
#include "ModelSizeProperties.h"
#include "ModelData.h"
using namespace DirectX;


ModelSizeProperties::ModelSizeProperties(void)
	:
	ModelSizeProperties(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f))
{
}

ModelSizeProperties::ModelSizeProperties(const XMFLOAT3 & minbounds, const XMFLOAT3 & maxbounds, const XMFLOAT3 & modelsize, const XMFLOAT3 & centrepoint)
	:
	ModelSizeProperties(minbounds, maxbounds)
{	
	// Override for derived values
	ModelSize = modelsize;						
	CentrePoint = centrepoint;					
}

ModelSizeProperties::ModelSizeProperties(const XMFLOAT3 & minbounds, const XMFLOAT3 & maxbounds)
	:
	MinBounds(minbounds), 
	MaxBounds(maxbounds)
{
	// Sanity check
	if (MinBounds.x > MaxBounds.x || MinBounds.y > MaxBounds.y || MinBounds.z > MaxBounds.z)
	{
		MinBounds = XMFLOAT3(0.0f, 0.0f, 0.0f);
		MaxBounds = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	RecalculateDerivedData();
}

void ModelSizeProperties::RecalculateDerivedData(void)
{
	ModelSize = XMFLOAT3(MaxBounds.x - MinBounds.x, MaxBounds.y - MinBounds.y, MaxBounds.z - MinBounds.z);
	CentrePoint = XMFLOAT3(MinBounds.x + (ModelSize.x * 0.5f), MinBounds.y + (ModelSize.y * 0.5f), MinBounds.z + (ModelSize.z * 0.5f));
}

ModelSizeProperties	ModelSizeProperties::Calculate(const ModelData & model)
{
	// Determine minimum and maximum vertex bounds
	XMFLOAT3 min_bounds = XMFLOAT3(+1e6, +1e6, +1e6);
	XMFLOAT3 max_bounds = XMFLOAT3(-1e6, -1e6, -1e6);
	for (unsigned int i = 0U; i < model.VertexCount; ++i)
	{
		const auto & p = model.VertexData[i].position;
		min_bounds = XMFLOAT3(fmin(min_bounds.x, p.x), fmin(min_bounds.y, p.y), fmin(min_bounds.z, p.z));
		max_bounds = XMFLOAT3(fmax(max_bounds.x, p.x), fmax(max_bounds.y, p.y), fmax(max_bounds.z, p.z));
	}

	// Update other derived data
	return ModelSizeProperties(min_bounds, max_bounds);
}

ModelSizeProperties ModelSizeProperties::Calculate(const std::vector<std::unique_ptr<ModelData>> & models, bool recalculate_individual_models)
{
	std::vector<ModelSizeProperties> properties;

	for (const auto & entry : models)
	{
		const auto model = entry.get();
		if (!model) continue;

		// Recalculate individual model size properties based on vertex data if flag is set
		ModelSizeProperties model_properties = (recalculate_individual_models ?
			ModelSizeProperties::Calculate(*model) :								// Either recalculate from vertex data
			model->SizeProperties);													// or using existing metadata

		properties.push_back(model_properties);
	}

	return ModelSizeProperties::Calculate(properties);
}

ModelSizeProperties ModelSizeProperties::Calculate(const std::vector<ModelSizeProperties> & properties)
{
	XMFLOAT3 minbounds = XMFLOAT3(+1e6, +1e6, +1e6);
	XMFLOAT3 maxbounds = XMFLOAT3(-1e6, -1e6, -1e6);

	for (const auto & prop : properties)
	{
		const auto & modelmin = prop.MinBounds;
		const auto & modelmax = prop.MaxBounds;

		minbounds = XMFLOAT3(min(minbounds.x, modelmin.x), min(minbounds.y, modelmin.y), min(minbounds.z, modelmin.z));
		maxbounds = XMFLOAT3(max(maxbounds.x, modelmax.x), max(maxbounds.y, modelmax.y), max(maxbounds.z, modelmax.z));
	}

	return ModelSizeProperties(minbounds, maxbounds);
}

bool ModelSizeProperties::HasData(void) const
{
	return (MinBounds.x != MaxBounds.x || MinBounds.y != MaxBounds.y || MinBounds.z != MaxBounds.z);
}



