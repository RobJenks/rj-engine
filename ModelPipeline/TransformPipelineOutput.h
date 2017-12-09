#pragma once

#include <string>
#include <memory>
#include "TransformerComponent.h"
#include "Model.h"


class TransformPipelineOutput : public TransformerComponent
{
public:

	virtual std::string			Transform(std::unique_ptr<Model> model) const = 0;


private:


};