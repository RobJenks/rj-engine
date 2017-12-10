#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include "TransformerComponent.h"
#include "Model.h"
namespace fs = std::experimental::filesystem;


class TransformPipelineInput : public TransformerComponent
{
public:

	virtual std::unique_ptr<Model>		Transform(const std::string & data) const = 0;

	virtual std::unique_ptr<Model>		Transform(fs::path file) const = 0;


private:


};