#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include "TransformerComponent.h"
#include "../Definitions/ModelData.h"
namespace fs = std::experimental::filesystem;


// Input transform may return multiple ModelData objects, each of which will be pushed through the pipeline independently
class TransformPipelineInput : public TransformerComponent
{
public:

	virtual std::vector<std::unique_ptr<ModelData>>		Transform(const std::string & data) const = 0;

	virtual std::vector<std::unique_ptr<ModelData>>		Transform(fs::path file) const = 0;


private:


};