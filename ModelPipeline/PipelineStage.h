#pragma once

#include <memory>
#include "TransformerComponent.h"
#include "Model.h"

class PipelineStage : public TransformerComponent
{
public:

	virtual std::unique_ptr<Model>				Transform(std::unique_ptr<Model> model) const = 0;


private:



};