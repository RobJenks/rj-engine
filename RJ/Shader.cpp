#include <string>
#include "Shader.h"


std::string Shader::ShaderTypeToString(Shader::Type type)
{
	switch (type)
	{
		case Type::ComputeShader:		return "Compute";
		case Type::DomainShader:		return "Domain";
		case Type::GeometryShader:		return "Geometry";
		case Type::HullShader:			return "Hull";
		case Type::PixelShader:			return "Pixel";
		case Type::VertexShader:		return "Vertex";
		default:						return "<unknown>";
	}
}

