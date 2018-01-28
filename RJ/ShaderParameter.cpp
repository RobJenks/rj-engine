#include <string>
#include "ShaderParameter.h"

std::string ShaderParameter::ParameterTypeToString(ShaderParameter::Type type)
{
	switch (type)
	{
		case Type::ConstantBuffer:		return "ConstantBuffer";
		case Type::RWBuffer:			return "RWBuffer";
		case Type::RWTexture:			return "RWTexture";
		case Type::Sampler:				return "Sampler";
		case Type::StructuredBuffer:	return "StructuredBuffer";
		case Type::Texture:				return "Texture";
		default:						return "<unknown>";
	}
}