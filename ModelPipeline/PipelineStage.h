#pragma once

#include <memory>
#include "TransformerComponent.h"
#include "../Definitions/ModelData.h"

class PipelineStage : public TransformerComponent
{
public:

	virtual std::unique_ptr<ModelData>				Transform(std::unique_ptr<ModelData> model) const = 0;


private:



};