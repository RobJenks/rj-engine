#include "RenderProcessDX11.h"
#include "Logging.h"

ShaderDX11::ShaderParameterIndex RenderProcessDX11::AttemptRetrievalOfShaderParameter(const ShaderDX11 *shader, const std::string & parameter_name)
{
	if (!shader)
	{
		Game::Log << LOG_ERROR << "Cannot retrieve shader parameter \"" << parameter_name << "\" during render process initialisation; no valid shader reference provided\n";
		return ShaderDX11::INVALID_SHADER_PARAMETER;
	}

	auto index = shader->GetParameterIndexByName(parameter_name);
	if (index == ShaderDX11::INVALID_SHADER_PARAMETER)
	{
		Game::Log << LOG_ERROR << "Failed to retrieve shader parameter \"" << parameter_name << "\" during render process initialisation; parameter does not exist\n";
	}

	return index;
}