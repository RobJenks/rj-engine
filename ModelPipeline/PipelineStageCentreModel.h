#pragma once

#include <iostream>
#include "../Definitions/ModelData.h"
#include "PipelineStage.h"

class PipelineStageCentreModel : public PipelineStage
{
public:

	inline std::string						GetName(void) const { return "PipelineStageCentreModel"; }

	std::unique_ptr<ModelData>				Transform(std::unique_ptr<ModelData> model) const;


private:


};