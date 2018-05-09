#include <iostream>
#include "PipelineStageAssimpTransform.h"
#include "PipelineUtil.h"
#include "ObjFormatIntegration.h"
#include "InputTransformerAssimp.h"


PipelineStageAssimpTransform::PipelineStageAssimpTransform(unsigned int operations)
	:
	m_operations(operations)
{
}

std::unique_ptr<ModelData> PipelineStageAssimpTransform::Transform(std::unique_ptr<ModelData> model) const
{
	ModelData *m = model.get();
	if (!m) return model;

	// Convert RJM to OBJ format
	TRANSFORM_INFO << "Converting RJM data to OBJ in order to apply desired transformations\n";
	std::string converted = ObjFormatIntegration::GenerateObjData(*m);
	if (converted.empty())
	{
		TRANSFORM_ERROR << "Conversion of RJM data to OBJ for pipeline transformations failed\n";
		return model;

	}

	// Run the Assimp transformer over this data, applying all desired operations at the same time
	TRANSFORM_INFO << "Applying Assimp input transformer over data (operations={" << m_operations << "})\n";
	InputTransformerAssimp *transformer = new InputTransformerAssimp(m_operations);
	auto transformed_data = transformer->Transform(converted);
	if (transformed_data.get() == NULL)
	{
		TRANSFORM_ERROR << "Failed to apply transformations (ops={" << m_operations << "}) to model data\n";
		return model;
	}

	// Return the new transformed version of model data; existing data will go out of scope and be deallocaated
	return transformed_data;
}