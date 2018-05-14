#pragma once

#include <iostream>
#include "../Definitions/ModelData.h"
#include "PipelineStage.h"

class PipelineStageUnitScaleModel : public PipelineStage
{
public:

	inline std::string						GetName(void) const { return "PipelineStageUnitScaleModel"; }

	std::unique_ptr<ModelData>				ExecuteTransform(std::unique_ptr<ModelData> model) const;


private:


};