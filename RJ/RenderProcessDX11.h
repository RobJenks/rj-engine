#pragma once

#include "RenderProcess.h"
#include "ConstantBufferDX11.h"

class RenderProcessDX11 : public RenderProcess
{
public:

	static ShaderDX11::ShaderParameterIndex AttemptRetrievalOfShaderParameter(const ShaderDX11 *shader, const std::string & parameter_name);



private:



};

