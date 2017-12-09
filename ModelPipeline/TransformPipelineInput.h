#pragma once

#include <string>
#include <memory>
#include "TransformerComponent.h"
#include "Model.h"

class TransformPipelineInput : public TransformerComponent
{
public:

	virtual std::unique_ptr<Model>		Transform(const std::string & data) const = 0;


private:


};